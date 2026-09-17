#include <lib/stdout.h>

void simplejs_platform_print_char(char character)
{
    printf("%c", character);
}

simplejs_status_t simplejs_platform_init_stdout()
{
    return SIMPLEJS_STATUS_SUCCESS;
}
