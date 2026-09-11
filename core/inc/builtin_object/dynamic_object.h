#pragma once
#include "../object.h"
#include <simplejs/builtin_object/dynamic_object.h>

typedef struct simplejs_dynamic_object_property
{
    bool is_hardcoded, is_hardlocked;

    simplejs_proxy_property_t property;

    simplejs_safe_list_entry_t safe_list_entry;
} simplejs_dynamic_object_property_t;

typedef struct simplejs_dynamic_object_raw
{
    uint32_t std_flags;

    simplejs_dynamic_object_property_t prototype_property;

    simplejs_safe_list_t property_list;
} simplejs_dynamic_object_raw_t;

simplejs_status_t simplejs_builtin_init_dynamic_object();
void simplejs_builtin_uninit_dynamic_object();
