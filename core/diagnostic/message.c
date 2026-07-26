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
    SIMPLEJS_ASSERT(message->linemap_ctx != NULL);

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

    simplejs_char_cache_fetch_t *cache_fetch = simplejs_get_linemap_cache_fetch(message->linemap_ctx);

    simplejs_linemap_position_t temp_cursor;
    simplejs_get_linemap_by_offset(message->linemap_ctx, message->token_offset.start, &temp_cursor);

    printf("%s:%lu:%lu: %s%s:%s %s\n",
           message->linemap_ctx->file_path, (unsigned long)(temp_cursor.line + 1), (unsigned long)(temp_cursor.column + 1),
           indicator_color, message->type, SIMPLEJS_ANSI_COMMAND("[0"),
           message->message);

    simplejs_linemap_position_t line_cursor;

    size_t error_line = temp_cursor.line;

    simplejs_get_linemap_by_offset(message->linemap_ctx, message->line_offset.start, &line_cursor);
    size_t line_start = line_cursor.line;

    simplejs_get_linemap_by_offset(message->linemap_ctx, message->line_offset.end, &line_cursor);
    size_t line_end = line_cursor.line;

    for (size_t current_line = line_start; current_line <= line_end; current_line++)
    {
        simplejs_linemap_t *linemap = simplejs_get_linemap_by_line(message->linemap_ctx, current_line);
        if (!linemap)
            break;

        uint64_t line_length = linemap->offset.end - linemap->offset.start;

        char temp_prefix_string[256] = {0};
        snprintf(temp_prefix_string, sizeof(temp_prefix_string) - 1, diagnostic_fmt_prefix, (unsigned long)(current_line + 1));

        printf(temp_prefix_string);

        for (uint64_t current_offset = linemap->offset.start; current_offset < linemap->offset.end; current_offset++)
        {
            char chr0;
            simplejs_fetch_char_from_cache(cache_fetch, current_offset, &chr0);

            if (current_offset == message->token_offset.start)
            {
                printf(indicator_color);
            }

            printf("%c", chr0);

            if ((current_offset + 1) == message->token_offset.end ||
                (current_offset + 1) >= linemap->offset.end)
            {
                printf(SIMPLEJS_ANSI_COMMAND("[0"));
            }
        }
        printf("\n");

        if (current_line == error_line)
        {
            for (size_t i = 0; temp_prefix_string[i] != '\0'; i++)
                printf(" ");

            for (uint64_t current_offset = linemap->offset.start; current_offset < linemap->offset.end; current_offset++)
            {
                char chr0;
                simplejs_fetch_char_from_cache(cache_fetch, current_offset, &chr0);

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
