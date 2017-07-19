#include <stdlib.h> // realloc
#include <string.h> // memset
#include <stdio.h> // fprintf, stderr

#include "scheme.h"
#include "vm.h"
#include "value.h"
#include "write.h"

#define MAX_ALLOCATED 1024*1024*16

static void *scm_realloc_default(void *ptr, size_t new_size) {
    if (new_size == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, new_size);
}

void scm_config_default(scm_config_t *config) {
    config->realloc_fn = scm_realloc_default;
    config->error_fn = NULL;
}

vm_t *vm_new(scm_config_t *config) {
    scm_realloc_fn reallocate = scm_realloc_default;
    if (config != NULL) reallocate = config->realloc_fn;

    vm_t *vm = (vm_t*) reallocate(NULL, sizeof(vm_t));

    memset(vm, 0, sizeof(vm_t));

    if (config != NULL) {
        memcpy(&vm->config, config, sizeof(scm_config_t));
    } else {
        scm_config_default(&vm->config);
    }

    vm->allocated = 0;
    vm->gc_threshold = 65536;

    vm->symbol_table = NULL;

    vm->env = NULL;
    vm->reader = NULL;

    vm->curval = NIL_VAL;

    vm->has_error = false;

    return vm;
}

void vm_free(vm_t *vm) {
    ptrvalue_t *ptr = vm->head;
    while (ptr != NULL) {
        ptrvalue_t *next = ptr->next;
        ptr_free(vm, ptr);
        ptr = next;
    }

    vm_realloc(vm, vm, 0, 0);
}


void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size) {
    vm->allocated += new_size - old_size;

    if (new_size > 0 && vm->allocated > vm->gc_threshold) {
        vm_gc(vm);
    }

    if (vm->allocated > MAX_ALLOCATED) {
        fprintf(stderr, "Error: Allocated more than %d! (MAX_ALLOCATED)\n", MAX_ALLOCATED);
        vm->allocated -= new_size;
        return NULL;
    }


    return vm->config.realloc_fn(ptr, new_size);
}

/* *** temp *** */

void vm_push_temp(vm_t *vm, ptrvalue_t *ptr) {
    if (ptr == NULL) {
        fprintf(stderr, "Error: Cannot root NULL\n");
    }
    if (vm->num_temp >= MAX_NUM_TEMP) {
        fprintf(stderr, "Error: Too many temp roots!\n");
    }
    vm->temp[vm->num_temp++] = ptr;
}

void vm_pop_temp(vm_t *vm) {
    if (vm->num_temp <= 0) {
        fprintf(stderr, "Error: No more temp roots to release!\n");
    }
    vm->num_temp--;
}


/* *** GC *** */

static void mark(value_t val) {
    if (!IS_PTR(val)) {
        return;
    }
    ptrvalue_t *ptr = AS_PTR(val);
    if (ptr == NULL) {
        fprintf(stderr, "Error: marking a NULL\n");
    }
    if (ptr->gcmark) {
        return;
    }

    ptr->gcmark = true;

    if (ptr->type == T_CONS) {
        cons_t *cons = (cons_t*) ptr;
        mark(cons->car);
        mark(cons->cdr);
    } else if (ptr->type == T_ENV) {
        env_t *env = (env_t*) ptr;
        mark(env->variables);
        if (!IS_NIL(env->variables)) {
            cons_t *vars = AS_CONS(env->variables);
            while (true) {
                value_t pair = vars->car;
                mark(pair);
                if (IS_NIL(vars->cdr)) {
                    break;
                }
                vars = AS_CONS(vars->cdr);
            }
        }

        if (env->up != NULL) {
            mark(PTR_VAL(env->up));
        }
    } else if (ptr->type == T_FUNCTION) {
        function_t *func = (function_t*) ptr;

        mark(func->params);
        mark(func->body);
        mark(PTR_VAL(func->env));
    }
}

static void markall(vm_t *vm) {
    env_t *env = vm->env;
    mark(PTR_VAL(env));

    if (vm->reader != NULL) {
        mark(vm->reader->tokval);
    }

    for (size_t i = 0; i < vm->num_temp; i++) {
        mark(PTR_VAL(vm->temp[i]));
    }
    mark(vm->curval);
}

// returns the size of a value
static size_t vm_size(vm_t *vm, value_t val) {
    if (!IS_PTR(val)) {
        return 0;
    } else if (IS_CONS(val)) {
        return sizeof(cons_t);
    } else if (IS_STRING(val)) {
        string_t *str = AS_STRING(val);
        return sizeof(string_t) + sizeof(char) * (str->len + 1);
    } else if (IS_SYMBOL(val)) {
        symbol_t *sym = AS_SYMBOL(val);
        return sizeof(symbol_t) + sizeof(char) * (sym->len + 1);
    } else if (IS_PRIMITIVE(val)) {
        return sizeof(primitive_t);
    } else if (IS_FUNCTION(val)) {
        return sizeof(function_t);
    } else if (IS_ENV(val)) {
        return sizeof(env_t);
    }
    fprintf(stderr, "Error: cannot calculate the size of a value!\n");
    return 0;
}

