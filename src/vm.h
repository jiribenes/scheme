#ifndef _vm_h
#define _vm_h

#include <stdlib.h> // size_t, malloc, realloc

#include "value.h" // ptrvalue_t, symbol_t

struct vm_t {
    ptrvalue_t *head; // most recent allocated

    size_t allocated;
    size_t gc_threshold; // allocated needed to trigger gc

    symbol_t *symbol_table;
};

void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len, primitive_fn fn);
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val);

value_t eval_list(vm_t *vm, env_t *env, value_t list);
value_t eval(vm_t *vm, env_t *env, value_t val);

#endif // _vm_h
