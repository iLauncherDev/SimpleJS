#pragma once
#include "default.h"

pvoid SIMPLEJS_HOOK_API simplejs_hook_malloc(size_t size);
void SIMPLEJS_HOOK_API simplejs_hook_mfree(void *ptr);
