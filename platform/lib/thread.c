#include <lib/thread.h>

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_create_thread(simplejs_thread_callback_f callback, void *arg, simplejs_thread_t **out)
{
    assert(callback != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_thread_t *thread = simplejs_hook_malloc(sizeof(*thread));
    if (!thread)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(thread, sizeof(*thread));

    thread->callback = callback;
    thread->arg = arg;

    status = simplejs_platform_setup_thread(thread);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    *out = thread;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (thread)
            simplejs_hook_mfree(thread);
    }

    return status;
}

void SIMPLEJS_PLATFORM_API simplejs_destroy_thread(simplejs_thread_t *thread)
{
    while (true)
    {
        bool expected_value = true;

        if (atomic_compare_exchange_weak_explicit(&thread->is_free, &expected_value, false, memory_order_acquire, memory_order_relaxed))
            break;
    }

    simplejs_platform_destroy_thread(thread);
    simplejs_hook_mfree(thread);
}
