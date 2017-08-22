#include <math.h>    // fmod
#include <stdarg.h>  // va_list
#include <stdio.h>   // FILE
#include <time.h>    // clock(), CLOCKS_PER_SECOND

#include "core.h"
#include "scheme.h"
#include "value.h"
#include "vm.h"
#include "write.h"

// Checks if there are exactly n arguments (if at_least is false)
//                  or at least n arguments (if at_least is true)
bool arity_check(vm_t *vm, const char *fn_name, value_t args, int n,
                 bool at_least) {
    int argc = cons_len(args);
    if (!at_least) {  // exactly
        if (argc != n) {
            error_runtime(vm, "%s: not enough args: %d expected, %d given!",
                          fn_name, n, argc);
            return false;
        }
    } else {  // at least
        if (argc < n) {
            error_runtime(vm, "%s: not enough args: >= %d expected, %d given!",
                          fn_name, n, argc);
            return false;
        }
    }
    return true;
}

/* /== -------------------------- ==\ */
/* *** core environment functions *** */
/* \== -------------------------- ==/ */

/* *** core - numbers *** */

// This macro creates unsafe operations builtin<op>
// as C functions 'builtin_<name>'.
// These still have to be put into scm_config_default to be registered!
#define BUILTIN_NUM_FN(name, op)                                            \
    static value_t builtin_##name(vm_t *vm, env_t *env, value_t args) {     \
        value_t eargs = eval_list(vm, env, args);                           \
        arity_check(vm, "builtin" #op, eargs, 2, false);                    \
        value_t a = AS_CONS(eargs)->car;                                    \
        value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;                      \
        if (!IS_NUM(a) || !IS_NUM(b)) {                                     \
            error_runtime(vm, "builtin" #op ": argument is not a number!"); \
            return NIL_VAL;                                                 \
        }                                                                   \
        return NUM_VAL(AS_NUM(a) op AS_NUM(b));                             \
    }

BUILTIN_NUM_FN(add, +)
BUILTIN_NUM_FN(mul, *)
BUILTIN_NUM_FN(sub, -)
BUILTIN_NUM_FN(div, /)

static value_t builtin_rem(vm_t *vm, env_t *env, value_t args) {
    // (remainder <n> <m>) => n `rem` m
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "remainder", args, 2, false);
    value_t n = AS_CONS(eargs)->car;
    value_t m = AS_CONS(AS_CONS(eargs)->cdr)->car;

    if (!IS_NUM(n) || !IS_NUM(m)) {
        error_runtime(vm, "remainder: argument is not a number!");
        return NIL_VAL;
    }

    return NUM_VAL(fmod(AS_NUM(n), AS_NUM(m)));
}

#define BUILTIN_NUM_COMP(name, op, scm_name)                                \
    static value_t builtin_num_##name(vm_t *vm, env_t *env, value_t args) { \
        value_t eargs = eval_list(vm, env, args);                           \
        arity_check(vm, scm_name, eargs, 2, false);                         \
        value_t a = AS_CONS(eargs)->car;                                    \
        value_t b = AS_CONS(AS_CONS(eargs)->cdr)->car;                      \
        if (!IS_NUM(a) || !IS_NUM(b)) {                                     \
            error_runtime(vm, #scm_name ": argument is not a number!");     \
            return NIL_VAL;                                                 \
        }                                                                   \
        return BOOL_VAL(AS_NUM(a) op AS_NUM(b));                            \
    }

BUILTIN_NUM_COMP(gt, >, "builtin>")
BUILTIN_NUM_COMP(lt, <, "builtin<")
BUILTIN_NUM_COMP(eq, ==, "builtin=")

/* *** core - types and predicates *** */

// Checks for eq? using the val_eq function from value.h
// BEWARE: It assumes transitivity (tries only a == b && b == c && c == d ...)
static value_t eq(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return TRUE_VAL;
    }
    value_t eargs = eval_list(vm, env, args);
    value_t prev = NIL_VAL;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NIL(prev)) {
            // if we find a pair where the 'eq?' relation doesn't hold,
            // return false
            if (!val_eq(prev, arg)) {
                return FALSE_VAL;
            }
        }
        prev = arg;
    }
    return TRUE_VAL;
}

