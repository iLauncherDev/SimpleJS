#include <lib/time.h>
#include <windows.h>

void simplejs_platform_get_performance_time(uint64_t *out_counter, uint64_t *out_frequency)
{
    LARGE_INTEGER counter, frequency;

    QueryPerformanceCounter(&counter);
    QueryPerformanceFrequency(&frequency);

    *out_counter = counter.QuadPart;
    *out_frequency = frequency.QuadPart;
}

void simplejs_platform_sleep(uint32_t ms)
{
    Sleep(ms);
}
