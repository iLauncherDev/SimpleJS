#include <diagnostic.h>

void simplejs_init_diagnostic_cursor(simplejs_diagnostic_cursor_t *cursor, simplejs_utf8_string_t *string)
{
    SIMPLEJS_ASSERT(cursor != NULL);
    SIMPLEJS_ASSERT(string != NULL);

    memclr(cursor, sizeof(*cursor));
    cursor->string = string;
}

bool simplejs_diagnostic_cursor_goto_offset(simplejs_diagnostic_cursor_t *cursor, size_t offset)
{
    size_t current_offset = 0;
    size_t current_line = 0, current_column = 0;

    for (current_offset = 0; current_offset < cursor->string->valid_size; current_offset++)
    {
        if (current_offset == offset)
        {
            cursor->offset = current_offset;
            cursor->line = current_line;
            cursor->column = current_column;

            return true;
        }

        char chr0 = simplejs_safe_string_fetch(cursor->string, current_offset);

        current_column++;

        if (chr0 == '\n')
        {
            current_line++;
            current_column = 0;
        }
    }

    return false;
}

bool simplejs_diagnostic_cursor_goto_line(simplejs_diagnostic_cursor_t *cursor, size_t line, size_t column)
{
    size_t current_offset = 0;
    size_t current_line = 0, current_column = 0;

    for (current_offset = 0; current_offset < cursor->string->valid_size; current_offset++)
    {
        if (current_line == line &&
            current_column == column)
        {
            cursor->offset = current_offset;
            cursor->line = current_line;
            cursor->column = current_column;

            return true;
        }

        char chr0 = simplejs_safe_string_fetch(cursor->string, current_offset);

        current_column++;

        if (chr0 == '\n')
        {
            current_line++;
            current_column = 0;
        }
    }

    return false;
}
