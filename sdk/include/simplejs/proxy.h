#pragma once
#include "default.h"
#include "variable.h"

#define SIMPLEJS_PROXY_STD_FLAG_PERMA_LOCK (1 << 0)
#define SIMPLEJS_PROXY_STD_FLAG_READ_ONLY (1 << 1)

typedef void simplejs_raw_object_t;
typedef struct simplejs_proxy simplejs_proxy_t;

typedef struct simplejs_proxy_context
{
    simplejs_raw_object_t *pointer;
    uint16_t value;
} simplejs_proxy_context_t;

typedef struct simplejs_proxy_property
{
    char *name;
    simplejs_variable_t value;
} simplejs_proxy_property_t;

typedef struct simplejs_proxy_property_query
{
    simplejs_proxy_property_t property;
    void *_current_pointer;
    bool query_ended;
} simplejs_proxy_property_query_t;

// proxy function types
typedef simplejs_status_t (*simplejs_proxy_release_f)(simplejs_proxy_context_t context);

typedef simplejs_status_t (*simplejs_proxy_set_std_flags_f)(simplejs_proxy_context_t context, uint32_t std_flags);
typedef simplejs_status_t (*simplejs_proxy_clear_std_flags_f)(simplejs_proxy_context_t context, uint32_t std_flags);

typedef simplejs_status_t (*simplejs_proxy_lock_property_list_f)(simplejs_proxy_context_t context);
typedef simplejs_status_t (*simplejs_proxy_unlock_property_list_f)(simplejs_proxy_context_t context);
typedef simplejs_status_t (*simplejs_proxy_query_property_f)(simplejs_proxy_context_t context, simplejs_proxy_property_query_t *out);

typedef simplejs_status_t (*simplejs_proxy_get_property_value_f)(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out);
typedef simplejs_status_t (*simplejs_proxy_set_property_value_f)(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in);

typedef simplejs_status_t (*simplejs_proxy_delete_property_f)(simplejs_proxy_context_t context, simplejs_variable_t *property);

typedef simplejs_status_t (*simplejs_proxy_get_string_f)(simplejs_proxy_context_t context, char **out);

simplejs_status_t SIMPLEJS_API simplejs_alloc_proxy(simplejs_proxy_t **out);
void SIMPLEJS_API simplejs_free_proxy(simplejs_proxy_t *proxy);

void SIMPLEJS_API simplejs_init_proxy_property_query(simplejs_proxy_property_query_t *out);
void SIMPLEJS_API simplejs_delete_proxy_property_query(simplejs_proxy_property_query_t *out);

#define _simplejs_proxy_define_name_callback_decl(name) \
    void SIMPLEJS_API simplejs_proxy_define_##name##_callback(simplejs_proxy_t *proxy, simplejs_proxy_##name##_f callback)

_simplejs_proxy_define_name_callback_decl(release);

_simplejs_proxy_define_name_callback_decl(set_std_flags);
_simplejs_proxy_define_name_callback_decl(clear_std_flags);

_simplejs_proxy_define_name_callback_decl(lock_property_list);
_simplejs_proxy_define_name_callback_decl(unlock_property_list);
_simplejs_proxy_define_name_callback_decl(query_property);

_simplejs_proxy_define_name_callback_decl(get_property_value);
_simplejs_proxy_define_name_callback_decl(set_property_value);
_simplejs_proxy_define_name_callback_decl(delete_property);

_simplejs_proxy_define_name_callback_decl(get_string);

simplejs_status_t SIMPLEJS_API simplejs_proxy_release(simplejs_proxy_t *proxy, simplejs_proxy_context_t context);

simplejs_status_t SIMPLEJS_API simplejs_proxy_set_std_flags(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, uint32_t std_flags);
simplejs_status_t SIMPLEJS_API simplejs_proxy_clear_std_flags(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, uint32_t std_flags);

simplejs_status_t SIMPLEJS_API simplejs_proxy_lock_property_list(simplejs_proxy_t *proxy, simplejs_proxy_context_t context);
simplejs_status_t SIMPLEJS_API simplejs_proxy_unlock_property_list(simplejs_proxy_t *proxy, simplejs_proxy_context_t context);
simplejs_status_t SIMPLEJS_API simplejs_proxy_query_property(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_proxy_property_query_t *out);

simplejs_status_t SIMPLEJS_API simplejs_proxy_get_property_value(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out);
simplejs_status_t SIMPLEJS_API simplejs_proxy_set_property_value(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in);
simplejs_status_t SIMPLEJS_API simplejs_proxy_delete_property(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property);

simplejs_status_t SIMPLEJS_API simplejs_proxy_get_string(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, char **out);
