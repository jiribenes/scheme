#include <stdio.h>
#include <string.h>  // memcpy, memcmp

#include "value.h"
#include "vm.h"  // vm_t, vm_realloc
#include "write.h"

static void ptr_init(vm_t *vm, ptrvalue_t *ptr, ptrvalue_type_t type) {
    ptr->type = type;
    ptr->gcmark = false;
    ptr->next = vm->head;
    vm->head = ptr;
}

void ptr_free(vm_t *vm, ptrvalue_t *ptr) {
    if (ptr->type == T_STRING) {
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_CONS) {  // TODO: is this a good idea?
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_SYMBOL) {
        symbol_t *ptr_sym = (symbol_t *) ptr;
        symbol_t *prev = NULL;
        symbol_t *s = vm->symbol_table;
        while (s != NULL) {
            if (s->len == ptr_sym->len &&
                memcmp(s->name, ptr_sym->name, s->len * sizeof(char)) == 0) {
                if (prev != NULL) {
                    prev->next = s->next;
                    vm->symbol_table = prev;
                } else {
                    vm->symbol_table = s->next;
                }
                break;
            }
            prev = s;
            s = s->next;
        }
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_PRIMITIVE) {
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_FUNCTION || ptr->type == T_MACRO) {
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_VECTOR) {
        vector_t *vec = (vector_t *) ptr;

        vm_realloc(vm, vec->data, 0, 0);

        vec->data = NULL;
        vec->capacity = 0;
        vec->count = 0;

        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_ENV) {
        vm_realloc(vm, ptr, 0, 0);
    }
}

/* *** HASHING *** */

/* Uses the pretty good FNV-1a hash, see
 * http://www.isthe.com/chongo/tech/comp/fnv/index.html */

const uint32_t HASH_PRIME = 16777619;
const uint32_t HASH_SEED = 2166136261u;

static uint32_t hash_ptr(ptrvalue_t *ptr) {
    if (ptr->type == T_STRING) {
        // all strings are hashed when created!
        return ((string_t *) ptr)->hash;
    } else {
        return 0;  // TODO: log error - mutable
    }
}
/*
inline static uint32_t hash_octet(uint8_t octet, uint32_t hash) {
    return (hash ^ octet) * 16777619;
}*/

static void hash_string(string_t *str) {
    uint32_t hash = HASH_SEED;

    for (uint32_t i = 0; i < str->len; i++) {
        hash ^= str->value[i];
        hash *= HASH_PRIME;
    }

    str->hash = hash;
}

static inline uint32_t hash_number(uint64_t num) {
    // TODO: is this really ideal for a double?
    const uint8_t *ptr = (const uint8_t *) &num;
    uint32_t hash = HASH_SEED;

    for (int i = 0; i < 8; i++) {  // I really hope the compiler can inline this
        hash ^= *ptr++;
        hash *= HASH_PRIME;
    }

    return hash;
}

uint32_t hash_value(value_t val) {
#if NANTAG
    if (IS_PTR(val)) {
        return hash_ptr(AS_PTR(val));
    } else {
        value_conv_t data;
        data.bits = val;
        return hash_number(data.bits);
    }
#else   // ! NANTAG
    switch (val.type) {
        case V_NIL:
            return 0;
        case V_TRUE:
            return 1;
        case V_FALSE:
            return 2;
        case V_UNDEFINED:
            return 3;
        case V_NUM:
            return hash_number(AS_NUM(val));
        case V_PTR:
            return hash_ptr(AS_PTR(val));
        default:
            return 0;
    }
#endif  // NANTAG
}

/* *** ptrvalue creating *** */
cons_t *cons_new(vm_t *vm) {
    cons_t *cons = (cons_t *) vm_realloc(vm, NULL, 0, sizeof(cons_t));

    ptr_init(vm, &cons->p, T_CONS);

    cons->car = NIL_VAL;
    cons->cdr = NIL_VAL;

    return cons;
}

string_t *string_new(vm_t *vm, const char *text, size_t len) {
    string_t *str = (string_t *) vm_realloc(
        vm, NULL, 0, sizeof(string_t) + sizeof(char) * (len + 1));

    ptr_init(vm, &str->p, T_STRING);

    str->len = (uint32_t) len;
    str->value[len] = '\0';

    if (len > 0 && text != NULL) {
        memcpy(str->value, text, len);
    }

    hash_string(str);

    return str;
}

symbol_t *symbol_new(vm_t *vm, const char *name, size_t len) {
    if (len == 0 || name == NULL) {  // TODO: better logging
        error_runtime(vm, "NULL or 0-length symbols are not allowed!");
        return NULL;
    }

    symbol_t *sym = (symbol_t *) vm_realloc(
        vm, NULL, 0, sizeof(symbol_t) + sizeof(char) * (len + 1));

    ptr_init(vm, &sym->p, T_SYMBOL);

    sym->len = (uint32_t) len;
    sym->name[len] = '\0';
    sym->next = NULL;

    memcpy(sym->name, name, len);

    return sym;
}

primitive_t *primitive_new(vm_t *vm, primitive_fn fn) {
    primitive_t *prim =
        (primitive_t *) vm_realloc(vm, NULL, 0, sizeof(primitive_t));

    ptr_init(vm, &prim->p, T_PRIMITIVE);

    prim->fn = fn;

    return prim;
}

function_t *function_new(vm_t *vm, env_t *env, value_t params, value_t body) {
    function_t *fn = (function_t *) vm_realloc(vm, NULL, 0, sizeof(function_t));

    ptr_init(vm, &fn->p, T_FUNCTION);

    fn->env = env;
    fn->params = params;
    fn->body = body;

    return fn;
}

