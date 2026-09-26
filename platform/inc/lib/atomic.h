#pragma once
#include "default.h"
#include <simplejs/lib/atomic.h>

#define platform_gen_atomic_int_load_decl(type, type_t) \
    type_t simplejs_platform_atomic_##type##_load(simplejs_atomic_##type##_t *atomic)

#define platform_gen_atomic_int_store_decl(type, type_t) \
    void simplejs_platform_atomic_##type##_store(simplejs_atomic_##type##_t *atomic, type_t value)

#define platform_gen_atomic_int_add_decl(type, type_t) \
    type_t simplejs_platform_atomic_##type##_add(simplejs_atomic_##type##_t *atomic, type_t value)

#define platform_gen_atomic_int_sub_decl(type, type_t)

#define platform_gen_atomic_int_exchange_if_decl(type, type_t) \
    bool simplejs_platform_atomic_##type##_exchange_if(simplejs_atomic_##type##_t *atomic, type_t *expected_value, type_t value)

#define platform_gen_atomic_int_all_func_decl(type, type_t) \
    _gen_atomic_int_all_func(type, type_t, platform_gen_atomic_int, decl)

_gen_atomic_int_all_type(platform_gen_atomic_int_all_func_decl);
