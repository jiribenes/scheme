#ifndef _write_h
#define _write_h

#include <stdio.h> // FILE

#include "value.h" // value_t

static void print_cons(FILE *f, cons_t *cons);

void print(FILE *f, value_t val); 

#endif // _write_h