function_t *macro_new(vm_t *vm, env_t *env, value_t params, value_t body) {
    function_t *macro =
        (function_t *) vm_realloc(vm, NULL, 0, sizeof(function_t));

    ptr_init(vm, &macro->p, T_MACRO);

    macro->env = env;
    macro->params = params;
    macro->body = body;

    return macro;
}

vector_t *vector_new(vm_t *vm, uint32_t count) {
    value_t *data = NULL;
    if (count > 0) {
        data = (value_t *) vm_realloc(vm, NULL, 0, sizeof(value_t) * count);
    }

    vector_t *vec = (vector_t *) vm_realloc(vm, NULL, 0, sizeof(vector_t));

    ptr_init(vm, &vec->p, T_VECTOR);

    vec->capacity = count;
    vec->count = count;
    vec->data = data;

    return vec;
}

env_t *env_new(vm_t *vm, value_t variables, env_t *up) {
    env_t *env = (env_t *) vm_realloc(vm, NULL, 0, sizeof(env_t));

    ptr_init(vm, &env->p, T_ENV);

    env->variables = variables;
    env->up = up;

    vm->env = env;

    return env;
}

/* *** UTILITY *** */

// This is the proper way to create interned symbols
// (so that we don't have two symbols that don't eq each other)
symbol_t *symbol_intern(vm_t *vm, const char *name, size_t len) {
    for (symbol_t *s = vm->symbol_table; s != NULL; s = s->next) {
        if (s->len == len && memcmp(s->name, name, len * sizeof(char)) == 0) {
            return s;
        }
    }

    symbol_t *sym = symbol_new(vm, name, len);

    sym->next = vm->symbol_table;
    vm->symbol_table = sym;

    return sym;
}

value_t cons_fn(vm_t *vm, value_t a, value_t b) {
    cons_t *result = cons_new(vm);
    result->car = a;
    result->cdr = b;
    return PTR_VAL(result);
}

// Finds the length of a cons cell
// Returns 0 if list is empty
//        -1 if list is circular
//         n if list's length is n
//      -2-n if list's length is n and is dotted
int32_t cons_len(value_t val) {
    int32_t len = 0;

    // Uses Floyd's cycle finding algorithm
    value_t fast, slow;
    fast = slow = val;

    while (true) {
        if (IS_NIL(fast)) {
            return len;
        }
        if (IS_CONS(fast) && !IS_NIL(AS_CONS(fast)->cdr) &&
            !IS_CONS(AS_CONS(fast)->cdr)) {
            return -2 - len;
        }
        fast = AS_CONS(fast)->cdr;
        ++len;
        if (IS_NIL(fast)) {
            return len;
        }
        if (IS_CONS(fast) && !IS_NIL(AS_CONS(fast)->cdr) &&
            !IS_CONS(AS_CONS(fast)->cdr)) {
            return -2 - len;
        }
        fast = AS_CONS(fast)->cdr;
        slow = AS_CONS(slow)->cdr;
        ++len;
        if (IS_EQ(fast, slow)) {
            return -1;
        }
    }
}

// Pushes <val> to the back of <vec>
// (equivalent to std::vector.push_back(val))
void vector_push(vm_t *vm, vector_t *vec, value_t val) {
    if (vec->count + 1 > vec->capacity) {
        uint32_t capacity = vec->capacity ? vec->capacity << 1 : 2;
        vec->data = (value_t *) vm_realloc(vm, vec->data,
                                           vec->capacity * sizeof(value_t),
                                           capacity * sizeof(value_t));
        vec->capacity = capacity;
    }
    vec->data[vec->count++] = val;
}

/* *** equality *** */
bool val_equal(value_t a, value_t b) {
    if (val_eq(a, b)) {
        return true;
    }

    if (IS_VAL(a) || IS_VAL(b)) {
        // if both are only plain values,
        // they should have been equal by <eq?>
        return false;
    }

    ptrvalue_t *pa = AS_PTR(a);
    ptrvalue_t *pb = AS_PTR(b);

    if (pa->type != pb->type) {
        return false;
    }

    if (pa->type == T_CONS) {
        cons_t *consa = (cons_t *) pa;
        cons_t *consb = (cons_t *) pb;

        return val_equal(consa->car, consb->car) &&
               val_equal(consa->cdr, consb->cdr);
    } else if (pa->type == T_STRING) {
        string_t *stra = (string_t *) pa;
        string_t *strb = (string_t *) pb;

        return stra->len == strb->len && stra->hash == strb->hash &&
               memcmp(stra->value, strb->value, stra->len * sizeof(char)) == 0;

    } else if (pa->type == T_SYMBOL) {
        symbol_t *syma = (symbol_t *) pa;
        symbol_t *symb = (symbol_t *) pb;
        if ((syma->len == symb->len) &&
            (memcmp(syma->name, symb->name, syma->len * sizeof(char)) == 0)) {
            // we should never get here! - two symbols with same name are eq?
            fprintf(stderr, "Error: symbol not interned!");
        }
        return false;
    } else if (pa->type == T_VECTOR) {
        vector_t *veca = (vector_t *) pa;
        vector_t *vecb = (vector_t *) pb;

        if (veca->count != vecb->count) {
            return false;
        }

        for (uint32_t i = 0; i < veca->count; i++) {
            if (!val_equal(veca->data[i], vecb->data[i])) {
                return false;
            }
        }
        return true;
    }

    return false;
}
