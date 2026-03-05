#pragma once
#include "../object.h"
#include <simplejs/mm/gc.h>

typedef struct simplejs_gc
{
    atomic_bool object_list_lock;
    simplejs_list_entry_t object_list;
} simplejs_gc_t;

#ifndef SIMPLEJS_API_IMPORT
simplejs_status_t simplejs_init_gc();
#endif