// Checks for equal? using the val_equal function from value.h
// BEWARE: It assumes transitivity (tries only a == b && b == c && c == d ...)
static value_t equal(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return TRUE_VAL;
    }
    value_t eargs = eval_list(vm, env, args);
    value_t prev = NIL_VAL;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NIL(prev)) {
            // if we find a pair where the 'equal?' relation doesn't hold,
            // return false
            if (!val_equal(prev, arg)) {
                return FALSE_VAL;
            }
        }
        prev = arg;
    }
    return TRUE_VAL;
}

// these ARE NOT added to environment automatically
// therefore any change here must be done also in <scm_env_default> fn
#define TYPE_PREDICATE_FN(type, is_type_fn)                                \
    static value_t builtin_is_##type(vm_t *vm, env_t *env, value_t args) { \
        value_t eargs = eval_list(vm, env, args);                          \
        arity_check(vm, #type "?", eargs, 1, false);                       \
        value_t a = AS_CONS(eargs)->car;                                   \
        return BOOL_VAL(is_type_fn(a));                                    \
    }

TYPE_PREDICATE_FN(cons, IS_CONS)
TYPE_PREDICATE_FN(integer, IS_INT)
TYPE_PREDICATE_FN(number, IS_NUM)
TYPE_PREDICATE_FN(string, IS_STRING)
TYPE_PREDICATE_FN(symbol, IS_SYMBOL)
TYPE_PREDICATE_FN(procedure, IS_PROCEDURE)
TYPE_PREDICATE_FN(vector, IS_VECTOR)
TYPE_PREDICATE_FN(environment, IS_ENV)

static value_t builtin_void(vm_t *vm, env_t *env, value_t args) {
    return VOID_VAL;
}

static value_t builtin_undefined(vm_t *vm, env_t *env, value_t args) {
    return UNDEFINED_VAL;
}

/* *** core - special constructs - flow control *** */

static value_t builtin_define(vm_t *vm, env_t *env, value_t args) {
    cons_t *rest = AS_CONS(args);

    if (IS_SYMBOL(rest->car)) {  // (define <name> <body>)
        symbol_t *sym = AS_SYMBOL(rest->car);
        value_t body = AS_CONS(rest->cdr)->car;
        value_t val = eval(vm, env, body);

        if (IS_FUNCTION(val)) {
            // If <body> is a nameless function, set its name to <name>
            // (we won't do any renaming...)
            function_t *func = AS_FUNCTION(val);
            if (func->name == NULL) {
                func->name = sym;
            }
        }
        variable_add(vm, env, sym, val);

        return VOID_VAL;
    } else if (IS_CONS(rest->car)) {  // (define (<name> <params...>) <body...>)
        symbol_t *sym = AS_SYMBOL(AS_CONS(rest->car)->car);
        value_t params = AS_CONS(rest->car)->cdr;
        cons_t *body = AS_CONS(rest->cdr);

        function_t *func = function_new(vm, env, params, PTR_VAL(body));
        func->name = sym;

        variable_add(vm, env, sym, PTR_VAL(func));

        return VOID_VAL;
    } else {
        error_runtime(vm, "define: is wrong - first argument has to be"
                          "either a list or a symbol!");
        return NIL_VAL;
    }
}

