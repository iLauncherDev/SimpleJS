#include <lib/time.h>

void SIMPLEJS_PLATFORM_API simplejs_get_performance_time(uint64_t *out_counter, uint64_t *out_frequency)
{
    SIMPLEJS_ASSERT(out_counter != NULL);
    SIMPLEJS_ASSERT(out_frequency != NULL);

    simplejs_platform_get_performance_time(out_counter, out_frequency);
}

double SIMPLEJS_PLATFORM_API simplejs_get_timestamp_f64()
{
    uint64_t counter, frequency;
    simplejs_get_performance_time(&counter, &frequency);

    return (double)counter / (double)frequency;
}

void SIMPLEJS_PLATFORM_API simplejs_sleep(uint32_t ms)
{
    simplejs_platform_sleep(ms);
}
