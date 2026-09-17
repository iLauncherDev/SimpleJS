#include <lib/ansi_color.h>

static inline bool is_valid_char(char chr_0)
{
    return chr_0 != '\0' && chr_0 != 'm';
}

void decode_ansi_code(char *ansi_code, ansi_color_info_t *out)
{
    SIMPLEJS_ASSERT(ansi_code != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    memclr(out, sizeof(*out));

    size_t i = 0;
    bool start_parsing = false;

    while (is_valid_char(ansi_code[i]))
    {
        char chr_0 = ansi_code[i + 0];
        char chr_1 = ansi_code[i + 1];

        if (start_parsing)
        {
            size_t width = 0;

            if (chr_0 == ';')
            {
                width = 1;
            }
            if (chr_0 == '0')
            {
                out->reset_signal = true;

                width = 1;
            }
            else if (chr_0 == '3' && (chr_1 >= '0' && chr_1 < '8'))
            {
                out->has_basic_foreground = true;
                out->basic_foreground = (chr_1 - '0');

                width = 2;
            }
            else if (chr_0 == '4' && (chr_1 >= '0' && chr_1 < '8'))
            {
                out->has_basic_background = true;
                out->basic_background = (chr_1 - '0');

                width = 2;
            }

            if (!width)
                break;

            i += width;
        }
        else
        {
            if (chr_0 == '[')
                start_parsing = true;

            i++;
        }
    }
}