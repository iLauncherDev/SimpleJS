#include <lib/stdout.h>

static simplejs_spinlock_t stdout_spinlock = 0;

#define vsnoprintf(buffer, size, offset, fmt, args)                 \
    do                                                              \
    {                                                               \
        vsnprintf(&buffer[offset], (size - offset) - 1, fmt, args); \
        buffer[size - 1] = '\0';                                    \
    } while (0)

#define snoprintf(buffer, size, offset, fmt, ...)                         \
    do                                                                    \
    {                                                                     \
        snprintf(&buffer[offset], (size - offset) - 1, fmt, __VA_ARGS__); \
        buffer[size - 1] = '\0';                                          \
    } while (0)

static int get_int_size(char *fmt_string)
{
    int int_size = sizeof(int);

    bool used_long = false;

    for (size_t i = 0; fmt_string[i] != '\0'; i++)
    {
        char chr_0 = fmt_string[i];


        switch (chr_0)
        {
        case 'l':
            if (used_long)
                int_size = sizeof(long long);
            else
                int_size = sizeof(long);

            used_long = true;
            break;

        case 'I':
            if (!strncmp(&fmt_string[i], "I64", 3))
                int_size = sizeof(int64_t);
            else if (!strncmp(&fmt_string[i], "I32", 3))
                int_size = sizeof(int32_t);

            break;
        }
    }

    return int_size;
}

void SIMPLEJS_PLATFORM_API simplejs_stdout_vprintf(char *fmt, va_list args)
{
    char fmt_out[128];
    size_t i = 0;

    simplejs_spinlock_acquire(&stdout_spinlock, true);

    while (fmt[i] != '\0')
    {
        char chr = fmt[i];

        if (chr == '%')
        {
            char *str_out = (char *)&fmt_out;
            str_out[0] = '\0';

            char fmt_string[32];
            size_t fmt_index = 0;
            size_t fmt_max_index = sizeof(fmt_string) / sizeof(*fmt_string);

            fmt_string[fmt_index + 0] = chr;
            fmt_string[fmt_index + 1] = '\0';
            fmt_index++;

        repeat:
            i++;

            char fmt_chr = fmt[i];

            if (fmt_index <= fmt_max_index - 1)
            {
                fmt_string[fmt_index + 0] = fmt_chr;
                fmt_string[fmt_index + 1] = '\0';
                fmt_index++;
            }

            switch (fmt_chr)
            {
            case '\0':
                break;

            case '*':
                int count = va_arg(args, int);

                snoprintf(fmt_string, fmt_max_index, fmt_index - 1, "%u", count);
                fmt_index = strlen(fmt_string);

                goto repeat;

            case 'I':
            case 'l':
            case 'h':

            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                goto repeat;

            case '.':
                goto repeat;

            case 'p':
                snoprintf(fmt_out, sizeof(fmt_out), 0, fmt_string, va_arg(args, void *));

                break;

            case 'x':
            case 'u':
            case 'd':
            case 'c':
            {
                int int_size = get_int_size(fmt_string);

                switch (int_size)
                {
                case 8:
                    snoprintf(fmt_out, sizeof(fmt_out), 0, fmt_string, va_arg(args, uint64_t));

                    break;

                default:
                    snoprintf(fmt_out, sizeof(fmt_out), 0, fmt_string, va_arg(args, uint32_t));

                    break;
                }

                break;
            }

            case 'f':
                snoprintf(fmt_out, sizeof(fmt_out), 0, fmt_string, va_arg(args, double));

                break;

            case 's':
                str_out = va_arg(args, char *);
                if (!str_out)
                    str_out = "[NULL]";

                break;

            default:
                SIMPLEJS_ASSERT(false && "undefined behaviour");
                break;
            }

            SIMPLEJS_ASSERT(str_out != NULL);

            for (size_t j = 0; str_out[j] != '\0'; j++)
                simplejs_platform_print_char(str_out[j]);
        }
        else
        {
            simplejs_platform_print_char(chr);
        }

        i++;
    }

    simplejs_spinlock_release(&stdout_spinlock);
}

void SIMPLEJS_PLATFORM_API simplejs_stdout_printf(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    simplejs_stdout_vprintf(fmt, args);
    va_end(args);
}

simplejs_status_t simplejs_init_stdout()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_platform_init_stdout(), result, status);

result:
    return status;
}
