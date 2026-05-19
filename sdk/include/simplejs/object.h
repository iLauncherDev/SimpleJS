#pragma once
#include "default.h"
#include "variable.h"
#include "proxy.h"

typedef enum
{
    SIMPLEJS_PROXY_PROPERTY_TYPE_NUMBER,
    SIMPLEJS_PROXY_PROPERTY_TYPE_STRING,
} simplejs_proxy_property_type_t;

typedef struct simplejs_object simplejs_object_t;

// object functions
simplejs_status_t SIMPLEJS_API simplejs_alloc_object(simplejs_raw_object_t *pointer, simplejs_proxy_t *proxy, simplejs_object_t **out);
void SIMPLEJS_API simplejs_free_object(simplejs_object_t *object);

void SIMPLEJS_API simplejs_object_lock_gc(simplejs_object_t *object);
void SIMPLEJS_API simplejs_object_unlock_gc(simplejs_object_t *object);

void SIMPLEJS_API simplejs_object_reference(simplejs_object_t *object);
void SIMPLEJS_API simplejs_object_dereference(simplejs_object_t *object);

// object proxy functions
simplejs_status_t SIMPLEJS_API simplejs_object_release(simplejs_object_t *object);

simplejs_status_t SIMPLEJS_API simplejs_object_lock_property_list(simplejs_object_t *object);
simplejs_status_t SIMPLEJS_API simplejs_object_unlock_property_list(simplejs_object_t *object);
simplejs_status_t SIMPLEJS_API simplejs_object_query_property(simplejs_object_t *object, simplejs_proxy_property_query_t *out);

simplejs_status_t SIMPLEJS_API simplejs_object_get_property_value(simplejs_object_t *object, simplejs_variable_t *property, simplejs_variable_t *out);
simplejs_status_t SIMPLEJS_API simplejs_object_set_property_value(simplejs_object_t *object, simplejs_variable_t *property, simplejs_variable_t *in);

simplejs_status_t SIMPLEJS_API simplejs_object_delete_property(simplejs_object_t *object, simplejs_variable_t *property);

simplejs_status_t SIMPLEJS_API simplejs_object_get_string(simplejs_object_t *object, char **out);
