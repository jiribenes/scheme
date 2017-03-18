#include <stdio.h> // FILE, fprintf, stderr

#include "value.h"
#include "write.h"


void print(FILE *f, value_t val) {
    if (IS_NUM(val)) {
        fprintf(f, "%.14g", AS_NUM(val));
    } else if (IS_NIL(val)) {
        fprintf(f, "()");
    } else if (IS_TRUE(val)) {
        fprintf(f, "T");
    } else if (IS_PTR(val)) {
        if (IS_CONS(val)) {
            cons_t *cons = AS_CONS(val);
            print_cons(f, cons);
        } else if (IS_STRING(val)) {
            string_t *str = AS_STRING(val);

            fprintf(f, "\"");

            for (int i = 0; i < str->len; i++) {
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
    }
}

static void print_cons(FILE *f, cons_t *cons) {
    value_t car = cons->car;
    value_t cdr = cons->cdr;
    fprintf(f, "(");
    print(f, car);

    if (IS_CONS(cdr)) {
        cons_t *cons_cdr = AS_CONS(cdr);
        print_cons(f, cons_cdr);
    } else if (IS_NIL(cdr)) {
       return; 
    } else {
        fprintf(f, " . ");
        print(f, cdr);
    }
    fprintf(f, ")");
}
