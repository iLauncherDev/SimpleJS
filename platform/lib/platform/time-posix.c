#include <lib/time.h>
#include <time.h>

void simplejs_platform_get_performance_time(uint64_t *out_counter, uint64_t *out_frequency)
{
    struct timespec timestamp;
    clock_gettime(CLOCK_MONOTONIC, &timestamp);

    uint64_t frequency = 1000000000;
    uint64_t counter = (uint64_t)timestamp.tv_sec * frequency + (uint64_t)timestamp.tv_nsec;

    *out_counter = counter;
    *out_frequency = frequency;
}

void simplejs_platform_sleep(uint32_t ms)
{
    struct timespec delay_time;
    delay_time.tv_sec = ms / 1000;
    delay_time.tv_nsec = (ms % 1000) * 1000 * 1000;

    nanosleep(&delay_time, NULL);
}
