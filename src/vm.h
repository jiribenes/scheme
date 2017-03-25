#ifndef _vm_h
#define _vm_h

#include <stdlib.h> // size_t, malloc, realloc

#include "value.h" // ptrvalue_t, symbol_t, env_t
#include "read.h" // reader_t

#define MAX_NUM_TEMP 256

struct vm_t {
    ptrvalue_t *head; // most recent allocated

    size_t allocated;
    size_t gc_threshold; // allocated needed to trigger gc

    symbol_t *symbol_table;

    env_t *env;
    reader_t *reader;

    value_t curval;
    
    size_t num_temp;
    ptrvalue_t *temp[MAX_NUM_TEMP];
};

void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len, primitive_fn fn);
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val);

value_t eval_list(vm_t *vm, env_t *env, value_t list);
value_t eval(vm_t *vm, env_t *env, value_t val);
value_t begin(vm_t *vm, env_t *env, value_t val);

void vm_push_temp(vm_t *vm, ptrvalue_t *ptr);
void vm_pop_temp(vm_t *vm);

#endif // _vm_h
