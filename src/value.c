#include <string.h> // memcpy, memcmp
#include <stdio.h>

#include "vm.h" // vm_t, vm_realloc
#include "value.h" 

static void ptr_init(vm_t *vm, ptrvalue_t *ptr, ptrvalue_type_t type) {
    ptr->type = type;
    ptr->gcmark = false;
    ptr->next = vm->head;
    vm->head = ptr;
} 

void ptr_free(vm_t *vm, ptrvalue_t *ptr) {
    if (ptr->type == T_STRING) {
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_CONS) { //TODO: is this a good idea?
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_SYMBOL) {
        vm_realloc(vm, ptr, 0, 0);
    } else if (ptr->type == T_PRIMITIVE) {
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
        return ((string_t*) ptr)->hash;
    } else {
        return 0; //TODO: log error - mutable
    }      
}

inline static uint32_t hash_octet(uint8_t octet, uint32_t hash) {
    return (hash ^ octet) * 16777619;
}

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
    const uint8_t *ptr = (const uint8_t*) &num;
    uint32_t hash = HASH_SEED;

    for (int i = 0; i < 8; i++) { //I really hope the compiler can inline this
        hash ^= *ptr++;
        hash *= HASH_PRIME;
    }

    return hash;
}

static uint32_t hash_value(value_t val) {
    if (IS_PTR(val)) {
        return hash_ptr(AS_PTR(val));
    } else {
        value_conv_t data;
        data.bits = val;
        return hash_number(data.bits);
    } 
}
/* *** ptrvalue creating *** */
cons_t *cons_new(vm_t *vm) {
    cons_t *cons = (cons_t*) vm_realloc(vm, NULL, 0, sizeof(cons_t));

    ptr_init(vm, &cons->p, T_CONS);

    cons->car = NIL_VAL;
    cons->cdr = NIL_VAL;

    return cons;
}

string_t *string_new(vm_t *vm, const char *text, size_t len) {
    string_t *str = (string_t*) vm_realloc(vm, NULL, 0, sizeof(string_t) + sizeof(char) * (len + 1));

    ptr_init(vm, &str->p, T_STRING);

    str->len = (uint32_t)len;
    str->value[len] = '\0';

    if (len > 0 && text != NULL) {
        memcpy(str->value, text, len);
    }

    hash_string(str);
    
    return str; 
}

symbol_t *symbol_new(vm_t *vm, const char *name, size_t len) {
    if (len == 0 || name == NULL) { //TODO: better logging
        fprintf(stderr, "Error: NULL/0-len symbols are not allowed");
    }
   
    symbol_t *sym = (symbol_t*) vm_realloc(vm, NULL, 0, sizeof(symbol_t) + sizeof(char) * (len + 1));

    ptr_init(vm, &sym->p, T_SYMBOL);

    sym->len = (uint32_t) len;
    sym->name[len] = '\0';
    sym->next = NULL;

    memcpy(sym->name, name, len);

    return sym;
}

primitive_t *primitive_new(vm_t *vm, primitive_fn *fn) {
    primitive_t *prim = (primitive_t*) vm_realloc(vm, NULL, 0, sizeof(primitive_t));

    ptr_init(vm, &prim->p, T_PRIMITIVE);

    prim->fn = fn;

    return prim;
}

env_t *env_new(vm_t *vm, value_t variables, env_t *up) {
    // I am not very sure about this
    env_t *env = (env_t*) vm_realloc(vm, NULL, 0, sizeof(env_t) + sizeof(cons_t));

    ptr_init(vm, &env->p, T_ENV);
    
    env->variables = variables;
    env->up = up;

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

uint32_t cons_len(cons_t *cons) {
    uint32_t len = 0;

    while (true) { // this could be a little dangerous...
        if (IS_NIL(cons->cdr)) {
            return len;
        } else if (!IS_CONS(cons->cdr)) {
            fprintf(stderr, "Error: Can't find the length of a dotted list!");
        }
        cons = AS_CONS(cons->cdr);

        len++;
    }
}

/* *** equality *** */
static bool val_equal(value_t a, value_t b) {
    if (val_eq(a, b)) {
        return true;
    }

    if (!IS_PTR(a) || !IS_PTR(b)) {
        return false;
    }

    ptrvalue_t *pa = AS_PTR(a);
    ptrvalue_t *pb = AS_PTR(b);

    if (pa->type != pb->type) {
        return false;
    }

    if (pa->type == T_CONS) {
        cons_t *consa = (cons_t*) pa;
        cons_t *consb = (cons_t*) pb;
    
        return val_equal(consa->car, consb->car) && val_equal(consa->cdr, consb->cdr); 
    } else if (pa->type == T_STRING) {
        string_t *stra = (string_t*) pa;
        string_t *strb = (string_t*) pb;

        return stra->len == strb->len && stra->hash == strb->hash && 
            memcmp(stra->value, strb->value, stra->len * sizeof(char)) == 0;
    
    }  else if (pa->type == T_SYMBOL) { 
        symbol_t *syma = (symbol_t*) pa;
        symbol_t *symb = (symbol_t*) pb;
        if ((syma->len == symb->len) && (memcmp(syma->name, symb->name, syma->len * sizeof(char)) == 0)) {
            // we should never get here! - two symbols with same name are eq?
            fprintf(stderr, "Error: symbol not interned!");
            }
        return false;
    }

    return false;
}
