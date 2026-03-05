#include <lib/sleep.h>
#include <windows.h>

void simplejs_platform_sleep(uint32_t ms)
{
    Sleep(ms);
}
