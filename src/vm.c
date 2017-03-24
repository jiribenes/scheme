#include <stdlib.h> // realloc
#include <string.h> // memset
#include <stdio.h> // fprintf, stderr

#include "scheme.h"
#include "vm.h"
#include "value.h"

static void *scm_realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, size);
}

vm_t *vm_new() {
    vm_t *vm = (vm_t*) scm_realloc(NULL, sizeof(vm_t));

    memset(vm, 0, sizeof(vm_t));

    vm->allocated = 0;
    vm->gc_threshold = 8;
   
    vm->symbol_table = NULL;

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
        gc(vm);
    }

    return scm_realloc(ptr, new_size);
}

/* *** GC *** */

// TODO: Implement GC
void gc(vm_t *vm) {
    return;
}

/* *** ENV *** */

// Adds a variable (pair of symbol and its value) to the env. frame
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val) {
    value_t pair = cons_fn(vm, PTR_VAL(sym), val); // (sym . val)
    value_t temp = cons_fn(vm, pair, env->variables);
    env->variables = temp;
}

void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len, primitive_fn fn) {
    symbol_t *sym = symbol_intern(vm, name, len);

    primitive_t *prim = primitive_new(vm, fn);

    variable_add(vm, env, sym, PTR_VAL(prim));
}

/* *** EVAL/APPLY *** */
static value_t apply(vm_t *vm, env_t *env, value_t fn, value_t args) {
    if (!IS_NIL(args) && !IS_CONS(args)) {
        fprintf(stderr, "Error: Cannot apply to a non-list\n");
    }
    if (IS_PRIMITIVE(fn)) {
        primitive_t *prim = AS_PRIMITIVE(fn);
        return prim->fn(vm, env, args);
    }

    fprintf(stderr, "Error: Cannot apply something else than a primitive\n");
    return NIL_VAL;
}

// TODO: this is a mess...
static value_t find(env_t *env, symbol_t *sym) {
    for (env_t *e = env; e != NULL; e = e->up) {
        if (IS_NIL(env->variables)) {
            return NIL_VAL;
        }
        cons_t *vars = AS_CONS(env->variables);
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
        } else {
            tail->cdr = cons_fn(vm, temp, NIL_VAL);
            tail = AS_CONS(tail->cdr);
        }
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }

    return PTR_VAL(head);
}

value_t eval(vm_t *vm, env_t *env, value_t val) {
    if (!IS_PTR(val) || IS_STRING(val) || IS_PRIMITIVE(val)) {
        return val;
    } else if (IS_SYMBOL(val)) {
        symbol_t *sym = AS_SYMBOL(val);
        return find(env, sym);
    } else if (IS_CONS(val)) {
        cons_t *cons = AS_CONS(val);
        value_t fn = eval(vm, env, cons->car);
        value_t args = cons->cdr;
        if (!IS_PRIMITIVE(fn)) {
            fprintf(stderr, "Error: Car of a cons must be a function in eval\n");
        }
        return apply(vm, env, fn, args);
    }
    fprintf(stderr, "Error: Unknown type to eval!\n");
    return NIL_VAL;
}
