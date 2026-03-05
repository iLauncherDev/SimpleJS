#pragma once
#include "../default.h"

typedef struct simplejs_utf8_string
{
    uint8_t *buffer;
    size_t valid_size;
    size_t max_size;
} simplejs_utf8_string_t;

size_t SIMPLEJS_API simplejs_strnlen(const char *str, size_t n);
uint8_t SIMPLEJS_API simplejs_safe_string_fetch(simplejs_utf8_string_t *string, size_t index);
int SIMPLEJS_API simplejs_strcasecmp(char *s1, char *s2);

void SIMPLEJS_API simplejs_vprintf_ex(char *file, int line, char *fmt, va_list args);
void SIMPLEJS_API simplejs_printf_ex(char *file, int line, char *fmt, ...);

#define simplejs_vprintf(fmt, args) simplejs_vprintf_ex(__FILE__, __LINE__, fmt, args)
#define simplejs_printf(fmt, ...) simplejs_printf_ex(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
