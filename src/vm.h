#ifndef _vm_h
#define _vm_h

#include <stdlib.h> // size_t, malloc, realloc

#include "value.h" // value_t

typedef struct {
    ptrvalue_t *head; // most recent allocated

    size_t allocated;
    size_t gc_threshold; // allocated needed to trigger gc
} vm_t;

vm_t *vm_new();
void vm_free(vm_t *vm);

void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size);

void gc(vm_t *vm);

#endif // _vm_h
