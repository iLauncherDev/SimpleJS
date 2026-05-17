#include <lib/thread.h>
#include <pthread.h>

static void *thread_function(void *arg)
{
    uintptr_t return_value = 0;
    simplejs_thread_t *thread = arg;

    assert(thread != NULL);

    thread_call(thread, return_value);

    return (void *)return_value;
}

simplejs_status_t simplejs_platform_setup_thread(simplejs_thread_t *thread)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    pthread_t thread_context;
    if (pthread_create(&thread_context, NULL, thread_function, thread) != 0)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    thread->context = (void *)((uintptr_t)thread_context);

result:
    return status;
}

void simplejs_platform_destroy_thread(simplejs_thread_t *thread)
{
    pthread_join((pthread_t)((uintptr_t)thread->context), NULL);
}
