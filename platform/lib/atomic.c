#include <lib/atomic.h>

int SIMPLEJS_PLATFORM_API simplejs_atomic_int_load(simplejs_atomic_int_t *atomic)
{
    return simplejs_platform_atomic_int_load(atomic);
}

void SIMPLEJS_PLATFORM_API simplejs_atomic_int_store(simplejs_atomic_int_t *atomic, int value)
{
    simplejs_platform_atomic_int_store(atomic, value);
}

int SIMPLEJS_PLATFORM_API simplejs_atomic_int_add(simplejs_atomic_int_t *atomic, int value)
{
    return simplejs_platform_atomic_int_add(atomic, value);;
}

int SIMPLEJS_PLATFORM_API simplejs_atomic_int_sub(simplejs_atomic_int_t *atomic, int value)
{
    return simplejs_atomic_int_add(atomic, -value);
}

bool SIMPLEJS_PLATFORM_API simplejs_atomic_int_exchange_if(simplejs_atomic_int_t *atomic, int *expected_value, int value)
{
    return simplejs_platform_atomic_int_exchange_if(atomic, expected_value, value);
}
