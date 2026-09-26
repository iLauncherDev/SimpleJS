#pragma once
#include "default.h"
#include <simplejs/lib/atomic.h>

int simplejs_platform_atomic_int_load(simplejs_atomic_int_t *atomic);
void simplejs_platform_atomic_int_store(simplejs_atomic_int_t *atomic, int value);

int simplejs_platform_atomic_int_add(simplejs_atomic_int_t *atomic, int value);
bool simplejs_platform_atomic_int_exchange_if(simplejs_atomic_int_t *atomic, int *expected_value, int value);
