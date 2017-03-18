#include <stdlib.h>

#include "vm.h"
#include "value.h"

// TODO: Finish VM implementation
vm_t *vm_new() {
    vm_t *vm = malloc(sizeof(vm_t));

    vm->allocated = 0;
    vm->gc_threshold = 8;
    
    return vm;
}

void vm_free(vm_t *vm);

static void *scm_alloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, size);
}

void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size) {
    vm->allocated += new_size - old_size;

    if (new_size > 0 && vm->allocated > vm->gc_threshold) {
        gc(vm);
    }

    return scm_alloc(ptr, new_size);
}

// TODO: Implement GC
void gc(vm_t *vm);
