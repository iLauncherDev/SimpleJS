#pragma once
#include "../default.h"

#include <time.h>

#include <simplejs/lib/pool.h>
#include <simplejs/lib/thread.h>
#include <simplejs/lib/page.h>
#include <simplejs/lib/sleep.h>

typedef struct simplejs_pool_entry
{
    void *data;
    size_t data_size;

    size_t used_data_size;

    size_t *data_map;
    size_t data_map_entries;

    clock_t modification_time;

    simplejs_safe_list_entry_t safe_list_entry;
} simplejs_pool_entry_t;

struct simplejs_pool
{
    simplejs_spinlock_t main_lock;

    volatile bool kill_thread_request;

    size_t block_size;
    size_t total_allocations;

    double allocation_expiration_time;

    simplejs_safe_list_t used_alloc_list;
    simplejs_safe_list_t free_alloc_list;

    simplejs_thread_t *thread;
};
