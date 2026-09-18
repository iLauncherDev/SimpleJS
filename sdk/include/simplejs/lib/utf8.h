#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

simplejs_status_t SIMPLEJS_API simplejs_encode_utf8_char(uint32_t unicode, uint8_t *len, char *bytes);

#ifdef __cplusplus
}
#endif
