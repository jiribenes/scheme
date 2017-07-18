#ifndef _scheme_h
#define _scheme_h

#include <stdlib.h> // size_t

// semantic versioning
#define SCM_VERSION_MAJOR 0
#define SCM_VERSION_MINOR 1
#define SCM_VERSION_PATCH 0

#define SCM_VERSION_STRING "0.1.0"

// VM for executing scm code
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