static value_t lambda(vm_t *vm, env_t *env, value_t args) {
    // (lambda (<params...>) <body...>)

    if (IS_NIL(AS_CONS(args)->car)) {
        // no parameters -> (lambda () <body...>)
        value_t body = AS_CONS(args)->cdr;
        function_t *func = function_new(vm, env, NIL_VAL, body);
        return PTR_VAL(func);
    }

    if (IS_SYMBOL(AS_CONS(args)->car)) {
        // variadic -> (lambda param <body...>)
        value_t param = AS_CONS(args)->car;
        value_t body = AS_CONS(args)->cdr;
        function_t *func = function_new(vm, env, param, body);
        return PTR_VAL(func);
    }

    // TODO: Rewrite using SCM_FOREACH macro
    for (cons_t *cons = AS_CONS(AS_CONS(args)->car);;
         cons = AS_CONS(cons->cdr)) {
        if (!IS_CONS(cons->cdr) && !IS_NIL(cons->cdr) && IS_SYMBOL(cons->cdr)) {
            // variadic (lambda (a b . rest) <body...>)
            break;
        }

        if (!IS_SYMBOL(cons->car)) {
            error_runtime(vm, "lambda: all parameters must be symbols!");
            return NIL_VAL;
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
    arity_check(vm, "if", args, 2, true);

    // First evaluate <condition>
    value_t condition = eval(vm, env, AS_CONS(args)->car);

    // if it's true, return <then>
    if (AS_BOOL(condition)) {
        value_t then = AS_CONS(AS_CONS(args)->cdr)->car;
        return eval(vm, env, then);
    }

    // If missing <otherwise> and <condition> is false, return false
    if (IS_NIL(AS_CONS(AS_CONS(args)->cdr)->cdr)) {
        return FALSE_VAL;
    }

    // Else do <otherwise>
    value_t otherwise = AS_CONS(AS_CONS(args)->cdr)->cdr;
    return begin(vm, env, otherwise);
}

static value_t builtin_set(vm_t *vm, env_t *env, value_t args) {
    // (set! <sym> <expr>)
    arity_check(vm, "set!", args, 2, false);
    value_t arg = AS_CONS(args)->car;
    if (!IS_SYMBOL(arg)) {
        error_runtime(vm, "set!: first argument must be a symbol!");
        return NIL_VAL;
    }

    symbol_t *sym = AS_SYMBOL(arg);

    arg = AS_CONS(AS_CONS(args)->cdr)->car;
    value_t val = eval(vm, env, arg);
    value_t result = find_replace(env, sym, val);

    if (IS_UNDEFINED(result)) {
        error_runtime(vm, "set!: assignment not allowed - %s is undefined!",
                      sym->name);
    }

    return VOID_VAL;
}

static value_t builtin_let(vm_t *vm, env_t *env, value_t args) {
    // (let ((a 10) (b 20) ...) <exprs...>)
    arity_check(vm, "let", args, 2, true);
    value_t bindings = AS_CONS(args)->car;
    if (!IS_CONS(bindings) && !IS_NIL(bindings)) {
        error_runtime(vm, "let: bindings (first argument) must be a list!");
        return UNDEFINED_VAL;
    }

    value_t vars = NIL_VAL;
    value_t vals = NIL_VAL;
    if (!IS_NIL(bindings)) {
        value_t binding, iter;
        SCM_FOREACH (binding, AS_CONS(bindings), iter) {
            if (!IS_CONS(binding) || cons_len(binding) != 2) {
                error_runtime(vm,
                              "let: binding is not of type (variable value)!");
                return UNDEFINED_VAL;
            }
            value_t var = AS_CONS(binding)->car;
            value_t val = AS_CONS(AS_CONS(binding)->cdr)->car;
            if (!IS_SYMBOL(var)) {
                error_runtime(vm,
                              "let: left side of the binding is not a symbol!");
                return UNDEFINED_VAL;
            }
            value_t temp = cons_fn(vm, var, vars);
            vars = temp;

            value_t val_eval = eval(vm, env, val);

            // If value in binding is an unnamed function,
            // set it's name to var in binding
            if (IS_FUNCTION(val_eval)) {
                function_t *func = AS_FUNCTION(val_eval);
                symbol_t *sym = AS_SYMBOL(var);
                if (func->name == NULL) {
                    func->name = sym;
                }
            }
            temp = cons_fn(vm, val_eval, vals);
            vals = temp;
        }
    }

    env_t *new_env = env_push(vm, env, vars, vals);
    value_t body = AS_CONS(args)->cdr;
    return begin(vm, new_env, body);
}

/* *** core - I/O *** */

static value_t builtin_write(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "write", eargs, 1, false);
    write(stdout, AS_CONS(eargs)->car);
    return VOID_VAL;
}

static value_t builtin_display(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "display", eargs, 1, false);
    display(stdout, AS_CONS(eargs)->car);
    return VOID_VAL;
}

