#include <stdio.h> // fprintf, stderr
#include <stdlib.h> // strtod

#include "value.h"
#include "vm.h"
#include "read.h"

inline static bool is_space(char c) {
    return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}

inline static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

inline static bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// R5RS without following: . @
// http://www.schemers.org/Documents/Standards/R5RS/HTML/r5rs-Z-H-5.html#%_sec_2.1
inline static bool is_symbol(char c) {
    return is_digit(c) || is_letter(c) ||
        (c == '!') || (c == '$') || (c == '%') || (c == '&') ||
        (c == '*') || (c == '+') || (c == '-') || (c == ':') ||
        (c == '<') || (c == '=') || (c == '>') || (c == '?') ||
        (c == '^') || (c == '_') || (c == '~') || (c == '/');
}

/* *** */

static char next_char(reader_t* reader) {
    char c = *reader->cur;
    //fprintf(stdout, "Note: %c -> %c\n", *reader->cur, *(reader->cur + 1));
    reader->cur++;
    if (c == '\n') {
        reader->line++;
    }
    return c;
}

static char peek_next_char(reader_t* reader) {
    if (*reader->cur == '\0') return '\0';
    return *(reader->cur + 1);
}

static void eat_whitespace(reader_t* reader) {
    while (is_space(*reader->cur) && (*reader->cur) != '\0') {
        next_char(reader);
    }
    if ((*reader->cur) == ';') {
        while (*reader->cur != '\n' && *reader->cur != '\0') {
            next_char(reader);
        }
        next_char(reader);
    }

    if ((*reader->cur) == '\0') {
        reader->toktype = TOK_EOF;
    }
}

static void next_token(reader_t *reader) {
    eat_whitespace(reader);

    if ((*reader->cur) == '(') {
        reader->toktype = TOK_LPAREN;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == ')') {
        reader->toktype = TOK_RPAREN;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == '#') {
        reader->toktype = TOK_HASH;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == '.') {
        reader->toktype = TOK_DOT;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == '\'') {
        reader->toktype = TOK_QUOTE;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == '\"') {
        reader->toktype = TOK_STRING;
        reader->tokstart = reader->cur;
    } else if (is_digit(*reader->cur)) {
        reader->toktype = TOK_NUMBER;
        reader->tokstart = reader->cur;
    } else if (((*reader->cur) == '-') && is_digit(peek_next_char(reader))) {
        reader->toktype = TOK_NUMBER;
        reader->tokstart = reader->cur;
    } else if ((*reader->cur) == '\0') {
        reader->toktype = TOK_EOF;
        reader->tokstart = reader->cur;
    } else if (is_symbol(*reader->cur)) {
        reader->toktype = TOK_SYMBOL;
        reader->tokstart = reader->cur;
    } else {
        fprintf(stderr, "Error: Unknown token at line %d (starts with '%c')", reader->line, *reader->cur);
    }
}

static void read1(reader_t *reader);
static void read_list(reader_t *reader);

static void read_number(reader_t *reader){
    double d = strtod(reader->tokstart, NULL);

    while (is_digit(*reader->cur)) {
        next_char(reader);
    }

    if (*reader->cur == '.' && is_digit(peek_next_char(reader))) {
        next_char(reader);
        while (is_digit(*reader->cur)) {
            next_char(reader);
        }
    }

    if (*reader->cur == 'e' || *reader->cur == 'E') {
        next_char(reader);
        if (*reader->cur == '-' || *reader->cur == '+') {
            next_char(reader);
        }

        while (is_digit(*reader->cur)) {
            next_char(reader);
        }
    }

    reader->tokval = NUM_VAL(d);
}

static void read_quote(reader_t *reader) {
    next_char(reader);
    next_token(reader);

    symbol_t *s = symbol_intern(reader->vm, "quote", 5);

    read1(reader);
    value_t val = reader->tokval;

    reader->tokval = cons_fn(reader->vm, PTR_VAL(s), cons_fn(reader->vm, val, NIL_VAL));
}

