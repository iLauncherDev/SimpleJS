#pragma once
#include "../object.h"
#include <simplejs/builtin_object/dynamic_string.h>

typedef struct simplejs_dynamic_string_raw
{
    uint32_t std_flags;

    char *string;
    size_t length, allocated_size;
} simplejs_dynamic_string_raw_t;

simplejs_status_t simplejs_builtin_init_dynamic_string();
void simplejs_builtin_uninit_dynamic_string();