static value_t builtin_newline(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "newline", args, 0, false);
    fprintf(stdout, "\n");
    return VOID_VAL;
}

static value_t builtin_read(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "read", args, 0, false);
    char line[1024];
    if (!fgets(line, 1024, stdin)) {
        return EOF_VAL;
    }
    return read_source(vm, line);
}

static value_t builtin_load(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "load", eargs, 1, false);
    value_t arg = AS_CONS(eargs)->car;
    if (!IS_STRING(arg)) {
        error_runtime(vm, "load: argument must be a string!");
        return UNDEFINED_VAL;
    }
    string_t *str = AS_STRING(arg);
    vm->config.load_fn(vm, env, str->value);
    return VOID_VAL;
}

/* *** core - or, and *** */

// TODO: When we have better macros, replace these :)

// Tries to evaluate arguments sequentially (short-circuits)
// If any arg is #t, returns #t, else #f
// (or #t (error "err")) => #t
static value_t builtin_or(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return FALSE_VAL;
    }
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(args), iter) {
        value_t earg = eval(vm, env, arg);
        if (!IS_BOOL(earg)) {
            error_runtime(vm, "or: argument is not a bool!");
            return UNDEFINED_VAL;
        }

        if (IS_TRUE(earg)) {
            return TRUE_VAL;
        }
    }
    return FALSE_VAL;
}

// Tries to evaluate arguments sequentially (short-circuits)
// If any arg is #f, returns #f, else #t
// (and #f (error "err")) => #f
static value_t builtin_and(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return TRUE_VAL;
    }
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(args), iter) {
        value_t earg = eval(vm, env, arg);
        if (!IS_BOOL(earg)) {
            error_runtime(vm, "and: argument is not a bool!");
            return UNDEFINED_VAL;
        }

        if (IS_FALSE(earg)) {
            return FALSE_VAL;
        }
    }
    return TRUE_VAL;
}

/* *** core - macros, eval *** */

static value_t define_macro(vm_t *vm, env_t *env, value_t args) {
    // (define-macro (<name> <params...>) <body...>)
    cons_t *rest = AS_CONS(args);
    if (!IS_CONS(rest->car)) {
        error_runtime(vm, "define-macro: first argument has to be a list!");
        return NIL_VAL;
    }

    symbol_t *sym = AS_SYMBOL(AS_CONS(rest->car)->car);
    value_t params = AS_CONS(rest->car)->cdr;
    cons_t *body = AS_CONS(rest->cdr);

    function_t *macro = macro_new(vm, env, params, PTR_VAL(body));
    macro->name = sym;

    variable_add(vm, env, sym, PTR_VAL(macro));

    return VOID_VAL;
}

static value_t builtin_eval(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "eval", eargs, 1, true);
    value_t first = AS_CONS(eargs)->car;
    if (cons_len(eargs) == 2) {
        value_t second = AS_CONS(AS_CONS(eargs)->cdr)->car;
        return eval(vm, (env_t *) AS_PTR(second), first);
    } else {
        return eval(vm, env, first);
    }
}

static value_t builtin_apply(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "apply", args, 2, false);
    value_t eargs = eval_list(vm, env, args);
    value_t fn = AS_CONS(eargs)->car;
    value_t fn_args = AS_CONS(AS_CONS(eargs)->cdr)->car;
    return apply(vm, env, fn, fn_args);
}

static value_t builtin_expand(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "expand", args, 1, false);
    return expand(vm, env, AS_CONS(args)->car);
}

static value_t builtin_gensym(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "gensym", args, 0, false);
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "g%u", vm->gensym_count++);
    return PTR_VAL(symbol_new(vm, buffer, 16));
}

