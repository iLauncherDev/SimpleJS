#pragma once
#include "default.h"
#include "number.h"
#include <simplejs/object.h>

#include <simplejs/lib/time.h>

struct simplejs_proxy
{
    simplejs_proxy_release_f f_release;

    simplejs_proxy_lock_property_list_f f_lock_property_list;
    simplejs_proxy_unlock_property_list_f f_unlock_property_list;
    simplejs_proxy_query_property_f f_query_property;

    simplejs_proxy_get_property_value_f f_get_property_value;
    simplejs_proxy_set_property_value_f f_set_property_value;

    simplejs_proxy_delete_property_f f_delete_property;

    simplejs_proxy_get_string_f f_get_string;
};

struct simplejs_object
{
    uint32_t flags;

    simplejs_spinlock_t gc_lock;
    atomic_int reference_count;
    double modification_time;

    simplejs_raw_object_t *pointer;
    simplejs_proxy_t *proxy;

    simplejs_safe_list_entry_t gc_list_entry;
};
