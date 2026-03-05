#pragma once
#include "../default.h"

typedef void simplejs_shared_lib_t;

simplejs_status_t SIMPLEJS_API simplejs_load_shared_lib(char *name, simplejs_shared_lib_t **out);
simplejs_status_t SIMPLEJS_API simplejs_shared_lib_get_proc_address(simplejs_shared_lib_t *shared_lib, char *name, void **out);
void SIMPLEJS_API simplejs_close_shared_lib(simplejs_shared_lib_t *shared_lib);
