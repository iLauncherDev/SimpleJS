#pragma once
#include "../default.h"

typedef struct simplejs_thread simplejs_thread_t;

typedef uintptr_t (*simplejs_thread_callback_f)(struct simplejs_thread *);

#ifndef SIMPLEJS_CORE
struct simplejs_thread;
#endif

simplejs_status_t SIMPLEJS_API simplejs_create_thread(simplejs_thread_callback_f callback, void *arg, simplejs_thread_t **out);
void SIMPLEJS_API simplejs_destroy_thread(simplejs_thread_t *thread);
