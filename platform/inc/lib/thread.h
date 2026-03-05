#pragma once
#include "../default.h"
#include <simplejs/lib/thread.h>

typedef struct simplejs_thread
{
    atomic_bool is_free;
    void *context;

    simplejs_thread_callback_f callback;
    void *arg;
} simplejs_thread_t;

#define thread_call(thread, return_value)    \
    return_value = thread->callback(thread); \
    atomic_store_explicit(&thread->is_free, true, memory_order_release)

simplejs_status_t simplejs_platform_setup_thread(simplejs_thread_t *thread);
void simplejs_platform_destroy_thread(simplejs_thread_t *thread);
