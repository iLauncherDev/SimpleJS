#pragma once
#include "default.h"

typedef struct simplejs_function simplejs_function_t;
typedef struct simplejs_function_header simplejs_function_header_t;
typedef simplejs_status_t (*simplejs_function_proxy_t)(simplejs_function_header_t *function_header);

typedef enum simplejs_function_type
{
    SIMPLEJS_FUNCTION_TYPE_NATIVE,
    SIMPLEJS_FUNCTION_TYPE_PROXY,

    SIMPLEJS_FUNCTION_TYPE_END,
} simplejs_function_type_t;

struct simplejs_function
{
    uint16_t reserved;
    uint16_t type;
    union
    {
        uintptr_t instruction_pointer;
        simplejs_function_proxy_t proxy;
    } value;
};

