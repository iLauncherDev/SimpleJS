#include <lib/spinlock.h>

void SIMPLEJS_PLATFORM_API simplejs_init_spinlock(simplejs_spinlock_t *spinlock)
{
    simplejs_atomic_int_store(spinlock, false);
}

bool SIMPLEJS_PLATFORM_API simplejs_spinlock_acquire(simplejs_spinlock_t *spinlock, bool wait_flag)
{
    do
    {
        int expected_value = false;

        if (simplejs_atomic_int_exchange_if(spinlock, &expected_value, true))
            return true;
    } while (wait_flag);

    return false;
}

void SIMPLEJS_PLATFORM_API simplejs_spinlock_release(simplejs_spinlock_t *spinlock)
{
    simplejs_atomic_int_store(spinlock, false);
}
