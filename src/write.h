#ifndef _write_h
#define _write_h

#include <stdio.h>  // FILE

#include "config.h"
#include "value.h"  // value_t

void write(FILE *f, value_t val);

void display(FILE *f, value_t val);

#endif  // _write_h
