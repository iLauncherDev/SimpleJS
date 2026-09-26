#include <stdatomic.h>
#include <lib/atomic.h>

#define platform_gen_atomic_int_load_impl(type, type_t)                               \
    platform_gen_atomic_int_load_decl(type, type_t)                                   \
    {                                                                                 \
        return atomic_load_explicit((_Atomic(type_t) *)atomic, memory_order_acquire); \
    }

#define platform_gen_atomic_int_store_impl(type, type_t)                               \
    platform_gen_atomic_int_store_decl(type, type_t)                                   \
    {                                                                                  \
        atomic_store_explicit((_Atomic(type_t) *)atomic, value, memory_order_release); \
    }

#define platform_gen_atomic_int_add_impl(type, type_t)                                                 \
    platform_gen_atomic_int_add_decl(type, type_t)                                                     \
    {                                                                                                  \
        return (int)atomic_fetch_add_explicit((_Atomic(type_t) *)atomic, value, memory_order_relaxed); \
    }

#define platform_gen_atomic_int_sub_impl(type, type_t)

#define platform_gen_atomic_int_exchange_if_impl(type, type_t)                                                                                      \
    platform_gen_atomic_int_exchange_if_decl(type, type_t)                                                                                          \
    {                                                                                                                                               \
        return atomic_compare_exchange_strong_explicit((_Atomic(type_t) *)atomic, expected_value, value, memory_order_acquire, memory_order_relaxed); \
    }

#define platform_gen_atomic_int_all_func_impl(type, type_t) \
    _gen_atomic_int_all_func(type, type_t, platform_gen_atomic_int, impl)

_gen_atomic_int_all_type(platform_gen_atomic_int_all_func_impl);
