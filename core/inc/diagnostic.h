#pragma once
#include "default.h"
#include <simplejs/lib/s_string.h>
#include <tokenizer.h>

typedef struct simplejs_diagnostic_cursor
{
    simplejs_utf8_string_t *string;
    size_t line, column;
    size_t offset;
} simplejs_diagnostic_cursor_t;

#define SIMPLEJS_ANSI_COMMAND(cmd) "\x1b" cmd "m"

#define SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR "error"
#define SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_WARNING "warning"

typedef struct simplejs_diagnostic_message
{
    char *file_path;
    simplejs_utf8_string_t *code;
    struct {
        size_t start, end;
    } line_offset;
    struct {
        size_t start, end;
    } token_offset;

    char *indicator_color;

    char *type;
    char *message;
} simplejs_diagnostic_message_t;

void simplejs_init_diagnostic_cursor(simplejs_diagnostic_cursor_t *cursor, simplejs_utf8_string_t *string);
bool simplejs_diagnostic_cursor_goto_offset(simplejs_diagnostic_cursor_t *cursor, size_t offset);
bool simplejs_diagnostic_cursor_goto_line(simplejs_diagnostic_cursor_t *cursor, size_t line, size_t column);

void simplejs_init_diagnostic_message(simplejs_diagnostic_message_t *message);
void simplejs_present_diagnostic_message(simplejs_diagnostic_message_t *message);
