#include <stdarg.h>  // va_list, ...
#include <stdio.h>   // fprintf, stderr
#include <stdlib.h>  // realloc
#include <string.h>  // memset

#if DEBUG
#include <time.h>  // clock(), CLOCKS_PER_SEC
#endif             // DEBUG`

#include "scheme.h"
#include "value.h"
#include "vm.h"
#include "write.h"

#define MAX_ALLOCATED 1024 * 1024 * 32  // 32 MB

void error_runtime(vm_t *vm, const char *format, ...) {
    vm->has_error = true;
    if (vm->config.error_fn == NULL) {
        return;
    }

    char message[256];

    va_list contents;
    va_start(contents, format);

    vsnprintf(message, 256, format, contents);

    va_end(contents);

    vm->config.error_fn(vm, -1, message);
}

/* *** */

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
    config->load_fn = NULL;

    // TODO: tweak the initial and min heap sizes
    config->heap_size_initial = 512 * 1024;  // 512 kB
    config->heap_size_min = 64 * 1024;       //  64 kB
    config->heap_growth = 0.5;               //  50%
}

vm_t *vm_new(scm_config_t *config) {
    scm_realloc_fn reallocate = scm_realloc_default;
    if (config != NULL) reallocate = config->realloc_fn;

    vm_t *vm = (vm_t *) reallocate(NULL, sizeof(vm_t));

    memset(vm, 0, sizeof(vm_t));

    if (config != NULL) {
        memcpy(&vm->config, config, sizeof(scm_config_t));
    } else {
        scm_config_default(&vm->config);
    }

    vm->allocated = 0;
    vm->gc_threshold = vm->config.heap_size_initial;

    vm->symbol_table = NULL;

    vm->env = NULL;
    vm->reader = NULL;

    vm->top_env = NULL;

    vm->curval = NIL_VAL;
    vm->gensym_count = 0;

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

void *vm_realloc(vm_t *vm, void *ptr, size_t old_size, size_t new_size) {
    vm->allocated += new_size - old_size;

    if (new_size > 0 && vm->allocated > vm->gc_threshold) {
        vm_gc(vm);
    }
#if !NOGC
    if (vm->allocated > MAX_ALLOCATED) {
        error_runtime(
            vm,
            "Can't allocate - already allocated more than %d (MAX_ALLOCATED)!",
            MAX_ALLOCATED);
        vm->allocated -= new_size;
        return NULL;
    }
#endif  // !NOGC
    return vm->config.realloc_fn(ptr, new_size);
}

/* *** temp *** */

void vm_push_temp(vm_t *vm, ptrvalue_t *ptr) {
    if (ptr == NULL) {
        error_runtime(vm, "Cannot have a NULL as a temporary root!");
    }
    if (vm->num_temp >= MAX_NUM_TEMP) {
        error_runtime(
            vm, "Trying to add a temporary root that is over limit (%d is max)",
            MAX_NUM_TEMP);
    }
    vm->temp[vm->num_temp++] = ptr;
}

void vm_pop_temp(vm_t *vm) {
    if (vm->num_temp <= 0) {
        error_runtime(vm, "No more temporary roots to release!");
    }
    vm->num_temp--;
}

/* *** GC *** */

static void mark(vm_t *vm, value_t val) {
    if (IS_VAL(val)) {
        return;
    }
    ptrvalue_t *ptr = AS_PTR(val);
    if (ptr == NULL) {
        error_runtime(vm, "|GC: Cannot mark a NULL!");
        return;
    }
    if (ptr->gcmark) {
        return;
    }

    ptr->gcmark = true;

    if (ptr->type == T_CONS) {
        cons_t *cons = (cons_t *) ptr;
        mark(vm, cons->car);
        mark(vm, cons->cdr);
    } else if (ptr->type == T_ENV) {
        env_t *env = (env_t *) ptr;
        mark(vm, env->variables);
        if (!IS_NIL(env->variables)) {
            cons_t *vars = AS_CONS(env->variables);
            while (true) {
                value_t pair = vars->car;
                mark(vm, pair);
                if (IS_NIL(vars->cdr)) {
                    break;
                }
                vars = AS_CONS(vars->cdr);
            }
        }

        if (env->up != NULL) {
            mark(vm, PTR_VAL(env->up));
        }
    } else if (ptr->type == T_PRIMITIVE) {
        primitive_t *prim = (primitive_t *) ptr;
        if (prim->name != NULL) {
            mark(vm, PTR_VAL(prim->name));
        }
    } else if (ptr->type == T_FUNCTION || ptr->type == T_MACRO) {
        function_t *func = (function_t *) ptr;

        if (func->name != NULL) {
            mark(vm, PTR_VAL(func->name));
        }

        mark(vm, func->params);
        mark(vm, func->body);
        mark(vm, PTR_VAL(func->env));
    } else if (ptr->type == T_VECTOR) {
        vector_t *vec = (vector_t *) ptr;

        for (uint32_t i = 0; i < vec->count; i++) {
            mark(vm, vec->data[i]);
        }
    }
}

static void markall(vm_t *vm) {
    env_t *env = vm->env;
    mark(vm, PTR_VAL(env));

    if (vm->reader != NULL) {
        mark(vm, vm->reader->tokval);
    }

    for (size_t i = 0; i < vm->num_temp; i++) {
        mark(vm, PTR_VAL(vm->temp[i]));
    }

    mark(vm, vm->curval);
}

// returns the size of a value
static size_t vm_size(vm_t *vm, value_t val) {
    if (IS_VAL(val)) {
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
    } else if (IS_FUNCTION(val) || IS_MACRO(val)) {
        return sizeof(function_t);
    } else if (IS_VECTOR(val)) {
        vector_t *vec = AS_VECTOR(val);
        return sizeof(vector_t) + sizeof(value_t) * (vec->capacity);
    } else if (IS_ENV(val)) {
        return sizeof(env_t);
    }
    // This should be an assert
    error_runtime(vm, "Cannot calculate the size of this value!");
    return 0;
}

// A very basic mark and sweep GC
void vm_gc(vm_t *vm) {
#if NOGC
    return;
#endif  // NOGC
#if DEBUG
    fprintf(stdout, "GC started\n");
    size_t allocated_prev = vm->allocated;
    double time_start = (double) clock() / CLOCKS_PER_SEC;
#endif  // DEBUG
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
    vm->gc_threshold = vm->allocated * (1 + vm->config.heap_growth);
    if (vm->gc_threshold < vm->config.heap_size_min) {
        vm->gc_threshold = vm->config.heap_size_min;
    }
#if DEBUG
    double time_delta = ((double) clock() / CLOCKS_PER_SEC) - time_start;
    fprintf(stdout, "GC finished: %zuB previously, %zuB now (%zuB gc'd), "
                    "new threshold at %zuB! Time delta: %.3lfs.\n",
            allocated_prev, vm->allocated, (allocated_prev - vm->allocated),
            vm->gc_threshold, time_delta);
#endif  // DEBUG
}

/* *** ENV *** */

// Adds a variable (pair of symbol and its value) to the env. frame
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val) {
    value_t pair = cons_fn(vm, PTR_VAL(sym), val);  // (sym . val)
    value_t temp = cons_fn(vm, pair, env->variables);
    env->variables = temp;
}

