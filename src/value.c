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
        value_t car = ((cons_t*) ptr)->car;
        value_t cdr = ((cons_t*) ptr)->cdr;
        if (IS_PTR(car)) {
            ptr_free(vm, AS_PTR(car));
        }
        if (IS_PTR(cdr)) {
            ptr_free(vm, AS_PTR(cdr));
        }
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
        return 0; //TODO: log error
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

