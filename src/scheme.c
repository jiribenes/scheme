#include "scheme.h"

int main(void) {
    vm_t *vm = vm_new();

    vm_free(vm);
    return 0;
}
