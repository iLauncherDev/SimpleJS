#include <lib/sleep.h>

void SIMPLEJS_API simplejs_sleep(uint32_t ms)
{
    simplejs_platform_sleep(ms);
}
