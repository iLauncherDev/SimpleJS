#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SIMPLEJS_UTF8_REPLACEMENT_CHAR 0xFFFD

#define SIMPLEJS_UTF8_SURROGATE_LOW_MIN 0xD800
#define SIMPLEJS_UTF8_SURROGATE_LOW_MAX 0xDBFF

#define SIMPLEJS_UTF8_SURROGATE_HIGH_MIN 0xDC00
#define SIMPLEJS_UTF8_SURROGATE_HIGH_MAX 0xDFFF

bool SIMPLEJS_API simplejs_utf8_is_continuation_byte(uint8_t value);
uint8_t SIMPLEJS_API simplejs_utf8_get_unicode_char_size(uint32_t unicode);
bool SIMPLEJS_API simplejs_utf8_decode_char(const char *buffer, const char *end_buffer, uint8_t *len, uint32_t *out);
simplejs_status_t SIMPLEJS_API simplejs_utf8_encode_char(uint32_t unicode, char *buffer, const char *buffer_end, uint8_t *len);

#ifdef __cplusplus
}
#endif
