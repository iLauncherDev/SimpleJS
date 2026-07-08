#pragma once
#include "../default.h"

typedef atomic_bool simplejs_spinlock_t;

void SIMPLEJS_PLATFORM_API simplejs_init_spinlock(simplejs_spinlock_t *spinlock);
bool SIMPLEJS_PLATFORM_API simplejs_spinlock_acquire(simplejs_spinlock_t *spinlock, bool wait_flag);
void SIMPLEJS_PLATFORM_API simplejs_spinlock_release(simplejs_spinlock_t *spinlock);
