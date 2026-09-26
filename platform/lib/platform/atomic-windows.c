#include <windows.h>
#include <lib/atomic.h>

typedef LONG win32_int_t;
typedef LONGLONG win32_int64_t;

#define InterlockedAdd_int InterlockedAdd
#define InterlockedExchange_int InterlockedExchange
#define InterlockedCompareExchange_int InterlockedCompareExchange

#define InterlockedAdd_int64 InterlockedAdd64
#define InterlockedExchange_int64 InterlockedExchange64
#define InterlockedCompareExchange_int64 InterlockedCompareExchange64

#define platform_gen_atomic_int_load_impl(type, type_t)                                      \
    platform_gen_atomic_int_load_decl(type, type_t)                                          \
    {                                                                                        \
        return InterlockedCompareExchange_##type((volatile win32_##type##_t *)atomic, 0, 0); \
    }

#define platform_gen_atomic_int_store_impl(type, type_t)                        \
    platform_gen_atomic_int_store_decl(type, type_t)                            \
    {                                                                           \
        InterlockedExchange_##type((volatile win32_##type##_t *)atomic, value); \
    }

#define platform_gen_atomic_int_add_impl(type, type_t)                                                              \
    platform_gen_atomic_int_add_decl(type, type_t)                                                                  \
    {                                                                                                               \
        return (type_t)InterlockedAdd_##type((volatile win32_##type##_t *)atomic, (win32_##type##_t)value) - value; \
    }

#define platform_gen_atomic_int_sub_impl(type, type_t)

#define platform_gen_atomic_int_exchange_if_impl(type, type_t)                                                                                                          \
    platform_gen_atomic_int_exchange_if_decl(type, type_t)                                                                                                              \
    {                                                                                                                                                                   \
        type_t old_expected_value = *expected_value;                                                                                                                    \
                                                                                                                                                                        \
        *expected_value = (type_t)InterlockedCompareExchange_##type((volatile win32_##type##_t *)atomic, (win32_##type##_t)value, (win32_##type##_t)(*expected_value)); \
                                                                                                                                                                        \
        return old_expected_value == *expected_value;                                                                                                                   \
    }

#define platform_gen_atomic_int_all_func_impl(type, type_t) \
    _gen_atomic_int_all_func(type, type_t, platform_gen_atomic_int, impl)

_gen_atomic_int_all_type(platform_gen_atomic_int_all_func_impl);
