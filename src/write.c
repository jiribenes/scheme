#include <stdio.h> // FILE, fprintf, stderr

#include "value.h"
#include "write.h"

static void write_cons(FILE *f, cons_t *cons) {
    value_t car = cons->car;
    value_t cdr = cons->cdr;
    fprintf(f, "(");
    write(f, car);

    if (IS_CONS(cdr)) {
        cons_t *cons_cdr = AS_CONS(cdr);
        write_cons(f, cons_cdr);
    } else if (IS_NIL(cdr)) {
       return; 
    } else {
        fprintf(f, " . ");
        write(f, cdr);
    }
    fprintf(f, ")");
}

// write val as a s-expr
void write(FILE *f, value_t val) {
    if (IS_NUM(val)) {
        fprintf(f, "%.14g", AS_NUM(val));
    } else if (IS_NIL(val)) {
        fprintf(f, "()");
    } else if (IS_TRUE(val)) {
        fprintf(f, "T");
    } else if (IS_PTR(val)) {
        if (IS_CONS(val)) {
            cons_t *cons = AS_CONS(val);
            write_cons(f, cons);
        } else if (IS_STRING(val)) {
            string_t *str = AS_STRING(val);

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
        } else if (IS_VECTOR(val)) {
            vector_t *vec = AS_VECTOR(val);
            
            fprintf(f, "#(");

            for (uint32_t i = 0; i < vec->count; i++) {
                write(f, vec->data[i]);
                if (i != vec->count - 1) {
                    fprintf(f, " ");
                }
            }

            fprintf(f, ")");
        } else if (IS_SYMBOL(val)) {
            symbol_t *sym = AS_SYMBOL(val);
            fprintf(f, "%s", sym->name);
        }
    }
}