static value_t quote(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "quote", args, 1, false);
    return AS_CONS(args)->car;
}

/* *** core - list functions *** */

static value_t builtin_cons(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "cons", args, 2, false);

    value_t car = eval(vm, env, AS_CONS(args)->car);
    value_t cdr = eval(vm, env, AS_CONS(AS_CONS(args)->cdr)->car);

    value_t cons = cons_fn(vm, car, cdr);
    return cons;
}

static value_t builtin_car(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "car", eargs, 1, false);
    value_t a = AS_CONS(eargs)->car;
    if (!IS_CONS(a)) {
        error_runtime(vm, "car: argument is not a cons cell!");
    }
    return AS_CONS(a)->car;
}

static value_t builtin_cdr(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "cdr", eargs, 1, false);
    value_t a = AS_CONS(eargs)->car;
    if (!IS_CONS(a)) {
        error_runtime(vm, "cdr: argument is not a cons cell!");
    }
    return AS_CONS(a)->cdr;
}

static value_t builtin_length(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "builtin-length", eargs, 1, false);
    return NUM_VAL(cons_len(AS_CONS(eargs)->car));
}


/* *** core - vector functions *** */

static value_t builtin_vec_length(vm_t *vm, env_t *env, value_t args) {
    // (vector-length <vec>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "vector-length", eargs, 1, false);
    value_t arg = AS_CONS(eargs)->car;
    if (!IS_VECTOR(arg)) {
        error_runtime(vm, "vector-length: argument must be a vector");
        return UNDEFINED_VAL;
    }
    return NUM_VAL(AS_VECTOR(arg)->count);
}

static value_t builtin_vec_ref(vm_t *vm, env_t *env, value_t args) {
    // (vector-ref! <vec> <k>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "vector-ref", eargs, 2, false);
    value_t first = AS_CONS(eargs)->car;
    value_t second = AS_CONS(AS_CONS(eargs)->cdr)->car;
    if (!IS_VECTOR(first)) {
        error_runtime(vm, "vector-ref: first argument must be a vector");
        return UNDEFINED_VAL;
    }
    vector_t *vec = AS_VECTOR(first);
    if (!IS_INT(second) ||
        !(AS_INT(second) >= 0 && AS_INT(second) < vec->count)) {
        error_runtime(
            vm, "vector-ref: second argument must be a valid integer in range");
        return UNDEFINED_VAL;
    }
    return vec->data[AS_INT(second)];
}

static value_t builtin_vec_set(vm_t *vm, env_t *env, value_t args) {
    // (vector-set! <vec> <k> <obj>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "vector-set!", eargs, 3, false);
    value_t first = AS_CONS(eargs)->car;
    value_t second = AS_CONS(AS_CONS(eargs)->cdr)->car;
    value_t third = AS_CONS(AS_CONS(AS_CONS(eargs)->cdr)->cdr)->car;
    if (!IS_VECTOR(first)) {
        error_runtime(vm, "vector-set!: first argument must be a vector");
        return UNDEFINED_VAL;
    }
    vector_t *vec = AS_VECTOR(first);
    if (!IS_INT(second) ||
        !(AS_INT(second) >= 0 && AS_INT(second) < vec->count)) {
        error_runtime(
            vm,
            "vector-set!: second argument must be a valid integer in range");
        return UNDEFINED_VAL;
    }
    vec->data[AS_INT(second)] = third;
    return VOID_VAL;
}

static value_t builtin_vec_make(vm_t *vm, env_t *env, value_t args) {
    // (make-vector <k> <fill>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "make-vector", eargs, 2, false);
    value_t first = AS_CONS(eargs)->car;
    value_t second = AS_CONS(AS_CONS(eargs)->cdr)->car;
    if (!IS_INT(first) || AS_INT(first) < 0) {
        error_runtime(
            vm, "make-vector: first argument must be a positive integer!");
        return UNDEFINED_VAL;
    }
    uint32_t count = AS_INT(first);
    vector_t *vec = vector_new(vm, count);
    for (uint32_t i = 0; i < count; i++) {
        vec->data[i] = second;
    }
    return PTR_VAL(vec);
}

