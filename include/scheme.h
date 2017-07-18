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

// Generic realloc function type (see scm_realloc_default for example)
typedef void* (*scm_realloc_fn) (void *ptr, size_t new_size);

// A function for reporting an error to the user
typedef void (*scm_error_fn) (vm_t *vm, int line, const char *message);

// Configuration struct for VM creation
typedef struct {
    // A function for allocating, reallocating and freeing memory
    scm_realloc_fn realloc_fn;

    // A function for reporting an error to the user
    scm_error_fn error_fn;
} scm_config_t;

// Loads a default config into the config struct
// Always use this before you begin chaning its members
void scm_config_default(scm_config_t *config);

// creates a new vm
vm_t *vm_new(scm_config_t *config);

// frees a vm
void vm_free(vm_t *vm);

// (re)allocates a pointer, uses vm->config.realloc_fn inside
void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size);

// garbage collect
void vm_gc(vm_t *vm);

#endif // _scheme_h
