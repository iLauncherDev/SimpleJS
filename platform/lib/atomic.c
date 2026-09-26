#include <lib/atomic.h>

#define _gen_atomic_int_load_impl(type, type_t)                \
    _gen_atomic_int_load_decl(type, type_t)                    \
    {                                                          \
        return simplejs_platform_atomic_##type##_load(atomic); \
    }

#define _gen_atomic_int_store_impl(type, type_t)                \
    _gen_atomic_int_store_decl(type, type_t)                    \
    {                                                           \
        simplejs_platform_atomic_##type##_store(atomic, value); \
    }

#define _gen_atomic_int_add_impl(type, type_t)                       \
    _gen_atomic_int_add_decl(type, type_t)                           \
    {                                                                \
        return simplejs_platform_atomic_##type##_add(atomic, value); \
    }

#define _gen_atomic_int_sub_impl(type, type_t)                        \
    _gen_atomic_int_sub_decl(type, type_t)                            \
    {                                                                 \
        return simplejs_platform_atomic_##type##_add(atomic, -value); \
    }

#define _gen_atomic_int_exchange_if_impl(type, type_t)                                       \
    _gen_atomic_int_exchange_if_decl(type, type_t)                                           \
    {                                                                                        \
        return simplejs_platform_atomic_##type##_exchange_if(atomic, expected_value, value); \
    }

#define _gen_atomic_int_all_func_impl(type, type_t) \
    _gen_atomic_int_all_func(type, type_t, _gen_atomic_int, impl)

_gen_atomic_int_all_type(_gen_atomic_int_all_func_impl);
