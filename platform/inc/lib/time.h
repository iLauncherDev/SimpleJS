#pragma once
#include "../default.h"
#include <simplejs/lib/time.h>

void simplejs_platform_get_performance_time(uint64_t *out_counter, uint64_t *out_frequency);
void simplejs_platform_sleep(uint32_t ms);
