#pragma once
#include "../default.h"
#include <simplejs/object.h>

#ifdef __cplusplus
extern "C" {
#endif

simplejs_status_t SIMPLEJS_API simplejs_create_builtin_dynamic_string(simplejs_object_t **out);
simplejs_status_t SIMPLEJS_API simplejs_builtin_dynamic_string_get_size(simplejs_object_t *object, size_t *out_valid_size, size_t *out_allocated_size);
simplejs_status_t SIMPLEJS_API simplejs_builtin_dynamic_string_write(simplejs_object_t *object, const char *string, size_t offset);

#ifdef __cplusplus
}
#endif
