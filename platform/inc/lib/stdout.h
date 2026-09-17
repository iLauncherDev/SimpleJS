#pragma once
#include "../default.h"
#include <simplejs/lib/stdout.h>

void simplejs_platform_print_char(char character);
simplejs_status_t simplejs_platform_init_stdout();
simplejs_status_t simplejs_init_stdout();