// A very basic mark and sweep GC
void vm_gc(vm_t *vm) {
#ifdef NOVM
    return;
#endif
#ifdef DEBUG
    fprintf(stdout, "GC started\n");
    size_t prev_allocated = vm->allocated;
#endif
    vm->allocated = 0;
    markall(vm);
    ptrvalue_t **head = &vm->head;
    while (*head != NULL) {
        if (!((*head)->gcmark)) {
            ptrvalue_t *not_found = *head;
            *head = not_found->next;
            ptr_free(vm, not_found);
        } else {
            (*head)->gcmark = false;
            vm->allocated += vm_size(vm, PTR_VAL(*head));
            head = &(*head)->next;
        }
    }
#ifdef DEBUG
    fprintf(stdout, "GC finished: %zu bytes previously, %zu bytes now!\n", prev_allocated, vm->allocated);
#endif
    vm->gc_threshold = vm->allocated * 2;
    return;
}

/* *** ENV *** */

// Adds a variable (pair of symbol and its value) to the env. frame
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val) {
    value_t pair = cons_fn(vm, PTR_VAL(sym), val); // (sym . val)
    value_t temp = cons_fn(vm, pair, env->variables);
    env->variables = temp;
}

// Creates a new env frame
env_t *env_push(vm_t *vm, env_t *env, value_t vars, value_t vals) {
    if (cons_len(vars) != cons_len(vals)) {
        fprintf(stderr, "Error: Number of arguments doesn't match!\n");
    }
    value_t alist = NIL_VAL;
    cons_t *var = NULL;
    cons_t *val = NULL;
    if (!IS_NIL(vars)) {
        var = AS_CONS(vars);
        val = AS_CONS(vals);
    }
    while (!IS_NIL(var->car)) {
        value_t pair = cons_fn(vm, var->car, val->car);
        value_t temp = cons_fn(vm, pair, alist);
        alist = temp;

        if (IS_NIL(var->cdr)) {
            break;
        }
        var = AS_CONS(var->cdr);
        val = AS_CONS(val->cdr);
    }
    env_t *new_env = env_new(vm, alist, env);
    return new_env;
}

void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len, primitive_fn fn) {
    symbol_t *sym = symbol_intern(vm, name, len);

    primitive_t *prim = primitive_new(vm, fn);

    variable_add(vm, env, sym, PTR_VAL(prim));
}

/* *** EVAL/APPLY *** */
static value_t apply_func(vm_t *vm, env_t *env, function_t *func, value_t args) {
    value_t params = func->params;
    env_t *new_env = func->env;
    new_env = env_push(vm, new_env, params, args);
    value_t body = func->body;
    return begin(vm, new_env, body);
}

static value_t apply(vm_t *vm, env_t *env, value_t fn, value_t args) {
    if (!IS_NIL(args) && !IS_CONS(args)) {
        fprintf(stderr, "Error: Cannot apply to a non-list\n");
    }
    if (IS_PRIMITIVE(fn)) {
        primitive_t *prim = AS_PRIMITIVE(fn);
        return prim->fn(vm, env, args);
    } else if (IS_FUNCTION(fn)) {
        function_t *func = AS_FUNCTION(fn);
        value_t eargs = eval_list(vm, env, args);
        return apply_func(vm, env, func, eargs);
    }

    fprintf(stderr, "Error: Cannot apply something else than a primitive or a function\n");
    return NIL_VAL;
}

static value_t find(env_t *env, symbol_t *sym) {
    for (env_t *e = env; e != NULL; e = e->up) {
        if (IS_NIL(e->variables)) {
            return NIL_VAL;
        }
        cons_t *vars = AS_CONS(e->variables);
        while (true) {
            cons_t *pair = AS_CONS(vars->car);
            symbol_t *key = AS_SYMBOL(pair->car);
            if (key == sym) {
                return pair->cdr;
            }
            if (IS_NIL(vars->cdr)) {
                break;
            }
            vars = AS_CONS(vars->cdr);
        }
    }

    fprintf(stderr, "Error: Symbol %s not bound\n", sym->name);
    return NIL_VAL;
}

value_t eval_list(vm_t *vm, env_t *env, value_t list) {
    if (IS_NIL(list)) {
        return NIL_VAL;
    }
    cons_t *head = NULL;
    cons_t *tail = NULL;

    for (cons_t *cons = AS_CONS(list); ;cons = AS_CONS(cons->cdr)) {
        value_t temp = eval(vm, env, cons->car);
        if (head == NULL) {
            head = tail = AS_CONS(cons_fn(vm, temp, NIL_VAL));
            vm_push_temp(vm, (ptrvalue_t*) (head));
        } else {
            tail->cdr = cons_fn(vm, temp, NIL_VAL);
            tail = AS_CONS(tail->cdr);
        }
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    vm_pop_temp(vm); //head
    return PTR_VAL(head);
}

value_t eval(vm_t *vm, env_t *env, value_t val) {
    if (!IS_PTR(val) || IS_STRING(val) || IS_PRIMITIVE(val) || IS_FUNCTION(val)) {
        return val;
    } else if (IS_SYMBOL(val)) {
        symbol_t *sym = AS_SYMBOL(val);
        return find(env, sym);
    } else if (IS_CONS(val)) {
        cons_t *cons = AS_CONS(val);
        value_t fn = cons->car;
        fn = eval(vm, env, cons->car);
        value_t args = cons->cdr;
        if (!IS_PRIMITIVE(fn) && !IS_FUNCTION(fn)) {
            fprintf(stderr, "Error: Car of a cons must be a function in eval\n");
        }
        return apply(vm, env, fn, args);
    }
    fprintf(stderr, "Error: Unknown type to eval!\n");
    return NIL_VAL;
}


value_t begin(vm_t *vm, env_t *env, value_t val) {
    value_t result = NIL_VAL;
    for (cons_t *cons = AS_CONS(val); ;cons = AS_CONS(cons->cdr)) {
        result = eval(vm, env, cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return result;
}
