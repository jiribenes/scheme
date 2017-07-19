#include <stdio.h>
#include <stdarg.h>

#include "scheme.h"

#include "value.h"
#include "write.h"
#include "read.h"
#include "vm.h"

static void error_report(vm_t *vm, int line, const char *message) {
    if (line > 0) {
        fprintf(stderr, "ERROR @ line %d : %s\n", line, message);
    } else if (line == -1) {
        fprintf(stderr, "ERROR @ runtime : %s\n", message);
    }
}

static void error_runtime(vm_t *vm, const char *format, ...) {
    vm->has_error = true;
    if (vm->config.error_fn == NULL) {
        return;
    }

    char message[256];

    va_list contents;
    va_start(contents, format);

    vsnprintf(message, 256, format, contents);

    va_end(contents);

    vm->config.error_fn(vm, -1, message);
}

/* *** */

static value_t add(vm_t *vm, env_t *env, value_t args) {
    double result = 0.0F;
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        error_runtime(vm, "+: not enough args (has %d)", cons_len(eargs));
        return NIL_VAL;
    }
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            error_runtime(vm, "+: car is not a number!");
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
        error_runtime(vm, "*: not enough args (has %d)", cons_len(eargs));
        return NIL_VAL;
    }
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            error_runtime(vm, "*: car is not a number!");
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
        error_runtime(vm, "-: not enough args (has %d)", cons_len(eargs));
        return NIL_VAL;
    }
    result = AS_NUM(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(AS_CONS(eargs)->cdr); ; cons = AS_CONS(cons->cdr)){
        if (!IS_NUM(cons->car)) {
            error_runtime(vm, "-: car is not a number!");
            return NIL_VAL;
        }
        result -= AS_NUM(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return NUM_VAL(result);
}

static value_t gt(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 2) {
        error_runtime(vm, ">: args (has %d) != 2", cons_len(args));
    }

    value_t a = AS_CONS(eargs)->car;
    value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;

    if (!IS_NUM(a) || !IS_NUM(b)) {
        error_runtime(vm, ">: arg is not a number!");
    }
    return BOOL_VAL(AS_NUM(a) > AS_NUM(b));
}

