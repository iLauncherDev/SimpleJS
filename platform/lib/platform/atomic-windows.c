#include <windows.h>
#include <lib/atomic.h>

int simplejs_platform_atomic_int_load(simplejs_atomic_int_t *atomic)
{
    return InterlockedCompareExchange((volatile LONG *)atomic, 0, 0);
}

void simplejs_platform_atomic_int_store(simplejs_atomic_int_t *atomic, int value)
{
    InterlockedExchange((volatile LONG *)atomic, value);
}

int simplejs_platform_atomic_int_add(simplejs_atomic_int_t *atomic, int value)
{
    return (int)InterlockedAdd((volatile LONG *)atomic, (LONG)value) - value;
}

bool simplejs_platform_atomic_int_exchange_if(simplejs_atomic_int_t *atomic, int *expected_value, int value)
{
    int old_expected_value = *expected_value;

    *expected_value = (int)InterlockedCompareExchange((volatile LONG *)atomic, (LONG)value, (LONG)*expected_value);

    return old_expected_value == *expected_value;
}
