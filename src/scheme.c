#include <stdio.h>

#include "scheme.h"
#include "value.h"
#include "write.h"
#include "read.h"
#include "vm.h"

// write to stdout with newline
static void test_write(value_t val) {
    write(stdout, val);
    puts("");
}

static cons_t *together(vm_t *vm, value_t a, value_t b) {
    cons_t *c = cons_new(vm);
    c->car = a;
    c->cdr = b;
    return c;
}

void test2(vm_t *vm) {
    puts("----\nTest 2");

    value_t eof = read_source(vm, "");
    test_write(eof);
    puts("");

    value_t val = read_source(vm, "42");
    test_write(val);
    puts("");

    value_t nil = read_source(vm, "()");
    test_write(nil);
    puts("");

    value_t twice = read_source(vm, "(())");
    test_write(twice);
    puts("");

    value_t list = read_source(vm, "(42 43 45)");
    test_write(list);
    puts("");

    value_t anotherlist = read_source(vm, "(42 (43 44 45 46) (47 48 49))");
    test_write(anotherlist);
    puts("");

    value_t bignumberlist = read_source(vm, "(0 (1.3 5.42 59.9999 (1.0 2.0 3.0)) (3 (4 (5 6))))");
    test_write(bignumberlist);
    puts("");

    value_t sym = read_source(vm, "symb");
    test_write(sym);
    puts("");
    
    value_t func = read_source(vm, "(define (square x) (* x x))");
    test_write(func);
    puts("");

    value_t truth = read_source(vm, "#t");
    test_write(truth);
    puts("");

    value_t truthy_fn = read_source(vm, "(define (truthy_fn)\n (#t))");
    test_write(truthy_fn);
    puts("");

    value_t arith_fn = read_source(vm, "(+ (* 20 0.55) (/ 2 3))");
    test_write(arith_fn);
    puts("");

    value_t dotted = read_source(vm, "((2 3) . 1");
    test_write(dotted);
    puts("");

    value_t dotted2 = read_source(vm, "(0 . (1 . 2))");
    test_write(dotted2);
    puts("");

    value_t dotted_simple = read_source(vm, "(2 . 1)");
    test_write(dotted_simple);
    puts("");

    value_t quoted = read_source(vm, "\'(a b)");
    test_write(quoted);
    puts("");

    value_t quotedsym = read_source(vm, "\'lambda");
    test_write(quotedsym);
    puts("");

    value_t str = read_source(vm, "\"str\"");
    test_write(str);
    puts("");

    value_t str2 = read_source(vm, "(define hello (display \"Hello\"))");
    test_write(str2);
    puts("");

    value_t f = read_source(vm, "#f");
    test_write(f);
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

    value_t f = FALSE_VAL;
    test_write(f);
}

void repl(vm_t *vm, env_t *env) {
    fprintf(stdout, "|Scheme 0.0 - REPL|\n|Use ^D to exit!:)|\n");
    char buf[512];

    while (true) {
        fprintf(stdout, ">> ");

        if (fgets(buf, 512, stdin) == NULL) {
            break;
        }
        value_t val = read_source(vm, buf);

        fprintf(stdout, "symbol table:\n");
        for (symbol_t *s = vm->symbol_table; s != NULL; s = s->next) {
            test_write(PTR_VAL(s));
        }
        fprintf(stdout, "env: ");
        test_write(env->variables);
        
        value_t result = eval(vm, env, val); 
        fprintf(stdout, "Your result:");
 
        test_write(result);
    }
    
    fprintf(stdout, "\nQuiting!\n");
}

static value_t add(vm_t *vm, env_t *env, value_t args) {
    double result = 0.0F;
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        fprintf(stderr, "Error: +: not enough args (has %d)\n", cons_len(eargs));
        test_write(eargs);
        return NIL_VAL;
    }
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            fprintf(stderr, "Error: +: car is not a number!\n");
            return NIL_VAL;
        }
        result += AS_NUM(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return NUM_VAL(result);
}

int main(void) {
    vm_t *vm = vm_new();
    env_t *env = env_new(vm, NIL_VAL, NULL);

    //test(vm);

    //test2(vm);
    
    symbol_t *pi_sym = symbol_intern(vm, "pi", 2);
    value_t pi = NUM_VAL(3.1415);
    variable_add(vm, env, pi_sym, pi);

    primitive_add(vm, env, "+", 1, add);

    fprintf(stdout, "---\n");

    repl(vm, env);

    vm_free(vm);
    return 0;
}