// TODO: add escape characters
static void read_string(reader_t *reader) {
    next_char(reader);
    reader->tokstart = reader->cur;
    while ((*reader->cur) != '\"') {
        next_char(reader);
    }

    next_char(reader);
    size_t len = reader->cur - reader->tokstart - 1;

    string_t *str = string_new(reader->vm, reader->tokstart, len);
    reader->tokval = PTR_VAL(str);
}

static void read_symbol(reader_t *reader) {
    while (is_symbol(*reader->cur)) {
        next_char(reader);
    }

    size_t len = reader->cur - reader->tokstart;

    symbol_t *sym = symbol_intern(reader->vm, reader->tokstart, len);
    reader->tokval = PTR_VAL(sym);
}

static void read1(reader_t *reader) {
    eat_whitespace(reader);

    if (reader->toktype == TOK_EOF) {
        return;
    } else if (reader->toktype == TOK_LPAREN) {
        next_char(reader);
        read_list(reader);
    } else if (reader->toktype == TOK_NUMBER) {
        read_number(reader);
    } else if (reader->toktype == TOK_STRING) {
        read_string(reader);
    } else if (reader->toktype == TOK_QUOTE) {
        read_quote(reader);
    } else if (reader->toktype == TOK_HASH) {
        if (peek_next_char(reader) == 't') {
            next_char(reader);
            next_char(reader);
            reader->tokval = TRUE_VAL;
        } else if (peek_next_char(reader) == 'f') {
            next_char(reader);
            next_char(reader);
            reader->tokval = FALSE_VAL;
        } else {
            fprintf(stderr, "Invalid token: #t and #f are the only symbols that can begin with #\n");
        }
    } else if (reader->toktype == TOK_RPAREN) {
        fprintf(stderr, "Unexpected ')'\n");
        next_char(reader);
        return;
    } else if (reader->toktype == TOK_DOT) {
        fprintf(stderr, "Unexpected '.'\n");
        next_char(reader);
        return;
    } else if (reader->toktype == TOK_SYMBOL) {
        read_symbol(reader);
        return;
    }

    next_token(reader);
}

static void read_list(reader_t *reader) {
    eat_whitespace(reader);
    next_token(reader);
    if (reader->toktype == TOK_RPAREN) {
        next_char(reader);
        reader->tokval = NIL_VAL;
        return;
    } else if (reader->toktype == TOK_EOF) {
        fprintf(stderr, "Unexpected EOF while parsing\n");
        return;
    } else if (reader->toktype == TOK_DOT) {
        fprintf(stderr, "Unexpected dot in list\n");
        return;
    }

    read1(reader);
    eat_whitespace(reader);
    value_t val = reader->tokval;

    cons_t *head, *tail;
    head = tail = AS_CONS(cons_fn(reader->vm, val, NIL_VAL));

    while (reader->toktype != TOK_EOF) {
        next_token(reader);
        if (reader->toktype == TOK_RPAREN) {
            reader->tokval = PTR_VAL(head);
            next_char(reader);
            return;
        } else if (reader->toktype == TOK_EOF) {
            fprintf(stderr, "Unexpected EOF while parsing\n");
            return;
        } else if (reader->toktype == TOK_DOT) {
            next_char(reader);
            next_token(reader);

            read1(reader);

            tail->cdr = reader->tokval;
            reader->tokval = PTR_VAL(head);

            next_char(reader);
            return;
        }

        read1(reader);
        val = reader->tokval;

        tail->cdr = cons_fn(reader->vm, val, NIL_VAL);
        tail = AS_CONS(tail->cdr);
    }
}

value_t read_source(vm_t *vm, const char *source) {
    reader_t reader;

    reader.vm = vm;
    reader.source = source;
    reader.tokstart = source;
    reader.cur = source;
    reader.line = 1;
    reader.tokval = NIL_VAL;
    reader.toktype = TOK_NONE;

    vm->reader = &reader;

    /* new here */
    next_token(&reader);

    symbol_t *eof = symbol_intern(vm, "eof", 3);

    if (reader.toktype == TOK_EOF) {
        vm->curval = PTR_VAL(eof);
        reader.tokval = PTR_VAL(eof);
    } else {
        read1(&reader);
    }

    vm->curval = reader.tokval;
    return reader.tokval;
}
