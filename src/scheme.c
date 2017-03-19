#include <stdio.h>

#include "scheme.h"
#include "value.h"
#include "write.h"

// write to stdout with newline
void test_write(value_t val) {
    write(stdout, val);
    puts("");
}

void test(vm_t *vm) {
    test_write(NIL_VAL);

    value_t num = NUM_VAL(42);
    test_write(num);
    
    cons_t *cons = cons_new(vm);
    cons->car = num;
    test_write(PTR_VAL(cons));

    cons->cdr = NUM_VAL(45);
    test_write(PTR_VAL(cons));

    cons_t *other = cons_new(vm);
    other->cdr = PTR_VAL(cons);
    other->car = NUM_VAL(43);
    test_write(PTR_VAL(other));

    string_t *str = string_new(vm, "test", 4);
    test_write(PTR_VAL(str));

    cons->car = PTR_VAL(str);
    test_write(PTR_VAL(other));

    vector_t *vec = vector_new(vm, 1);
    vec->data[0] = num;
    test_write(PTR_VAL(vec));

    vector_insert(vm, vec, NUM_VAL(14), 1);
    test_write(PTR_VAL(vec));

    symbol_t *sym = symbol_new(vm, "lambda", 6);
    test_write(PTR_VAL(sym));
}

int main(void) {
    vm_t *vm = vm_new();

    test(vm);

    vm_free(vm);
    return 0;
}
