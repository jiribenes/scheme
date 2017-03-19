#ifndef _value_h
#define _value_h

#include <stdint.h> // uint64_t, uintptr_t
#include <stdbool.h> // bool
#include <math.h> // trunc

#include "scheme.h"

// a generic value
typedef uint64_t value_t;

typedef enum {
    T_CONS,
    T_STRING,
    T_VECTOR,
    T_SYMBOL
} ptrvalue_type_t;

// ptrvalue is a heap allocated object
typedef struct _ptrvalue {
    ptrvalue_type_t type;
    bool gcmark;
    // next heap allocated object
    struct _ptrvalue *next;
} ptrvalue_t;

typedef struct {
    ptrvalue_t p;

    value_t car, cdr;
} cons_t;

typedef struct {
    ptrvalue_t p;

    uint32_t len, hash;
    //C99 only - flexible array
    char value[];
} string_t;

typedef struct {
    ptrvalue_t p;

    value_t *data;
    uint32_t count, capacity;
} vector_t;

typedef struct { 
    ptrvalue_t p;

    uint32_t len;
    // TODO: couldn't this be const/ regular char*?
    char name[];
} symbol_t;

// 1--------------------------------------------------------------- 
#define SIGN_BIT ((uint64_t) 1 << 63)
// -1111111111111--------------------------------------------------
#define QUIET_NAN ((uint64_t) 0x7ffc000000000000)

#define IS_NUM(val) (((val) & QUIET_NAN) != QUIET_NAN)
#define IS_PTR(val) (((val) & (QUIET_NAN | SIGN_BIT)) == (QUIET_NAN | SIGN_BIT))

#define IS_TRUE(val) (val == TRUE_VAL)
#define IS_NIL(val) (val == NIL_VAL)
#define IS_INT(val) (IS_NUM(val) && (trunc(val) == val))
#define IS_DOUBLE(val) (IS_NUM(val) && !(trunc(val) == val))

#define IS_CONS(val) (val_is_ptr(val, T_CONS))
#define IS_STRING(val) (val_is_ptr(val, T_STRING))
#define IS_VECTOR(val) (val_is_ptr(val, T_VECTOR))
#define IS_SYMBOL(val) (val_is_ptr(val, T_SYMBOL))

// used for singletons
// --------------------------------------------------------------11
#define MASK_TAG (3)

#define GET_TAG(val) ((int) ((val) & MASK_TAG))

// NaN is reserved!
#define TAG_NAN (0)

#define TAG_NIL (1)
#define TAG_TRUE (2)
#define TAG_UNUSED (3)

#define NIL_VAL ((value_t)(uint64_t) (QUIET_NAN | TAG_NIL))
#define TRUE_VAL ((value_t)(uint64_t) (QUIET_NAN | TAG_TRUE))

// C value -> value
#define BOOL_VAL(b) (b ? TRUE_VAL : NIL_VAL)
#define NUM_VAL(num) (num_to_val(num))
#define PTR_VAL(ptr) (ptr_to_val((ptrvalue_t*) (ptr)))

// value -> C value
#define AS_BOOL(val) ((val) != NIL_VAL)
#define AS_PTR(val) ((ptrvalue_t*)(uintptr_t) ((val) & ~(SIGN_BIT | QUIET_NAN)))

#define AS_NUM(val) (val_to_num(val))

// doesn't check anything
#define AS_CONS(val) ((cons_t*) AS_PTR(val))
#define AS_STRING(val) ((string_t*) AS_PTR(val))
#define AS_VECTOR(val) ((vector_t*) AS_PTR(val))
#define AS_SYMBOL(val) ((symbol_t*) AS_PTR(val))

// GC (memory management) functions

void ptr_free(vm_t *vm, ptrvalue_t *ptr);

cons_t *cons_new(vm_t *vm);
string_t *string_new(vm_t *vm, const char *text, size_t len);
vector_t *vector_new(vm_t *vm, uint32_t count);
symbol_t *symbol_new(vm_t *vm, const char *name, size_t len);

void vector_insert(vm_t *vm, vector_t *vec, value_t val, uint32_t index);

typedef union {
	uint64_t bits;
	double num;
} value_conv_t;
// a conversion type from double to uint64_t

static inline double val_to_num(value_t val) {
    value_conv_t data;
    data.bits = val;
    return data.num;
}

static inline value_t num_to_val(double num) {
    value_conv_t data;
    data.num = num;
    return data.bits;
}

static inline value_t ptr_to_val(ptrvalue_t *ptr) {
    return (value_t) (SIGN_BIT | QUIET_NAN | (uint64_t)(uintptr_t) (ptr));
}

/* *** equality *** */

static inline bool val_is_ptr(value_t val, ptrvalue_type_t t) {
    return IS_PTR(val) && AS_PTR(val)->type == t;
}

// scm: equal?
// true for both ptrvalues and values when they are equal by value
static bool val_equal(value_t a, value_t b);

// scm: eq? or eqv?
// true for ptrvalues when they are identical
// true for values when they are equal
static inline bool val_eq(value_t a, value_t b) {
    return a == b;
}
#endif // _value_h 
