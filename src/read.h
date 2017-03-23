#ifndef _read_h
#define _read_h

#include "scheme.h"
#include "value.h"

typedef enum {
    TOK_EOF, TOK_NONE, TOK_LPAREN, TOK_RPAREN, TOK_DOT,
    TOK_QUOTE, TOK_STRING, TOK_SYMBOL, TOK_HASH, TOK_NUMBER 
} tok_type_t;

typedef struct {
    vm_t *vm;
    
    const char *source;
    const char *cur;
    const char *tokstart;

    int line;

    value_t tokval;
    tok_type_t toktype;

    char buffer[128];
} reader_t;

value_t read_source(vm_t *vm, const char *source);

#endif // _read_h