// Creates a new env frame
env_t *env_push(vm_t *vm, env_t *env, value_t vars, value_t vals) {
    value_t alist = NIL_VAL;
    value_t vars_iter, vals_iter;

    // TODO: should we create a macro - like SCM_ASSOC_FOREACH?
    for (vars_iter = vars, vals_iter = vals; IS_CONS(vars_iter);
         vars_iter = AS_CONS(vars_iter)->cdr,
        vals_iter = AS_CONS(vals_iter)->cdr) {
        if (!IS_CONS(vals_iter)) {
            // TODO: Add proper function arity?
            error_runtime(vm, "Number of arguments doesn't match!");
        }

        value_t par = AS_CONS(vars_iter)->car;
        value_t arg = AS_CONS(vals_iter)->car;

        value_t pair = cons_fn(vm, par, arg);
        value_t temp = cons_fn(vm, pair, alist);

        alist = temp;
    }
    if (!IS_NIL(vars_iter)) {
        value_t pair = cons_fn(vm, vars_iter, vals_iter);
        value_t temp = cons_fn(vm, pair, alist);

        alist = temp;
    }
    env_t *new_env = env_new(vm, alist, env);
    return new_env;
}

void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len,
                   primitive_fn fn) {
    symbol_t *sym = symbol_intern(vm, name, len);

    primitive_t *prim = primitive_new(vm, fn);

    prim->name = sym;

    variable_add(vm, env, sym, PTR_VAL(prim));
}

/* *** EVAL/APPLY *** */

// Applies <func> in <env> to <args>
static value_t apply_func(vm_t *vm, env_t *env, function_t *func,
                          value_t args) {
    value_t params = func->params;
    env_t *new_env = func->env;
    new_env = env_push(vm, new_env, params, args);
    value_t body = func->body;
    return begin(vm, new_env, body);
}

// Tries to apply the value <fn> in <env> to <args>
value_t apply(vm_t *vm, env_t *env, value_t fn, value_t args) {
    if (!IS_NIL(args) && !IS_CONS(args)) {
        error_runtime(vm, "|apply: Cannot apply to a non-list!");
        return NIL_VAL;
    }

    if (IS_PRIMITIVE(fn)) {
        primitive_t *prim = AS_PRIMITIVE(fn);
        return prim->fn(vm, env, args);
    } else if (IS_FUNCTION(fn)) {
        function_t *func = AS_FUNCTION(fn);
        value_t eargs = eval_list(vm, env, args);
        return apply_func(vm, env, func, eargs);
    }

    error_runtime(vm, "|apply: Cannot apply something else than a procedure!");
    return NIL_VAL;
}

