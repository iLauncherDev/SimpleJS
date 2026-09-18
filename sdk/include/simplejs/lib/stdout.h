#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

void SIMPLEJS_PLATFORM_API simplejs_stdout_vprintf(char *fmt, va_list args);
void SIMPLEJS_PLATFORM_API simplejs_stdout_printf(char *fmt, ...);

#ifdef __cplusplus
}
#endif
