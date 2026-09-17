#include <windows.h>
#include <lib/stdout.h>
#include <lib/ansi_color.h>

#define STDOUT_CACHE_SIZE 4096

typedef struct
{
    char chr;
    uint8_t color;
} win_stdout_entry_t;

static win_stdout_entry_t stdout_buffer[STDOUT_CACHE_SIZE / sizeof(win_stdout_entry_t)];
static char stdout_string_buffer[sizeof(stdout_buffer) / sizeof(*stdout_buffer)];

static size_t stdout_buffer_max_chars = sizeof(stdout_buffer) / sizeof(*stdout_buffer);
static size_t stdout_buffer_index = 0;

bool is_scanning_ansi = false;

char ansi_code[32];
size_t ansi_code_index = 0;

#define attributes_background_default (0)
#define attributes_foreground_default (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

HANDLE stdout_handle = 0;
WORD current_attributes = attributes_background_default | attributes_foreground_default;

static inline uint8_t convert_basic_color_to_mask(uint8_t basic_color, uint8_t r_mask, uint8_t g_mask, uint8_t b_mask)
{
    return (r_mask * (bool)(basic_color & (1 << 0))) |
           (g_mask * (bool)(basic_color & (1 << 1))) |
           (b_mask * (bool)(basic_color & (1 << 2)));
}

static void flush_cache()
{
    size_t start_index = 0;

    for (size_t i = 0; i < stdout_buffer_index; i++)
    {
        win_stdout_entry_t *entry = &stdout_buffer[i];
        win_stdout_entry_t *next_entry = (i + 1) < stdout_buffer_index ? &stdout_buffer[i + 1] : NULL;

        stdout_string_buffer[i - start_index] = entry->chr;

        if (!next_entry ||
            (current_attributes & 0xFF) != next_entry->color)
        {
            DWORD written_bytes = 0;
            win_stdout_entry_t *start_entry = &stdout_buffer[start_index];

            current_attributes &= ~0xFF;
            current_attributes |= start_entry->color;

            SetConsoleTextAttribute(stdout_handle, current_attributes);
            WriteFile(stdout_handle, stdout_string_buffer, (i - start_index) + 1, &written_bytes, NULL);

            start_index = i + 1;
        }
    }

    stdout_buffer_index = 0;
}

static void push_to_cache(char character)
{
    if (stdout_buffer_index >= stdout_buffer_max_chars)
    {
        flush_cache();
    }

    win_stdout_entry_t *entry = &stdout_buffer[stdout_buffer_index++];

    entry->chr = character;
    entry->color = current_attributes & 0xFF;

    if (character == '\n')
    {
        flush_cache();
    }
}

void simplejs_platform_print_char(char character)
{
    if (character == '\x1b')
    {
        memclr(ansi_code, sizeof(ansi_code));
        ansi_code_index = 0;

        is_scanning_ansi = true;
        return;
    }

    if (is_scanning_ansi)
    {
        if (ansi_code_index <= sizeof(ansi_code) - 1)
        {
            ansi_code[ansi_code_index + 0] = character;
            ansi_code[ansi_code_index + 1] = '\0';
            ansi_code_index++;
        }

        if (character == 'm')
        {
            ansi_color_info_t info = {0};
            decode_ansi_code(ansi_code, &info);

            if (info.has_basic_foreground)
            {
                current_attributes &= ~0x0F;
                current_attributes |= convert_basic_color_to_mask(info.basic_foreground, FOREGROUND_RED, FOREGROUND_GREEN, FOREGROUND_BLUE);
            }

            if (info.has_basic_background)
            {
                current_attributes &= ~0xF0;
                current_attributes |= convert_basic_color_to_mask(info.basic_foreground, BACKGROUND_RED, BACKGROUND_GREEN, BACKGROUND_BLUE);
            }

            if (info.reset_signal)
            {
                current_attributes = attributes_background_default | attributes_foreground_default;
            }

            SetConsoleTextAttribute(stdout_handle, current_attributes);

            is_scanning_ansi = false;
        }

        return;
    }

    push_to_cache(character);
}

simplejs_status_t simplejs_platform_init_stdout()
{
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(stdout_handle, current_attributes);

    return SIMPLEJS_STATUS_SUCCESS;
}
