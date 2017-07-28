#include <math.h>    // fmod
#include <stdarg.h>  // va_list
#include <stdio.h>   // FILE

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

static value_t add(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return NUM_VAL(0.0F);
    }
    value_t eargs = eval_list(vm, env, args);
    double result = 0.0F;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NUM(arg)) {
            error_runtime(vm, "+: argument is not a number!");
            return NIL_VAL;
        }
        result += AS_NUM(arg);
    }
    return NUM_VAL(result);
}

static value_t multiply(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return NUM_VAL(1.0F);
    }
    value_t eargs = eval_list(vm, env, args);
    double result = 1.0F;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NUM(arg)) {
            error_runtime(vm, "*: argument is not a number!");
            return NIL_VAL;
        }
        result *= AS_NUM(arg);
    }
    return NUM_VAL(result);
}

static value_t subtract(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "-", args, 1, true);
    double result = 0.0F;
    value_t eargs = eval_list(vm, env, args);
    value_t arg = AS_CONS(eargs)->car;
    result = AS_NUM(arg);
    if (IS_NIL(AS_CONS(eargs)->cdr)) {
        return NUM_VAL(-result);
    }
    value_t iter;
    SCM_FOREACH (arg, AS_CONS(AS_CONS(eargs)->cdr), iter) {
        if (!IS_NUM(arg)) {
            error_runtime(vm, "-: argument is not a number!");
            return NIL_VAL;
        }
        result -= AS_NUM(arg);
    }
    return NUM_VAL(result);
}

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

// TODO: we shouldn't need to remember <prev>,
//       we just need first...
static value_t gt(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return TRUE_VAL;
    }
    value_t eargs = eval_list(vm, env, args);
    value_t prev = NIL_VAL;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NUM(arg)) {
            error_runtime(vm, ">: argument is not a number!");
            return NIL_VAL;
        }
        if (!IS_NIL(prev)) {
            // if we find a pair where the '>' relation doesn't hold,
            // return false
            if (AS_NUM(prev) <= AS_NUM(arg)) {
                return FALSE_VAL;
            }
        }
        prev = arg;
    }
    return TRUE_VAL;
}

static value_t num_equal(vm_t *vm, env_t *env, value_t args) {
    if (IS_NIL(args)) {
        return TRUE_VAL;
    }
    value_t eargs = eval_list(vm, env, args);
    value_t prev = NIL_VAL;
    value_t arg, iter;
    SCM_FOREACH (arg, AS_CONS(eargs), iter) {
        if (!IS_NUM(arg)) {
            error_runtime(vm, "=: argument is not a number!");
            return NIL_VAL;
        }
        if (!IS_NIL(prev)) {
            // if we find a pair where the '==' relation doesn't hold,
            // return false
            if (AS_NUM(prev) != AS_NUM(arg)) {
                return FALSE_VAL;
            }
        }
        prev = arg;
    }
    return TRUE_VAL;
}

/* *** core - predicates and equalities *** */

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

// type predicate functions
// they ARE NOT added to environment automatically
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

// FIXME I have a feeling that this is not the correct functionality...
static value_t quote(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "quote", args, 1, false);
    return AS_CONS(args)->car;
}

static value_t builtin_list(vm_t *vm, env_t *env, value_t args) {
    return eval_list(vm, env, args);
}

static value_t builtin_define(vm_t *vm, env_t *env, value_t args) {
    cons_t *rest = AS_CONS(args);

    if (IS_SYMBOL(rest->car)) {  // (define <name> <body...>)
        symbol_t *sym = AS_SYMBOL(rest->car);
        cons_t *body = AS_CONS(rest->cdr);
        value_t val = eval(vm, env, body->car);
        variable_add(vm, env, sym, val);
        return val;
    } else if (IS_CONS(rest->car)) {  // (define (<name> <params...>) <body...>)
        symbol_t *sym = AS_SYMBOL(AS_CONS(rest->car)->car);
        value_t params = AS_CONS(rest->car)->cdr;
        cons_t *body = AS_CONS(rest->cdr);
        function_t *func = function_new(vm, env, params, PTR_VAL(body));
        value_t val = eval(vm, env, PTR_VAL(func));
        variable_add(vm, env, sym, val);
        return val;
    } else {
        error_runtime(vm, "define: is wrong - second argument has to be"
                          "either a list or a symbol!");
        return NIL_VAL;
    }
}

static value_t lambda(vm_t *vm, env_t *env, value_t args) {
    // (lambda (<params...>) <body...>)

    if (IS_NIL(AS_CONS(args)->car)) {  // no parameters
        value_t body = AS_CONS(args)->cdr;
        function_t *func = function_new(vm, env, NIL_VAL, body);
        return PTR_VAL(func);
    }
    // TODO: Rewrite using SCM_FOREACH macro
    for (cons_t *cons = AS_CONS(AS_CONS(args)->car);;
         cons = AS_CONS(cons->cdr)) {
        if (!IS_SYMBOL(cons->car)) {
            error_runtime(vm, "lambda: all parameters must be symbols!");
            return NIL_VAL;
        } else if (!IS_NIL(cons->cdr) && !IS_CONS(cons->cdr)) {
            error_runtime(vm, "lambda: parameter list must not be dotted");
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
        value_t then = eval(vm, env, AS_CONS(AS_CONS(args)->cdr)->car);
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
    if (IS_NIL(a)) {
        error_runtime(vm, "car: cannot give car of NIL");
    }
    return AS_CONS(a)->car;
}

static value_t builtin_cdr(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "cdr", eargs, 1, false);
    value_t a = AS_CONS(eargs)->car;
    if (IS_NIL(a)) {
        error_runtime(vm, "cdr: cannot give cdr of NIL");
    }
    return AS_CONS(a)->cdr;
}

static value_t builtin_write(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "write", eargs, 1, false);
    write(stdout, AS_CONS(eargs)->car);
    fprintf(stdout, "\n");
    return NIL_VAL;
}

