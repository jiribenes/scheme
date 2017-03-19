#ifndef _scheme_h
#define _scheme_h

typedef struct vm_t vm_t;

vm_t *vm_new();
void vm_free(vm_t *vm);

void *vm_realloc(vm_t *vm, void* ptr, size_t old_size, size_t new_size);

void gc(vm_t *vm);

#endif // _scheme_h
