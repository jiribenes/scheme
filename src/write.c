#include <math.h>   // isnan, isinf
#include <stdio.h>  // FILE, fprintf, stderr

#include "value.h"
#include "write.h"

static void write_cons(FILE *f, cons_t *cons) {
    // First check if the list is circular
    // (we don't want to recurse forever)
    int32_t len = cons_len(PTR_VAL(cons));
    if (len == -1) {
        fprintf(f, "#<circular list>");
        return;
    }

    value_t arg, iter;
    SCM_FOREACH (arg, cons, iter) {
        write(f, arg);

        if (IS_NIL(AS_CONS(iter)->cdr)) {
            return;
        } else if (IS_CONS(AS_CONS(iter)->cdr)) {
            fprintf(f, " ");
        } else {
            fprintf(f, " . ");
            write(f, AS_CONS(iter)->cdr);
            return;
        }
    }
}

static void write_string(FILE *f, string_t *str) {
    fprintf(f, "\"");

    for (uint32_t i = 0; i < str->len; i++) {
        char c = str->value[i];
        if (c == '\n') {
            fprintf(f, "\\n");
        } else if (c == '\\') {
            fprintf(f, "\\\\");
        } else if (c == '\"') {
            fprintf(f, "\\\"");
        } else {
            fprintf(f, "%c", c);
        }
    }

    fprintf(f, "\"");
}

static void write_number(FILE *f, double d) {
    if (isnan(d)) {
        fprintf(f, "+nan.0");
    } else if (isinf(d)) {
        if (d > 0) {
            fprintf(f, "+inf.0");
        } else {
            fprintf(f, "-inf.0");
        }
    } else {
        fprintf(f, "%.14g", d);
    }
}

static void write_vector(FILE *f, vector_t *vec) {
    fprintf(f, "#(");
    for (uint32_t i = 0; i < vec->count; i++) {
        write(f, vec->data[i]);
        if (i + 1 != vec->count) {
            fprintf(f, " ");
        }
    }
    fprintf(f, ")");
}

// write val as a s-expr
void write(FILE *f, value_t val) {
    if (IS_NUM(val)) {
        write_number(f, AS_NUM(val));
    } else if (IS_NIL(val)) {
        fprintf(f, "()");
    } else if (IS_TRUE(val)) {
        fprintf(f, "#t");
    } else if (IS_FALSE(val)) {
        fprintf(f, "#f");
    } else if (IS_UNDEFINED(val)) {
        fprintf(f, "#<undefined>");
    } else if (IS_VOID(val)) {
        fprintf(f, "#<void>");
    } else if (IS_EOF(val)) {
        fprintf(f, "#<eof>");
    } else if (IS_PTR(val)) {
        if (IS_CONS(val)) {
            cons_t *cons = AS_CONS(val);

            fprintf(f, "(");
            write_cons(f, cons);
            fprintf(f, ")");
        } else if (IS_STRING(val)) {
            string_t *str = AS_STRING(val);
            write_string(f, str);
        } else if (IS_SYMBOL(val)) {
            symbol_t *sym = AS_SYMBOL(val);
            fprintf(f, "%s", sym->name);
        } else if (IS_PRIMITIVE(val)) {
            // TODO: A lot of repetition going on here, can we shorten this?
            primitive_t *prim = AS_PRIMITIVE(val);
            fprintf(f, "#<primitive ");
            if (prim->name != NULL) {
                display(f, PTR_VAL(prim->name));
            } else {
                fprintf(f, "?");
            }
            fprintf(f, ">");
        } else if (IS_FUNCTION(val)) {
            fprintf(f, "#<function ");
            function_t *fn = AS_FUNCTION(val);
            if (fn->name != NULL) {
                display(f, PTR_VAL(fn->name));
            } else {
                fprintf(f, "?");
            }
            fprintf(f, " ");
            write(f, fn->params);
            fprintf(f, ">");
        } else if (IS_MACRO(val)) {
            fprintf(f, "#<macro ");
            // TODO: If we ever have a AS_MACRO C macro, use it here
            function_t *mac = AS_FUNCTION(val);
            if (mac->name != NULL) {
                display(f, PTR_VAL(mac->name));
            } else {
                fprintf(f, "?");
            }
            fprintf(f, ">");
        } else if (IS_VECTOR(val)) {
            vector_t *vec = AS_VECTOR(val);
            write_vector(f, vec);
        } else if (IS_ENV(val)) {
            env_t *env = (env_t *) AS_PTR(val);
            fprintf(f, "#<");
            if (env == NULL) {
                fprintf(f, "top level ");
            }
            fprintf(f, "environment>");
        } else {
            fprintf(f, "#<unknown ptrvalue>");
        }
    } else {
        fprintf(f, "#<unknown value>");
    }
}

// display val as a s-expr
void display(FILE *f, value_t val) {
    // void is not printed with display
    if (!IS_STRING(val) && !IS_VOID(val)) {
        write(f, val);
    } else if (IS_STRING(val)) {
        string_t *str = AS_STRING(val);
        fprintf(f, "%.*s", str->len, str->value);
    }
}
