#pragma once
#include "../default.h"

typedef volatile int simplejs_atomic_int_t;
typedef volatile int64_t simplejs_atomic_int64_t;

#define _gen_atomic_int_load_decl(type, type_t)                \
    /*                                                         
     * Loads the current value of the atomic integer.          
     * The operation is atomic and safe for concurrent access. 
     */                                                        \
    int SIMPLEJS_PLATFORM_API simplejs_atomic_##type##_load(simplejs_atomic_##type##_t *atomic)

#define _gen_atomic_int_store_decl(type, type_t)               \
    /*                                                         
     * Stores a value into the atomic integer.
     * The operation is atomic and safe for concurrent access.
     */                                                        \
    void SIMPLEJS_PLATFORM_API simplejs_atomic_##type##_store(simplejs_atomic_##type##_t *atomic, type_t value)

#define _gen_atomic_int_add_decl(type, type_t)                  \
    /*                                                         
     * Performs an atomic add operation on a atomic integer.
     * Returns the previous value before the operation.
     */                                                         \
    type_t SIMPLEJS_PLATFORM_API simplejs_atomic_##type##_add(simplejs_atomic_##type##_t *atomic, type_t value)

#define _gen_atomic_int_sub_decl(type, type_t)                      \
    /*                                                         
     * Performs an atomic subtract operation on a atomic integer.
     * Returns the previous value before the operation.
     */                                                             \
    type_t SIMPLEJS_PLATFORM_API simplejs_atomic_##type##_sub(simplejs_atomic_##type##_t *atomic, type_t value)

#define _gen_atomic_int_exchange_if_decl(type, type_t)                                          \
    /*
     * Atomically compares the current value with expected_value and updates it to
     * value only if they are equal.
     *
     * On return, expected_value contains the previous value that was stored in atomic integer.
     * Returns true if the value was exchanged, otherwise false.
     */                                                                                         \
    bool SIMPLEJS_PLATFORM_API simplejs_atomic_##type##_exchange_if(simplejs_atomic_##type##_t *atomic, type_t *expected_value, type_t value)

#define _gen_atomic_int_all_func(type, type_t, prefix, suffix) \
    prefix##_load_##suffix(type, type_t);                      \
    prefix##_store_##suffix(type, type_t);                     \
    prefix##_add_##suffix(type, type_t);                       \
    prefix##_sub_##suffix(type, type_t);                       \
    prefix##_exchange_if_##suffix(type, type_t)

#define _gen_atomic_int_all_func_decl(type, type_t) \
    _gen_atomic_int_all_func(type, type_t, _gen_atomic_int, decl)

#define _gen_atomic_int_all_type(callback) \
    callback(int, int);                    \
    callback(int64, int64_t)

_gen_atomic_int_all_type(_gen_atomic_int_all_func_decl);
