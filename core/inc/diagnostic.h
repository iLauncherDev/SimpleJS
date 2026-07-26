#pragma once
#include "default.h"
#include "linemap.h"
#include <simplejs/lib/s_string.h>

#define SIMPLEJS_ANSI_COMMAND(cmd) "\x1b" cmd "m"

#define SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR "error"
#define SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_WARNING "warning"

typedef struct simplejs_diagnostic_message
{
    simplejs_linemap_ctx_t *linemap_ctx;
    simplejs_linemap_offset_t line_offset;
    simplejs_linemap_offset_t token_offset;

    char *indicator_color;

    char *type;
    char *message;
} simplejs_diagnostic_message_t;

void simplejs_init_diagnostic_message(simplejs_diagnostic_message_t *message);
void simplejs_present_diagnostic_message(simplejs_diagnostic_message_t *message);