static value_t eq(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 2) {
        error_runtime(vm, "eq?: args (has %d) != 2", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;
    bool result = val_eq(a, b);
    return BOOL_VAL(result);
}

static value_t equal(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 2) {
        error_runtime(vm, "equal?: args (has %d) != 2", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;
    bool result = val_equal(a, b);
    return BOOL_VAL(result);
}

static value_t quote(vm_t *vm, env_t *env, value_t args) {
    if (cons_len(args) != 1) {
        error_runtime(vm, "': args (has %d) != 1", cons_len(args));
    }
    return AS_CONS(args)->car;
}

static value_t list(vm_t *vm, env_t *env, value_t args) {
    return eval_list(vm, env, args);
}

static value_t builtin_define(vm_t *vm, env_t *env, value_t args) {
    cons_t *rest = AS_CONS(args);

    if (IS_SYMBOL(rest->car)) { // (define <name> <body...>)
        symbol_t *sym = AS_SYMBOL(rest->car);
        cons_t *body = AS_CONS(rest->cdr);
        value_t val = eval(vm, env, body->car);
        variable_add(vm, env, sym, val);
        return val;
    } else if (IS_CONS(rest->car)) { // (define (<name> <params...>) <body...>)
        symbol_t *sym = AS_SYMBOL(AS_CONS(rest->car)->car);
        value_t params = AS_CONS(rest->car)->cdr;
        cons_t *body = AS_CONS(rest->cdr);
        function_t *func = function_new(vm, env, params, PTR_VAL(body));
        value_t val = eval(vm, env, PTR_VAL(func));
        variable_add(vm, env, sym, val);
        return val;
     } else {
        error_runtime(vm, "define: is wrong - second argument has to be either a list or a symbol!");
        return NIL_VAL;
     }
}

static value_t lambda(vm_t *vm, env_t *env, value_t args) {
    // (lambda (<params...>) <body...>)

    if (IS_NIL(AS_CONS(args)->car)) { // no parameters
        value_t body = AS_CONS(args)->cdr;
        function_t *func = function_new(vm, env, NIL_VAL, body);
        return PTR_VAL(func);
    }

    for (cons_t *cons = AS_CONS(AS_CONS(args)->car); ; cons = AS_CONS(cons->cdr)) {
        if (!IS_SYMBOL(cons->car)) {
            error_runtime(vm, "lambda: all parameters must be symbols!");
        } else if (!IS_NIL(cons->cdr) && !IS_CONS(cons->cdr)) {
            error_runtime(vm, "lambda: parameter list must not be dotted");
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
        error_runtime(vm, "if: not enough args (has %d)", cons_len(args));
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
        error_runtime(vm, "eq?: args (has %d) != 2", cons_len(args));
    }

    value_t car = eval(vm, env, AS_CONS(args)->car);
    value_t cdr = eval(vm, env, AS_CONS(AS_CONS(args)->cdr)->car);

    value_t cons = cons_fn(vm, car, cdr);
    return cons;
}

static value_t builtin_is_cons(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        error_runtime(vm, "cons?: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    return BOOL_VAL(IS_NIL(a) || IS_CONS(a));
}

static value_t builtin_car(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        error_runtime(vm, "car: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    if (IS_NIL(a)) {
        error_runtime(vm, "car: cannot give car of NIL");
    }
    return AS_CONS(a)->car;
}

static value_t builtin_cdr(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) != 1) {
        error_runtime(vm, "cdr: args (has %d) != 1", cons_len(args));
    }
    value_t a = AS_CONS(eargs)->car;
    if (IS_NIL(a)) {
        error_runtime(vm, "cdr: cannot give cdr of NIL");
    }
    return AS_CONS(a)->cdr;
}

static value_t builtin_write(vm_t *vm, env_t *env, value_t args) {
    write(stdout, eval_list(vm, env, args));
    fprintf(stdout, "\n");
    return NIL_VAL;
}

static value_t builtin_or(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        error_runtime(vm, "or: not enough args (has %d)", cons_len(eargs));
        return NIL_VAL;
    }
    bool result = AS_BOOL(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_BOOL(cons->car)) {
            error_runtime(vm, "or: car is not a bool!");
            return NIL_VAL;
        }
        result |= AS_BOOL(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return BOOL_VAL(result);
}

static value_t builtin_and(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    if (cons_len(eargs) < 2) {
        error_runtime(vm, "and: not enough args (has %d)", cons_len(eargs));
        return NIL_VAL;
    }
    bool result = AS_BOOL(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(eargs); ; cons = AS_CONS(cons->cdr)){
        if (!IS_BOOL(cons->car)) {
            error_runtime(vm, "and: car is not a bool!");
            return NIL_VAL;
        }
        result &= AS_BOOL(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return BOOL_VAL(result);
}

/* *** */
#ifdef DEBUG
static value_t builtin_gc(vm_t *vm, env_t *env, value_t args) {
    vm_gc(vm);
    return NIL_VAL;
}

static value_t builtin_env(vm_t *vm, env_t *env, value_t args) {
    env_t *e = env;
    while (e != NULL) {
        write(stdout, e->variables);
        fprintf(stdout, "\n");
        e = e->up;
    }
    return NIL_VAL;
}
#endif
/* *** */

// This function is reaaaally unsafe. Please don't break it.
static value_t file_read(vm_t *vm, const char *filename) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *string = (char *) malloc(fsize + 1);
    size_t bytes_read = fread(string, fsize, 1, f);
    fclose(f);
    if (bytes_read != 1) {
        error_runtime(vm, "Could not read file %s!", filename);
        free(string);
        return NIL_VAL;
    }
    string[fsize] = '\0';

    value_t val = read_source(vm, string);
#ifdef DEBUG
    fprintf(stdout, "I read in %zu bytes: ", bytes_read);
    write(stdout, val);
    fprintf(stdout, "\n");
#endif
    free(string);
    return val;
}

static void stdlib_load(vm_t *vm, env_t *env, const char *filename) {
    value_t val = file_read(vm, filename);
    eval(vm, env, val);
}

static env_t *env_default(vm_t *vm) {
    env_t *env = env_new(vm, NIL_VAL, NULL);

    symbol_t *pi_sym = symbol_intern(vm, "pi", 2);
    value_t pi = NUM_VAL(3.1415);
    variable_add(vm, env, pi_sym, pi);

    primitive_add(vm, env, "+", 1, add);
    primitive_add(vm, env, "*", 1, multiply);
    primitive_add(vm, env, "-", 1, subtract);
    primitive_add(vm, env, ">", 1, gt);
    primitive_add(vm, env, "eq?", 3, eq);
    primitive_add(vm, env, "equal?", 6, equal);
    primitive_add(vm, env, "quote", 5, quote);
    primitive_add(vm, env, "list", 4, list);
    primitive_add(vm, env, "begin", 5, begin);
    primitive_add(vm, env, "define", 6, builtin_define);
    primitive_add(vm, env, "lambda", 6, lambda);
    primitive_add(vm, env, "if", 2, builtin_if);
    primitive_add(vm, env, "cons", 4, builtin_cons);
    primitive_add(vm, env, "cons?", 5, builtin_is_cons);
    primitive_add(vm, env, "car", 3, builtin_car);
    primitive_add(vm, env, "cdr", 3, builtin_cdr);
    primitive_add(vm, env, "write", 5, builtin_write);
    primitive_add(vm, env, "or", 2, builtin_or);
    primitive_add(vm, env, "and", 3, builtin_and);
#ifdef DEBUG
    primitive_add(vm, env, "gc", 2, builtin_gc);
    primitive_add(vm, env, "env", 3, builtin_env);
#endif
    stdlib_load(vm, env, "src/stdlib.scm");

    return env;
}

void repl(vm_t *vm, env_t *env) {
    fprintf(stdout, "|Scheme " SCM_VERSION_STRING " - REPL|\n|Use Ctrl+C to exit!|\n");
    char buf[512];

    while (true) {
        fprintf(stdout, ">> ");

        while (fgets(buf, 512, stdin) == NULL) {
            break;
        }
        value_t val = read_source(vm, buf);

        value_t result = eval(vm, env, val);

        write(stdout, result);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "Quiting!\n");
}

int main(int argc, char* argv[]) {
    scm_config_t config;
    scm_config_default(&config);

    config.error_fn = error_report;

    vm_t *vm = vm_new(&config);
    env_t *env = env_default(vm);

    if (argc == 1) repl(vm, env);
    if (argc == 2) {
        value_t val = file_read(vm, argv[1]);
        value_t result = eval(vm, env, val);

        fprintf(stdout, "Result: ");
        write(stdout, result);
        fprintf(stdout, "\n");
    }

    vm_free(vm);
    return 0;
}
