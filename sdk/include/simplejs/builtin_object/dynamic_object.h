#pragma once
#include "../default.h"
#include <simplejs/object.h>

#ifdef __cplusplus
extern "C" {
#endif

simplejs_status_t SIMPLEJS_API simplejs_create_builtin_dynamic_object(simplejs_object_t **out);
void SIMPLEJS_API simplejs_builtin_dynamic_object_set_prototype(simplejs_object_t *object, simplejs_variable_t *prototype_value);
void SIMPLEJS_API simplejs_builtin_dynamic_object_set_read_only(simplejs_object_t *object, bool read_only);

#ifdef __cplusplus
}
#endif
