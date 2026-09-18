#pragma once
#include "default.h"

#ifdef __cplusplus
extern "C" {
#endif

pvoid SIMPLEJS_HOOK_API simplejs_hook_malloc(size_t size);
void SIMPLEJS_HOOK_API simplejs_hook_mfree(void *ptr);

#ifdef __cplusplus
}
#endif