#pragma once
#include "default.h"
#include "number.h"
#include "function.h"

typedef union simplejs_variable simplejs_variable_t;

typedef enum simplejs_variable_type
{
    SIMPLEJS_VARIABLE_TYPE_NUMBER,
    SIMPLEJS_VARIABLE_TYPE_OBJECT,
    SIMPLEJS_VARIABLE_TYPE_FAST_STRING,
    SIMPLEJS_VARIABLE_TYPE_FUNCTION,
} simplejs_variable_type_t;

union simplejs_variable
{
    uint16_t type;

    union
    {
        simplejs_number_t number;

        struct
        {
            uint16_t reserved_object;
            uint16_t object_value;
            void *object;
        };

        struct
        {
            uint16_t reserved_fast_string;
            char *fast_string;
        };

        simplejs_function_t function;
    } value;
};

struct simplejs_function_header
{
    pvoid vm;
    uint32_t header_size;
    uint32_t argument_count;

    simplejs_variable_t *return_variable;
    simplejs_variable_t this_variable;
    simplejs_variable_t arguments[];
};

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable);
bool SIMPLEJS_API simplejs_variable_get_double(simplejs_variable_t *variable, double *out);

void SIMPLEJS_API simplejs_variable_to_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, char **out);

void SIMPLEJS_API simplejs_variable_dereference(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_reference(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_lock_gc(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_unlock_gc(simplejs_variable_t *variable);

void SIMPLEJS_API simplejs_variable_init_undefined(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_init_null(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_init_number(simplejs_variable_t *variable, simplejs_number_t *number);
void SIMPLEJS_API simplejs_variable_init_object(simplejs_variable_t *variable, void *object, uint16_t object_value);
void SIMPLEJS_API simplejs_variable_init_function(simplejs_variable_t *variable, simplejs_function_t *function);
void SIMPLEJS_API simplejs_variable_init_fast_string(simplejs_variable_t *variable, char *fast_string);

void SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, simplejs_variable_t *new_variable);
