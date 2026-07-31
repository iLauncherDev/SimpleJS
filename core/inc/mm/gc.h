#pragma once
#include "../object.h"
#include <simplejs/mm/gc.h>

typedef struct simplejs_gc
{
    simplejs_safe_list_t object_list;
} simplejs_gc_t;

simplejs_status_t simplejs_init_gc();