/* *** core - other library functions *** */

static value_t builtin_error(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "error", eargs, 1, false);
    value_t arg = AS_CONS(eargs)->car;
    if (!IS_STRING(arg)) {
        error_runtime(vm, "error: argument must be a string");
        return UNDEFINED_VAL;
    }
    error_runtime(vm, AS_STRING(arg)->value);
    return VOID_VAL;
}

static value_t builtin_time(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "current-time", args, 0, false);
    return NUM_VAL((double) clock() / (CLOCKS_PER_SEC / 1000.0F));
}

// Exits the program - this procedure is really harsh
// TODO: Can we just stop the interpret loop and exit more gracefully?
static value_t builtin_exit(vm_t *vm, env_t *env, value_t args) {
    // (exit <status>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "exit", eargs, 1, false);
    value_t arg = AS_CONS(eargs)->car;
    if (!IS_INT(arg)) {
        error_runtime(vm, "exit: argument must be an integer");
        return UNDEFINED_VAL;
    }
    int exit_status = AS_INT(arg);
    vm_free(vm);
    exit(exit_status);
}

static value_t builtin_hash(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "hash", eargs, 1, false);

    value_t arg = AS_CONS(eargs)->car;
    if (IS_VAL(arg) || IS_STRING(arg) || IS_SYMBOL(arg)) {
        return NUM_VAL(hash_value(arg));
    }
    error_runtime(vm, "hash: cannot hash non-immutable type");
    return UNDEFINED_VAL;
}

/* *** debug functions *** */

#if DEBUG

static value_t builtin_env_cur(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "current-environment", args, 0, false);
    return PTR_VAL(env);
}

static value_t builtin_env_top(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "top-level-environment", args, 0, false);
    return PTR_VAL(vm->top_env);
}

static value_t builtin_env_vars(vm_t *vm, env_t *env, value_t args) {
    // (environment-variables <env>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "environment-variables", eargs, 1, false);
    if (!IS_ENV(AS_CONS(eargs)->car)) {
        error_runtime(vm,
                      "environment-variables: argument must be an environment");
        return UNDEFINED_VAL;
    }
    return AS_ENV(AS_CONS(eargs)->car)->variables;
}

static value_t builtin_env_up(vm_t *vm, env_t *env, value_t args) {
    // (environment-parent <env>)
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "environment-parent", eargs, 1, false);
    if (!IS_ENV(AS_CONS(eargs)->car)) {
        write(stderr, AS_CONS(eargs)->car);
        error_runtime(vm,
                      "environment-parent: argument must be an environment");
        return UNDEFINED_VAL;
    }
    env_t *e = AS_ENV(AS_CONS(eargs)->car);
    if (e->up == NULL) {
        return NIL_VAL;
    }
    return PTR_VAL(e->up);
}

static value_t builtin_gc(vm_t *vm, env_t *env, value_t args) {
    vm_gc(vm);
    return NIL_VAL;
}
#endif

/* *** DEFAULT ENVIRONMENT *** */

