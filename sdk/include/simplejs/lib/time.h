#pragma once
#include "../default.h"

void SIMPLEJS_PLATFORM_API simplejs_get_performance_time(uint64_t *out_counter, uint64_t *out_frequency);
double SIMPLEJS_PLATFORM_API simplejs_get_timestamp_f64();
void SIMPLEJS_PLATFORM_API simplejs_sleep(uint32_t ms);
