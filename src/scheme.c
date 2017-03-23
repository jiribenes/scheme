#include <stdio.h>

#include "scheme.h"
#include "value.h"
#include "write.h"

// write to stdout with newline
static void test_write(value_t val) {
    write(stdout, val);
    puts("");
}

void test(vm_t *vm) {
    test_write(NIL_VAL); // ()

    value_t num = NUM_VAL(42);
    test_write(num); // 42
    
    cons_t *cons = cons_new(vm);
    cons->car = num;
    test_write(PTR_VAL(cons)); // (42)

    cons->cdr = NUM_VAL(45);
    test_write(PTR_VAL(cons)); // (42 . 45)

    cons_t *other = cons_new(vm);
    other->cdr = PTR_VAL(cons);
    other->car = NUM_VAL(43);
    test_write(PTR_VAL(other)); // (43 42 . 45)

    string_t *str = string_new(vm, "test", 4);
    test_write(PTR_VAL(str)); // "test"

    cons->car = PTR_VAL(str);
    test_write(PTR_VAL(other)); // (43 "test" . 45)

    symbol_t *sym = symbol_new(vm, "lambda", 6);
    test_write(PTR_VAL(sym)); // lambda

    other = together(vm, PTR_VAL(other), PTR_VAL(sym));
    test_write(PTR_VAL(other)); // (43 . lambda)
    
    cons_t *c = together(vm, NUM_VAL(14), NIL_VAL);
    c = together(vm, NUM_VAL(15), PTR_VAL(c));
    c = together(vm, NUM_VAL(16), PTR_VAL(c));
    test_write(PTR_VAL(c)); // (14 15 16)
}

int main(void) {
    vm_t *vm = vm_new();

    test(vm);

    vm_free(vm);
    return 0;
}
