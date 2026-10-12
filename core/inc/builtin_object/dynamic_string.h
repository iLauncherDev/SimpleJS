#pragma once
#include "../default.h"
#include <simplejs/object.h>
#include <simplejs/builtin_object/dynamic_string.h>

typedef struct simplejs_dynamic_string_raw simplejs_dynamic_string_raw_t;

typedef enum simplejs_dynamic_string_property_type
{
    SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_OFFSET,
    SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FIELD_OFFSET_SIZE_T,
    SIMPLEJS_DYNAMIC_STRING_PROPERTY_TYPE_FUNCTION_PROXY,
} simplejs_dynamic_string_property_type_t;

typedef struct simplejs_dynamic_string_property
{
    char *name;

    uint64_t hash;
    simplejs_dynamic_string_property_type_t type;

    simplejs_status_t (*on_change_f)(simplejs_dynamic_string_raw_t *dynamic_string, size_t old_value);

    union
    {
        size_t offset;
        simplejs_function_proxy_t function_proxy;
    };

    simplejs_list_entry_t list_entry;
} simplejs_dynamic_string_property_t;

struct simplejs_dynamic_string_raw
{
    uint32_t std_flags;

    simplejs_dynamic_string_property_t *property_list;
    size_t property_list_count;

    simplejs_spinlock_t property_lock;
    simplejs_spinlock_t buffer_lock;

    uint64_t string_hash;

    bool is_heap_allocated;
    char *string;
    size_t valid_size, allocated_size;

    bool is_dirty;
};

simplejs_status_t simplejs_builtin_init_dynamic_string();
void simplejs_builtin_uninit_dynamic_string();
