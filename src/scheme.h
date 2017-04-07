#ifndef _scheme_h
#define _scheme_h

#include <stdlib.h> // size_t

typedef struct vm_t vm_t;

// creates a new vm
vm_t *vm_new();
// frees a vm
void vm_free(vm_t *vm);

// (re)allocates a pointer
void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size);

// garbage collect
void vm_gc(vm_t *vm);

#endif // _scheme_h
