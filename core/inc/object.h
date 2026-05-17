#pragma once
#include "default.h"
#include "number.h"
#include <simplejs/object.h>

struct simplejs_proxy
{
    simplejs_proxy_release_f f_release;

    simplejs_proxy_lock_property_list_f f_lock_property_list;
    simplejs_proxy_unlock_property_list_f f_unlock_property_list;
    simplejs_proxy_get_property_list_f f_get_property_list;

    simplejs_proxy_get_property_value_f f_get_property_value;
    simplejs_proxy_set_property_value_f f_set_property_value;

    simplejs_proxy_delete_property_f f_delete_property;

    simplejs_proxy_get_string_f f_get_string;
};

struct simplejs_object
{
    atomic_bool gc_lock;
    atomic_int reference_count;

    simplejs_raw_object_t *pointer;
    simplejs_proxy_t *proxy;

    simplejs_list_entry_t gc_list_entry;
};

// proxy functions
simplejs_status_t simplejs_proxy_release(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer);

simplejs_status_t simplejs_proxy_lock_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer);
simplejs_status_t simplejs_proxy_unlock_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer);
simplejs_status_t simplejs_proxy_get_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_list_entry_t **property_list);

simplejs_status_t simplejs_proxy_get_property_value(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *out);
simplejs_status_t simplejs_proxy_set_property_value(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *in);

simplejs_status_t simplejs_proxy_delete_property(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property);

simplejs_status_t simplejs_proxy_get_string(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, char **out);
