#include <lib/stdout_buffer.h>

#define stdout_buffer_size 16 * 1024

void SIMPLEJS_PLATFORM_API simplejs_platform_enable_stdout_buffer()
{
    setvbuf(stdout, NULL, _IONBF, 0);

    setvbuf(stdout, NULL, _IOFBF, stdout_buffer_size);
}
