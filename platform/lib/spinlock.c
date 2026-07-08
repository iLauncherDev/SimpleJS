#include <lib/spinlock.h>

void SIMPLEJS_PLATFORM_API simplejs_init_spinlock(simplejs_spinlock_t *spinlock)
{
    atomic_store_explicit(spinlock, false, memory_order_release);
}

bool SIMPLEJS_PLATFORM_API simplejs_spinlock_acquire(simplejs_spinlock_t *spinlock, bool wait_flag)
{
    do
    {
        bool expected_value = false;

        if (atomic_compare_exchange_weak_explicit(spinlock, &expected_value, true, memory_order_acquire, memory_order_relaxed))
            return true;
    } while (wait_flag);

    return false;
}

void SIMPLEJS_PLATFORM_API simplejs_spinlock_release(simplejs_spinlock_t *spinlock)
{
    atomic_store_explicit(spinlock, false, memory_order_release);
}