env_t *scm_env_default(vm_t *vm) {
    env_t *env = env_new(vm, NIL_VAL, NULL);
    vm->top_env = env;

    /* numeric functions */
    symbol_t *pi_sym = symbol_intern(vm, "pi", 2);
    variable_add(vm, env, pi_sym, NUM_VAL(3.14159265358979323846));
    primitive_add(vm, env, "builtin+", 8, builtin_add);
    primitive_add(vm, env, "builtin*", 8, builtin_mul);
    primitive_add(vm, env, "builtin-", 8, builtin_sub);
    primitive_add(vm, env, "builtin/", 8, builtin_div);
    primitive_add(vm, env, "remainder", 9, builtin_rem);

    primitive_add(vm, env, "builtin>", 8, builtin_num_gt);
    primitive_add(vm, env, "builtin<", 8, builtin_num_lt);
    primitive_add(vm, env, "builtin=", 8, builtin_num_eq);

    /* types and predicates */
    primitive_add(vm, env, "eq?", 3, eq);
    primitive_add(vm, env, "equal?", 6, equal);

    primitive_add(vm, env, "cons?", 5, builtin_is_cons);
    primitive_add(vm, env, "integer?", 8, builtin_is_integer);
    primitive_add(vm, env, "number?", 7, builtin_is_number);
    primitive_add(vm, env, "string?", 7, builtin_is_string);
    primitive_add(vm, env, "symbol?", 7, builtin_is_symbol);
    primitive_add(vm, env, "procedure?", 10, builtin_is_procedure);
    primitive_add(vm, env, "vector?", 7, builtin_is_vector);
    primitive_add(vm, env, "environment?", 12, builtin_is_environment);
    primitive_add(vm, env, "void", 4, builtin_void);
    primitive_add(vm, env, "undefined", 9, builtin_undefined);
    symbol_t *eof_sym = symbol_intern(vm, "eof", 3);
    variable_add(vm, env, eof_sym, EOF_VAL);

    /* special constructs - flow control */
    primitive_add(vm, env, "begin", 5, begin);
    primitive_add(vm, env, "define", 6, builtin_define);
    primitive_add(vm, env, "lambda", 6, lambda);
    primitive_add(vm, env, "if", 2, builtin_if);
    primitive_add(vm, env, "set!", 4, builtin_set);
    primitive_add(vm, env, "let", 3, builtin_let);

    /* I/O */
    primitive_add(vm, env, "write", 5, builtin_write);
    primitive_add(vm, env, "display", 7, builtin_display);
    primitive_add(vm, env, "newline", 7, builtin_newline);
    primitive_add(vm, env, "read", 4, builtin_read);
    if (vm->config.load_fn != NULL) {
        primitive_add(vm, env, "load", 4, builtin_load);
    }

    /* or/and */
    primitive_add(vm, env, "or", 2, builtin_or);
    primitive_add(vm, env, "and", 3, builtin_and);

    /* macros, eval */
    primitive_add(vm, env, "define-macro", 12, define_macro);
    primitive_add(vm, env, "eval", 4, builtin_eval);
    primitive_add(vm, env, "apply", 5, builtin_apply);
    primitive_add(vm, env, "expand", 6, builtin_expand);
    primitive_add(vm, env, "gensym", 6, builtin_gensym);
    primitive_add(vm, env, "quote", 5, quote);

    /* list functions */
    primitive_add(vm, env, "cons", 4, builtin_cons);
    primitive_add(vm, env, "car", 3, builtin_car);
    primitive_add(vm, env, "cdr", 3, builtin_cdr);
    primitive_add(vm, env, "builtin-length", 14, builtin_length);

    /* vector functions */
    primitive_add(vm, env, "vector-length", 13, builtin_vec_length);
    primitive_add(vm, env, "vector-ref", 10, builtin_vec_ref);
    primitive_add(vm, env, "vector-set!", 11, builtin_vec_set);
    primitive_add(vm, env, "make-vector", 11, builtin_vec_make);

    /* other library functions */
    primitive_add(vm, env, "error", 5, builtin_error);
    primitive_add(vm, env, "current-time", 12, builtin_time);
    primitive_add(vm, env, "exit", 4, builtin_exit);
    primitive_add(vm, env, "hash", 4, builtin_hash);

#ifdef DEBUG
    primitive_add(vm, env, "current-environment", 19, builtin_env_cur);
    primitive_add(vm, env, "top-level-environment", 21, builtin_env_top);
    primitive_add(vm, env, "environment-variables", 21, builtin_env_vars);
    primitive_add(vm, env, "environment-parent", 18, builtin_env_up);

    primitive_add(vm, env, "gc", 2, builtin_gc);
#endif

    // Automatically loads the stdlib if a load function is present
    if (vm->config.load_fn != NULL) {
        vm->config.load_fn(vm, env, "src/stdlib.scm");
    }

    return env;
}
