#pragma once
#include "default.h"
#include "variable.h"

typedef void simplejs_raw_object_t;

typedef struct simplejs_proxy simplejs_proxy_t;

typedef struct simplejs_proxy_property
{
    char *name;
    simplejs_variable_t value;

    simplejs_list_entry_t list_entry;
} simplejs_proxy_property_t;

typedef struct simplejs_proxy_property_query
{
    simplejs_proxy_property_t property;
    void *_current_pointer;
    bool query_ended;
} simplejs_proxy_property_query_t;

// proxy function types
typedef simplejs_status_t (*simplejs_proxy_release_f)(simplejs_raw_object_t *pointer);

typedef simplejs_status_t (*simplejs_proxy_lock_property_list_f)(simplejs_raw_object_t *pointer);
typedef simplejs_status_t (*simplejs_proxy_unlock_property_list_f)(simplejs_raw_object_t *pointer);
typedef simplejs_status_t (*simplejs_proxy_query_property_f)(simplejs_raw_object_t *pointer, simplejs_proxy_property_query_t *out);

typedef simplejs_status_t (*simplejs_proxy_get_property_value_f)(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *out);
typedef simplejs_status_t (*simplejs_proxy_set_property_value_f)(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *in);

typedef simplejs_status_t (*simplejs_proxy_delete_property_f)(simplejs_raw_object_t *pointer, simplejs_variable_t *property);

typedef simplejs_status_t (*simplejs_proxy_get_string_f)(simplejs_raw_object_t *pointer, char **out);

simplejs_status_t SIMPLEJS_API simplejs_alloc_proxy(simplejs_proxy_t **out);
void SIMPLEJS_API simplejs_free_proxy(simplejs_proxy_t *proxy);

void SIMPLEJS_API simplejs_init_proxy_property_query(simplejs_proxy_property_query_t *out);
void SIMPLEJS_API simplejs_delete_proxy_property_query(simplejs_proxy_property_query_t *out);

#ifndef SIMPLEJS_CORE
#define _simplejs_proxy_set_name_callback(name) \
    void SIMPLEJS_API simplejs_proxy_set_##name##_callback(simplejs_proxy_t *proxy, simplejs_proxy_##name##_f callback)

_simplejs_proxy_set_name_callback(release);

_simplejs_proxy_set_name_callback(lock_property_list);
_simplejs_proxy_set_name_callback(unlock_property_list);
_simplejs_proxy_set_name_callback(query_property);

_simplejs_proxy_set_name_callback(get_property_value);
_simplejs_proxy_set_name_callback(set_property_value);
_simplejs_proxy_set_name_callback(delete_property);

_simplejs_proxy_set_name_callback(get_string);
#endif
