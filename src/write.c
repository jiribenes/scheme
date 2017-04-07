#include <stdio.h> // FILE, fprintf, stderr

#include "value.h"
#include "write.h"

static void write_cons(FILE *f, cons_t *cons) {
    fprintf(f, "(");
    cons_t *temp = cons;

    if (IS_NIL(temp->car)) {
        fprintf(f, "()"); //write(f, temp->car);
    }

    while (!IS_NIL(temp->car)) {
        write(f, temp->car);

        if (!IS_NIL(temp->cdr)) {
            fprintf(f, " ");
        } else {
            break;
        }

        if (!IS_CONS(temp->cdr)) {
            fprintf(f, ". ");
            write(f, temp->cdr);
            break;
        } else {
            temp = AS_CONS(temp->cdr);
        }
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
        fprintf(f, "#t");
    } else if (IS_FALSE(val)) {
        fprintf(f, "#f");
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
        } else if (IS_SYMBOL(val)) {
            symbol_t *sym = AS_SYMBOL(val);
            fprintf(f, "%s", sym->name);
        } else if (IS_PRIMITIVE(val)) {
            fprintf(f, "<primitive>");
        } else if (IS_FUNCTION(val)) {
            fprintf(f, "<function>");
        } else if (IS_ENV(val)) {
            env_t *env = (env_t*) AS_PTR(val);
            fprintf(f, "<environment> containing: ");
            write(f, env->variables);
            if (env->up != NULL) {
                fprintf(f, "\n up: ");
                write(f, PTR_VAL(env->up));
            }
        } else {
            fprintf(f, "<unknown ptrvalue>");
        }
    } else {
        fprintf(f, "<unknown value>");
    }
}
