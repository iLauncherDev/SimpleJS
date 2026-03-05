#pragma once
#include "default.h"
#include "number.h"

struct simplejs_function_header;
struct simplejs_variable;

typedef simplejs_status_t (*simplejs_function_proxy_t)(struct simplejs_function_header *function_header, struct simplejs_variable *out);

typedef enum simplejs_function_type
{
    SIMPLEJS_FUNCTION_TYPE_NATIVE,
    SIMPLEJS_FUNCTION_TYPE_PROXY,

    SIMPLEJS_FUNCTION_TYPE_END,
} simplejs_function_type_t;

typedef struct simplejs_function
{
    uint32_t reserved;
    uint32_t type;
    union
    {
        uintptr_t instruction_pointer;
        simplejs_function_proxy_t proxy;
    } value;
} simplejs_function_t;

typedef enum simplejs_variable_type
{
    SIMPLEJS_VARIABLE_TYPE_NUMBER,
    SIMPLEJS_VARIABLE_TYPE_OBJECT,
    SIMPLEJS_VARIABLE_TYPE_FUNCTION,
    SIMPLEJS_VARIABLE_TYPE_FAST_STRING,
} simplejs_variable_type_t;

typedef struct simplejs_variable
{
    union
    {
        uint32_t type;

        union
        {
            simplejs_number_t number;
            struct
            {
                uint32_t reserved_object;
                void *object;
            };
            simplejs_function_t function;
            struct
            {
                uint32_t reserved_fast_string;
                char *fast_string;
            };
        } value;
    };
} simplejs_variable_t;

typedef struct simplejs_function_header
{
    uint32_t argument_count;
    simplejs_variable_t arguments[];
} simplejs_function_header_t;

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable);

void SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, simplejs_variable_t *new_variable);
