#include <stdatomic.h>
#include <lib/atomic.h>

int simplejs_platform_atomic_int_load(simplejs_atomic_int_t *atomic)
{
    return atomic_load_explicit((atomic_int *)atomic, memory_order_acquire);
}

void simplejs_platform_atomic_int_store(simplejs_atomic_int_t *atomic, int value)
{
    atomic_store_explicit((atomic_int *)atomic, value, memory_order_release);
}

int simplejs_platform_atomic_int_add(simplejs_atomic_int_t *atomic, int value)
{
    return (int)atomic_fetch_add_explicit((atomic_int *)atomic, value, memory_order_relaxed);
}

bool simplejs_platform_atomic_int_exchange_if(simplejs_atomic_int_t *atomic, int *expected_value, int value)
{
    return atomic_compare_exchange_weak_explicit((atomic_int *)atomic, expected_value, value, memory_order_acquire, memory_order_relaxed);
}
