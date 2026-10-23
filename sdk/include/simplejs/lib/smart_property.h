#pragma once
#include "../default.h"
#include <simplejs/object.h>

bool SIMPLEJS_API simplejs_smart_property_reference(simplejs_object_t *object, simplejs_proxy_property_t *property);
bool SIMPLEJS_API simplejs_smart_property_dereference(simplejs_object_t *object, simplejs_proxy_property_t *property);

bool SIMPLEJS_API simplejs_smart_property_get(simplejs_object_t *object, simplejs_proxy_property_t *property, simplejs_variable_t *out);
bool SIMPLEJS_API simplejs_smart_property_set(simplejs_object_t *object, simplejs_proxy_property_t *property, simplejs_variable_t *in);