// Tries to find <sym> in <env>
// Returns `undefined` if not found
value_t find(env_t *env, symbol_t *sym) {
    for (env_t *e = env; e != NULL; e = e->up) {
        if (IS_NIL(e->variables)) {
            // we're not going to find anything here, let's move on
            continue;
        }
        value_t pair, iter;
        SCM_FOREACH (pair, AS_CONS(e->variables), iter) {
            symbol_t *key = AS_SYMBOL(AS_CONS(pair)->car);
            if (key == sym) {
                return AS_CONS(pair)->cdr;
            }
        }
    }

    fprintf(stderr, "|find: Error: Symbol %s not bound\n", sym->name);
    // error_runtime(vm, "|find: Symbol %s is not bound in environment!",
    // sym->name);
    return UNDEFINED_VAL;
}

// Tries to find <sym> in <env> and replace it's val with <new_val>
// Returns `undefined` if not found
// TODO: this is maybe unnecessary duplication with 'find' above?
value_t find_replace(env_t *env, symbol_t *sym, value_t new_val) {
    for (env_t *e = env; e != NULL; e = e->up) {
        if (IS_NIL(e->variables)) {
            // we're not going to find anything here, let's move on
            continue;
        }
        value_t pair, iter;
        SCM_FOREACH (pair, AS_CONS(e->variables), iter) {
            symbol_t *key = AS_SYMBOL(AS_CONS(pair)->car);
            if (key == sym) {
                AS_CONS(pair)->cdr = new_val;
                return new_val;
            }
        }
    }

    fprintf(stderr, "|find-replace: Error: Symbol %s not bound\n", sym->name);
    // error_runtime(vm, "|find: Symbol %s is not bound in environment!",
    // sym->name);
    return UNDEFINED_VAL;
}

// Evaluates all members of list and returns their return values as a list
value_t eval_list(vm_t *vm, env_t *env, value_t list) {
    if (IS_NIL(list)) {
        return NIL_VAL;
    }
    cons_t *head = NULL;
    cons_t *tail = NULL;

    for (cons_t *cons = AS_CONS(list);; cons = AS_CONS(cons->cdr)) {
        value_t temp = eval(vm, env, cons->car);
        if (head == NULL) {
            head = tail = AS_CONS(cons_fn(vm, temp, NIL_VAL));
            vm_push_temp(vm, (ptrvalue_t *) (head));
        } else {
            tail->cdr = cons_fn(vm, temp, NIL_VAL);
            tail = AS_CONS(tail->cdr);
        }
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    vm_pop_temp(vm);  // head
    return PTR_VAL(head);
}

// Evaluates the value <val>
// Returns `undefined` if symbol not found
value_t eval(vm_t *vm, env_t *env, value_t val) {
    if (IS_VAL(val) || IS_STRING(val) || IS_PROCEDURE(val) || IS_VECTOR(val) ||
        IS_ENV(val)) {
        // These values are self evaluating
        return val;
    } else if (IS_SYMBOL(val)) {
        // This is a variable
        symbol_t *sym = AS_SYMBOL(val);
        value_t result = find(env, sym);
        if (IS_UNDEFINED(result)) {
            error_runtime(vm, "|eval: Can't eval %s - symbol not bound!",
                          sym->name);
        }
        return result;
    } else if (IS_CONS(val)) {
        // It's a function application
        value_t expanded = expand(vm, env, val);

        if (!IS_EQ(expanded, val)) {
            // macros encountered and expanded, now eval the expanded value
            return eval(vm, env, expanded);
        }

        cons_t *cons = AS_CONS(val);
        value_t fn = eval(vm, env, cons->car);
        value_t args = cons->cdr;

        if (!IS_PROCEDURE(fn)) {
            error_runtime(vm, "|eval: Car of eval'd cons is not a procedure!");
        }
        return apply(vm, env, fn, args);
    }

    // This should be an assert
    error_runtime(vm, "|eval: Cannot eval - unknown value type!");
    return NIL_VAL;
}

// Evaluates expressions in order, returns the last evaluated
// or #<void> if <val> is empty (NIL)
value_t begin(vm_t *vm, env_t *env, value_t val) {
    value_t result = VOID_VAL;
    if (IS_NIL(val)) {
        return result;
    }
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(val), iter) { result = eval(vm, env, arg); }
    return result;
}

value_t expand(vm_t *vm, env_t *env, value_t val) {
    symbol_t *sym;
    if (IS_SYMBOL(val)) {
        sym = AS_SYMBOL(val);
    } else if (IS_CONS(val) && IS_SYMBOL(AS_CONS(val)->car)) {
        sym = AS_SYMBOL(AS_CONS(val)->car);
    } else {
        return val;
    }

    value_t found = find(env, sym);
    if (IS_UNDEFINED(found) || !IS_MACRO(found)) {
        return val;
    }

    // TODO: If ever a 'AS_MACRO' is created, here is a good place to put it
    function_t *macro = AS_FUNCTION(found);
    value_t args = AS_CONS(val)->cdr;
    return apply_func(vm, env, macro, args);
}
