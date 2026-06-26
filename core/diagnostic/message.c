#include <diagnostic.h>

static char *diagnostic_fmt_prefix = "    %lu | ";
static size_t diagnost_last_lines = 1;

typedef struct color_table
{
    char *type, *color;
} color_table_t;

static color_table_t message_type_color_table[] = {
    {SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, SIMPLEJS_ANSI_COMMAND("[31")},
    {SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_WARNING, SIMPLEJS_ANSI_COMMAND("[33")},
};

void simplejs_init_diagnostic_message(simplejs_diagnostic_message_t *message)
{
    memclr(message, sizeof(*message));
}

void simplejs_present_diagnostic_message(simplejs_diagnostic_message_t *message)
{
    SIMPLEJS_ASSERT(message->type != NULL);

    char *indicator_color = message->indicator_color;
    if (!indicator_color)
    {
        for (size_t i = 0; i < sizeof(message_type_color_table) / sizeof(*message_type_color_table); i++)
        {
            color_table_t *color_table_entry = &message_type_color_table[i];

            if (!strcmp(color_table_entry->type, message->type))
            {
                indicator_color = color_table_entry->color;
                goto found_indicator_color;
            }
        }

        indicator_color = SIMPLEJS_ANSI_COMMAND("[0");

    found_indicator_color:
    }

    simplejs_diagnostic_cursor_t temp_cursor;
    simplejs_init_diagnostic_cursor(&temp_cursor, message->code);
    simplejs_diagnostic_cursor_goto_offset(&temp_cursor, message->token_offset.start);

    printf("%s:%lu:%lu: %s%s:%s %s\n",
           message->file_path, (unsigned long)(temp_cursor.line + 1), (unsigned long)(temp_cursor.column + 1),
           indicator_color, message->type, SIMPLEJS_ANSI_COMMAND("[0"),
           message->message);

    simplejs_diagnostic_cursor_t line_cursor;
    simplejs_init_diagnostic_cursor(&line_cursor, temp_cursor.string);

    size_t error_line = temp_cursor.line;

    simplejs_diagnostic_cursor_goto_offset(&line_cursor, message->line_offset.start);
    size_t line_start = line_cursor.line;

    simplejs_diagnostic_cursor_goto_offset(&line_cursor, message->line_offset.end);
    size_t line_end = line_cursor.line;

    for (size_t current_line = line_start; current_line <= line_end; current_line++)
    {
        size_t line_length = 0;

        if (!simplejs_diagnostic_cursor_goto_line(&line_cursor, current_line, 0))
            break;

        char temp_prefix_string[256] = {0};
        snprintf(temp_prefix_string, sizeof(temp_prefix_string) - 1, diagnostic_fmt_prefix, (unsigned long)(current_line + 1));

        printf(temp_prefix_string);
        for (size_t i = 0; i == i; i++, line_length++)
        {
            size_t current_offset = line_cursor.offset + i;

            char chr0 = simplejs_safe_string_fetch(line_cursor.string, current_offset);
            if (chr0 == '\n' ||
                chr0 == '\0')
                break;

            if (current_offset == message->token_offset.start)
            {
                printf(indicator_color);
            }

            printf("%c", chr0);

            if ((current_offset + 1) == message->token_offset.end)
            {
                printf(SIMPLEJS_ANSI_COMMAND("[0"));
            }
        }
        printf("\n");

        if (current_line == error_line)
        {
            for (size_t i = 0; temp_prefix_string[i] != '\0'; i++)
                printf(" ");

            for (size_t i = 0; i < line_length; i++)
            {
                size_t current_offset = line_cursor.offset + i;
                char chr0 = simplejs_safe_string_fetch(line_cursor.string, current_offset);

                char *indicator_prefix = "%s%s" SIMPLEJS_ANSI_COMMAND("[0");

                if (current_offset >= message->token_offset.start &&
                    current_offset < message->token_offset.end)
                {
                    printf(indicator_prefix, indicator_color, "^");
                }
                else
                {
                    printf(" ");
                }
            }

            printf("\n");
        }
    }
}
