#ifndef _read_h
#define _read_h

#include "scheme.h"
#include "value.h"

// Types of tokens - no other are permitted
typedef enum {
    TOK_EOF, TOK_NONE, TOK_LPAREN, TOK_RPAREN, TOK_DOT,
    TOK_QUOTE, TOK_STRING, TOK_SYMBOL, TOK_HASH, TOK_NUMBER 
} tok_type_t;

// The main reader (parser) type
typedef struct {
    vm_t *vm;
    
    const char *source;
    const char *cur;
    const char *tokstart;

    int line;

    value_t tokval;
    tok_type_t toktype;
} reader_t;

// Reads the given source and returns a value.
// Creates a local reader_t on the inside.
value_t read_source(vm_t *vm, const char *source);

#endif // _read_h
