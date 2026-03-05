#pragma once
#include "default.h"
#include <simplejs/thread.h>

typedef struct simplejs_thread
{
    void *context;

    simplejs_thread_callback_f callback;
    void *arg;
} simplejs_thread_t;

simplejs_status_t simplejs_platform_setup_thread(simplejs_thread_t *thread);
void simplejs_platform_destroy_thread(simplejs_thread_t *thread);
