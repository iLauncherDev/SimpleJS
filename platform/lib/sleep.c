#include <lib/sleep.h>

void SIMPLEJS_PLATFORM_API simplejs_sleep(uint32_t ms)
{
    simplejs_platform_sleep(ms);
}
