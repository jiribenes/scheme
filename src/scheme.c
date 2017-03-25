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
        
        value_t result = eval(vm, vm->env, val); 
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

static value_t multiply(vm_t *vm, env_t *env, value_t args) {
    double result = 1.0F;
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        fprintf(stderr, "Error: *: not enough args (has %d)\n", cons_len(eargs));
        return NIL_VAL;
    }
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            fprintf(stderr, "Error: *: car is not a number!\n");
            return NIL_VAL;
        }
        result *= AS_NUM(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return NUM_VAL(result);
}

static value_t subtract(vm_t *vm, env_t *env, value_t args) {
    double result = 0.0F;
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        fprintf(stderr, "Error: -: not enough args (has %d)\n", cons_len(eargs));
        return NIL_VAL;
    }
    result = AS_NUM(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(AS_CONS(eargs)->cdr); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            fprintf(stderr, "Error: -: car is not a number!\n");
            return NIL_VAL;
        }
        result -= AS_NUM(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return NUM_VAL(result);
}

static value_t eq(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 2) {
        fprintf(stderr, "Error: eq?: args (has %d) != 2", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;
    bool result = val_eq(a, b);
    return BOOL_VAL(result);
}

static value_t equal(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 2) {
        fprintf(stderr, "Error: equal?: args (has %d) != 2", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;
    bool result = val_equal(a, b);
    return BOOL_VAL(result);    
}

static value_t quote(vm_t *vm, env_t *env, value_t args) {
    if (cons_len(args) != 1) {
        fprintf(stderr, "Error: ': args (has %d) != 1\n", cons_len(args));
    }
    return AS_CONS(args)->car;
}

static value_t list(vm_t *vm, env_t *env, value_t args) {
    return eval_list(vm, env, args);
}

static value_t define(vm_t *vm, env_t *env, value_t args) {
    if (!IS_CONS(args) || cons_len(args) != 2) {
        fprintf(stderr, "Error: define: is wrong (define <name> <body>) or (define (<name> <params> ...) <body>\n");
    }
    cons_t *rest = AS_CONS(args);
    if (IS_SYMBOL(rest->car)) {
        symbol_t *sym = AS_SYMBOL(rest->car);
        cons_t *body = AS_CONS(rest->cdr);
        value_t val = eval(vm, env, body->car);
        variable_add(vm, env, sym, val);
        return val;
    } else if (IS_CONS(rest->car)) {
        symbol_t *sym = AS_SYMBOL(AS_CONS(rest->car)->car);
        value_t params = AS_CONS(rest->car)->cdr;
        cons_t *body = AS_CONS(rest->cdr);
        function_t *func = function_new(vm, env, params, PTR_VAL(body));
        value_t val = eval(vm, env, PTR_VAL(func));
        variable_add(vm, env, sym, val);
        return val; 
     } else {
        fprintf(stderr, "Error: define: is wrong - second argument has to be either a list or a symbol!\n");
        return NIL_VAL;
     }
}

static value_t lambda(vm_t *vm, env_t *env, value_t args) {
    if (!IS_CONS(args) || cons_len(args) != 2 || !(IS_CONS(AS_CONS(args)->car) || IS_NIL(AS_CONS(args)->car))) {
        fprintf(stderr, "Error: lambda: is wrong (lambda (<params>) <body>)\n"); 
    }
    
    if (IS_NIL(AS_CONS(args)->car)) { // no parameters
        value_t body = AS_CONS(args)->cdr;
        function_t *func = function_new(vm, env, NIL_VAL, body);
        return PTR_VAL(func);
    }

    for (cons_t *cons = AS_CONS(AS_CONS(args)->car); ; cons = AS_CONS(cons->cdr)) {
        if (!IS_SYMBOL(cons->car)) {
            fprintf(stderr, "Error: lambda: all parameters must be symbols!\n");
        } else if (!IS_NIL(cons->cdr) && !IS_CONS(cons->cdr)) {
            fprintf(stderr, "Error: lambda: parameter list must not be dotted\n");
        }
        
        if (IS_NIL(cons->cdr)) {
            break;        
        }
    }
    value_t params = AS_CONS(args)->car;
    value_t body = AS_CONS(args)->cdr;
    function_t *func = function_new(vm, env, params, body);
    return PTR_VAL(func);
}

// (if <condition> <then> <otherwise> ...)
static value_t builtin_if(vm_t *vm, env_t *env, value_t args) {
    if (cons_len(args) < 2) {
        fprintf(stderr, "Error: if: not enough args (has %d)\n", cons_len(args));
    }

    value_t condition = eval(vm, env, AS_CONS(args)->car);
    if (AS_BOOL(condition)) {
        value_t then = eval(vm, env, AS_CONS(AS_CONS(args)->cdr)->car);
        return eval(vm, env, then);
    }

    if (IS_NIL(AS_CONS(AS_CONS(args)->cdr)->cdr)) {
        return FALSE_VAL;
    }
    
    value_t otherwise = AS_CONS(AS_CONS(args)->cdr)->cdr;
    return begin(vm, env, otherwise); 
}

static value_t builtin_cons(vm_t *vm, env_t *env, value_t args) {
    if (cons_len(args) != 2) {
        fprintf(stderr, "Error: eq?: args (has %d) != 2", cons_len(args));
    }

    value_t car = eval(vm, env, AS_CONS(args)->car);
    value_t cdr = eval(vm, env, AS_CONS(AS_CONS(args)->cdr)->car);

    value_t cons = cons_fn(vm, car, cdr);
    return cons;
}

static value_t builtin_is_cons(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        fprintf(stderr, "Error: cons?: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    return BOOL_VAL(IS_NIL(a) || IS_CONS(a));
}

static value_t builtin_car(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        fprintf(stderr, "Error: car: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    if (!IS_CONS(a)) {
        fprintf(stderr, "Error: car: arg is not a cons");
    }
    return AS_CONS(a)->car;
}

static value_t builtin_cdr(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        fprintf(stderr, "Error: cdr: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    if (!IS_CONS(a)) {
        fprintf(stderr, "Error: cdr: arg is not a cons");
    }
    return AS_CONS(a)->cdr;
}

/* *** */

static value_t builtin_gc(vm_t *vm, env_t *env, value_t args) {
    gc(vm);
    return NIL_VAL;
}

static value_t builtin_env(vm_t *vm, env_t *env, value_t args) {
    env_t *e = env;
    while (e != NULL) {
        test_write(e->variables);
        e = e->up;
    }
    return NIL_VAL;
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
    primitive_add(vm, env, "*", 1, multiply);
    primitive_add(vm, env, "-", 1, subtract);
    primitive_add(vm, env, "eq?", 3, eq);
    primitive_add(vm, env, "equal?", 6, equal);
    primitive_add(vm, env, "quote", 5, quote);
    primitive_add(vm, env, "list", 4, list);
    primitive_add(vm, env, "begin", 5, begin);
    primitive_add(vm, env, "define", 6, define);
    primitive_add(vm, env, "lambda", 6, lambda);
    primitive_add(vm, env, "if", 2, builtin_if);
    primitive_add(vm, env, "cons", 4, builtin_cons);
    primitive_add(vm, env, "cons?", 5, builtin_is_cons);
    primitive_add(vm, env, "car", 3, builtin_car);
    primitive_add(vm, env, "cdr", 3, builtin_cdr);

    primitive_add(vm, env, "gc", 2, builtin_gc);
    primitive_add(vm, env, "env", 3, builtin_env);

    repl(vm, env);

    vm_free(vm);
    return 0;
}
