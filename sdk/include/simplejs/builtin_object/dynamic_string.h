#pragma once
#include "../default.h"
#include <simplejs/object.h>

#ifdef __cplusplus
extern "C" {
#endif

simplejs_status_t SIMPLEJS_API simplejs_create_builtin_dynamic_string(simplejs_object_t **out, char *string);
simplejs_status_t SIMPLEJS_API simplejs_builtin_dynamic_string_write(simplejs_object_t *object, char *string, size_t offset);

#ifdef __cplusplus
}
#endif