static value_t builtin_display(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "write", eargs, 1, false);
    display(stdout, AS_CONS(eargs)->car);
    fprintf(stdout, "\n");
    return NIL_VAL;
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
    return find_replace(env, sym, val);
    // error-handling provided by find_replace (for now...)
}

// TODO Allow functions or, and to be with arbitrary amount of elements
//      Look at the RxRS for the details
//      Rewrite using SCM_FOREACH macro
static value_t builtin_or(vm_t *vm, env_t *env, value_t args) {
    value_t eargs = eval_list(vm, env, args);
    arity_check(vm, "or", eargs, 2, true);
    bool result = AS_BOOL(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(eargs);; cons = AS_CONS(cons->cdr)) {
        if (!IS_BOOL(cons->car)) {
            error_runtime(vm, "or: argument is not a bool!");
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
    arity_check(vm, "and", eargs, 2, true);
    bool result = AS_BOOL(AS_CONS(eargs)->car);
    for (cons_t *cons = AS_CONS(eargs);; cons = AS_CONS(cons->cdr)) {
        if (!IS_BOOL(cons->car)) {
            error_runtime(vm, "and: argument is not a bool!");
            return NIL_VAL;
        }
        result &= AS_BOOL(cons->car);
        if (IS_NIL(cons->cdr)) {
            break;
        }
    }
    return BOOL_VAL(result);
}

static value_t builtin_eval(vm_t *vm, env_t *env, value_t args) {
    arity_check(vm, "eval", args, 1, false);
    value_t eargs = eval_list(vm, env, args);
    return eval(vm, env, AS_CONS(eargs)->car);
}

static value_t builtin_void(vm_t *vm, env_t *env, value_t args) {
    return VOID_VAL;
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

env_t *scm_env_default(vm_t *vm) {
    env_t *env = env_new(vm, NIL_VAL, NULL);

    symbol_t *pi_sym = symbol_intern(vm, "pi", 2);
    value_t pi = NUM_VAL(3.1415);
    variable_add(vm, env, pi_sym, pi);

    symbol_t *eof_sym = symbol_intern(vm, "eof", 3);
    variable_add(vm, env, eof_sym, EOF_VAL);
    symbol_t *undefined_sym = symbol_intern(vm, "undefined", 9);
    variable_add(vm, env, undefined_sym, UNDEFINED_VAL);

    primitive_add(vm, env, "+", 1, add);
    primitive_add(vm, env, "*", 1, multiply);
    primitive_add(vm, env, "-", 1, subtract);
    primitive_add(vm, env, "remainder", 9, builtin_rem);
    primitive_add(vm, env, ">", 1, gt);
    primitive_add(vm, env, "=", 1, num_equal);

    primitive_add(vm, env, "eq?", 3, eq);
    primitive_add(vm, env, "equal?", 6, equal);

    primitive_add(vm, env, "cons?", 5, builtin_is_cons);
    primitive_add(vm, env, "integer?", 8, builtin_is_integer);
    primitive_add(vm, env, "number?", 7, builtin_is_number);
    primitive_add(vm, env, "string?", 7, builtin_is_string);
    primitive_add(vm, env, "symbol?", 7, builtin_is_symbol);
    primitive_add(vm, env, "procedure?", 10, builtin_is_procedure);

    primitive_add(vm, env, "quote", 5, quote);
    primitive_add(vm, env, "list", 4, builtin_list);

    primitive_add(vm, env, "begin", 5, begin);
    primitive_add(vm, env, "define", 6, builtin_define);
    primitive_add(vm, env, "lambda", 6, lambda);
    primitive_add(vm, env, "if", 2, builtin_if);
    primitive_add(vm, env, "cons", 4, builtin_cons);
    primitive_add(vm, env, "car", 3, builtin_car);
    primitive_add(vm, env, "cdr", 3, builtin_cdr);
    primitive_add(vm, env, "write", 5, builtin_write);
    primitive_add(vm, env, "display", 7, builtin_display);
    primitive_add(vm, env, "set!", 4, builtin_set);
    primitive_add(vm, env, "or", 2, builtin_or);
    primitive_add(vm, env, "and", 3, builtin_and);

    primitive_add(vm, env, "eval", 4, builtin_eval);
    primitive_add(vm, env, "void", 4, builtin_void);
#ifdef DEBUG
    primitive_add(vm, env, "gc", 2, builtin_gc);
    primitive_add(vm, env, "env", 3, builtin_env);
#endif

    return env;
}
