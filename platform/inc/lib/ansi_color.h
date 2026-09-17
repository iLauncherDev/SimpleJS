#pragma once
#include "../default.h"

#define ANSI_COLOR_CHAR '\x1b'

typedef struct ansi_color_info
{
    uint8_t reset_signal : 1;

    uint8_t has_basic_background : 1;
    uint8_t basic_background : 3;

    uint8_t has_basic_foreground : 1;
    uint8_t basic_foreground : 3;
} ansi_color_info_t;

void decode_ansi_code(char *ansi_code, ansi_color_info_t *out);
