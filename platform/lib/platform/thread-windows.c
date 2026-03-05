#include <lib/thread.h>
#include <process.h>
#include <windows.h>

static unsigned __stdcall thread_function(void *arg)
{
    uintptr_t return_value = 0;
    simplejs_thread_t *thread = arg;

    assert(thread != NULL);

    thread_call(thread, return_value);

    return (unsigned)return_value;
}

simplejs_status_t simplejs_platform_setup_thread(simplejs_thread_t *thread)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uint32_t thread_id;
    uintptr_t thread_handle = _beginthreadex(NULL, 0, thread_function, thread, 0, &thread_id);
    if (thread_handle == (uintptr_t)-1)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    thread->context = (void *)thread_handle;

result:
    return status;
}

void simplejs_platform_destroy_thread(simplejs_thread_t *thread)
{
    CloseHandle((HANDLE)thread->context);
}
