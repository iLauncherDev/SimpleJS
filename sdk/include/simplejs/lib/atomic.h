#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef volatile int simplejs_atomic_int_t;

/*
 * Loads the current value of the atomic integer.
 * The operation is atomic and safe for concurrent access.
 */
int SIMPLEJS_PLATFORM_API simplejs_atomic_int_load(simplejs_atomic_int_t *atomic);

/*
 * Stores a value into the atomic integer.
 * The operation is atomic and safe for concurrent access.
 */
void SIMPLEJS_PLATFORM_API simplejs_atomic_int_store(simplejs_atomic_int_t *atomic, int value);

/*
 * Performs an atomic add/subtract operation on a simplejs_atomic_int_t.
 * Returns the previous value before the operation.
 */
int SIMPLEJS_PLATFORM_API simplejs_atomic_int_add(simplejs_atomic_int_t *atomic, int value);
int SIMPLEJS_PLATFORM_API simplejs_atomic_int_sub(simplejs_atomic_int_t *atomic, int value);

/*
 * Atomically compares the current value with expected_value and updates it to
 * value only if they are equal.
 *
 * On return, expected_value contains the previous value that was stored in simplejs_atomic_int_t.
 * Returns true if the value was exchanged, otherwise false.
 */
bool SIMPLEJS_PLATFORM_API simplejs_atomic_int_exchange_if(simplejs_atomic_int_t *atomic, int *expected_value, int value);

#ifdef __cplusplus
}
#endif
