#include <lib/spinlock.h>
#include <simplejs/lib/thread.h>

void SIMPLEJS_PLATFORM_API simplejs_init_spinlock(simplejs_spinlock_t *spinlock)
{
    memclr(spinlock, sizeof(*spinlock));
}

static int64_t spinlock_get_thread_id()
{
    int64_t id = simplejs_get_current_thread_id() + 1;
    SIMPLEJS_ASSERT(id != 0);

    return id;
}

bool SIMPLEJS_PLATFORM_API simplejs_spinlock_acquire(simplejs_spinlock_t *spinlock, bool wait_flag)
{
    int64_t current_thread = spinlock_get_thread_id();

    do
    {
        int64_t expected_value = 0;

        if (simplejs_atomic_int64_exchange_if(&spinlock->thread, &expected_value, current_thread) ||
            expected_value == current_thread)
        {
            simplejs_atomic_int_add(&spinlock->count, 1);
            return true;
        }
    } while (wait_flag);

    return false;
}

void SIMPLEJS_PLATFORM_API simplejs_spinlock_release(simplejs_spinlock_t *spinlock)
{
    int count = simplejs_atomic_int_sub(&spinlock->count, 1) - 1;
    SIMPLEJS_ASSERT(count >= 0);

    if (count < 1)
        simplejs_atomic_int64_store(&spinlock->thread, 0);
}
