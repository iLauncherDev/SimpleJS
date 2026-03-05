#pragma once
#include "../default.h"
#include <simplejs/lib/shared_lib.h>

simplejs_status_t simplejs_platform_load_shared_lib(char *name, simplejs_shared_lib_t **out);
simplejs_status_t simplejs_platform_shared_lib_get_proc_address(simplejs_shared_lib_t *shared_lib, char *name, void **out);
void simplejs_platform_close_shared_lib(simplejs_shared_lib_t *shared_lib);
