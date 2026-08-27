#pragma once
#include "../object.h"

void SIMPLEJS_API simplejs_gc_event(bool ignore_expiration_time);
void SIMPLEJS_API simplejs_gc_add_object(simplejs_object_t *object);
