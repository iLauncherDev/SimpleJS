#include <lib/sleep.h>
#include <time.h>

void simplejs_platform_sleep(uint32_t ms)
{
    struct timespec delay_time;
    delay_time.tv_sec = ms / 1000;
    delay_time.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&delay_time, NULL);
}
