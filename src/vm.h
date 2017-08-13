#ifndef _vm_h
#define _vm_h

#include <stdarg.h>  // va_list, ...
#include <stdlib.h>  // size_t, malloc, realloc

#include "config.h"
#include "read.h"   // reader_t
#include "value.h"  // ptrvalue_t, symbol_t, env_t

#define MAX_NUM_TEMP 256

// Signals a runtime error
void error_runtime(vm_t *vm, const char *format, ...);

struct vm_t {
    // a linked list of all allocated values
    ptrvalue_t *head;

    // total size of all allocated values
    size_t allocated;
    // the size of allocated values
    // to trigger the garbage collector
    size_t gc_threshold;

    // a table of all symbols (needed for interning)
    symbol_t *symbol_table;

    scm_config_t config;

    env_t *env;
    reader_t *reader;

    env_t *top_env;

    value_t curval;

    uint32_t gensym_count;

    // a stack of temporary roots
    // these are values, that shouldn't be deleted by the gc
    size_t num_temp;
    ptrvalue_t *temp[MAX_NUM_TEMP];

    // indicates if the VM encountered an error
    // we want to accumulate as many errors as possible!
    bool has_error;
};

// adds a primitive function under name [name] to env
void primitive_add(vm_t *vm, env_t *env, const char *name, size_t len,
                   primitive_fn fn);
// adds a variable to env
void variable_add(vm_t *vm, env_t *env, symbol_t *sym, value_t val);

// tries to find a <sym> in <env>
value_t find(env_t *env, symbol_t *sym);

// tries to find a <sym> in <env>, if found, replaces it's val with <new_val>
value_t find_replace(env_t *env, symbol_t *sym, value_t new_val);

// evaluates a list
value_t eval_list(vm_t *vm, env_t *env, value_t list);
// evaluates a value
value_t eval(vm_t *vm, env_t *env, value_t val);
// applies <fn> to <args>
value_t apply(vm_t *vm, env_t *env, value_t fn, value_t args);
// evaluates all arguments in [val] and returns the latest value
value_t begin(vm_t *vm, env_t *env, value_t val);
value_t expand(vm_t *vm, env_t *env, value_t val);

void vm_push_temp(vm_t *vm, ptrvalue_t *ptr);
void vm_pop_temp(vm_t *vm);

#endif  // _vm_h
