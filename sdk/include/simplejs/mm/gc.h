#pragma once
#include "../object.h"

#ifdef __cplusplus
extern "C" {
#endif

void SIMPLEJS_API simplejs_gc_event(bool ignore_expiration_time);
void SIMPLEJS_API simplejs_gc_add_object(simplejs_object_t *object);

#ifdef __cplusplus
}
#endif
