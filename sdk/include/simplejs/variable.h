#pragma once
#include "default.h"
#include "number.h"
#include "function.h"
#include "hash_string.h"
#include "proxy_enum.h"

typedef union simplejs_variable simplejs_variable_t;

typedef enum simplejs_variable_type
{
    SIMPLEJS_VARIABLE_TYPE_UNDEFINED,
    SIMPLEJS_VARIABLE_TYPE_NAN,
    SIMPLEJS_VARIABLE_TYPE_NUMBER,
    SIMPLEJS_VARIABLE_TYPE_OBJECT,
    SIMPLEJS_VARIABLE_TYPE_HASH_STRING,
    SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX,
    SIMPLEJS_VARIABLE_TYPE_FAST_STRING,
    SIMPLEJS_VARIABLE_TYPE_FUNCTION,
    SIMPLEJS_VARIABLE_TYPE_REFERENCE,

    SIMPLEJS_VARIABLE_TYPE_END,
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
            uint16_t reserved_hash_string;

            union
            {
                const simplejs_hash_string_t *hash_string;
                const simplejs_hash_string_ctx_t *hash_string_ctx;
            };
        };

        struct
        {
            uint16_t reserved_fast_string;
            const char *fast_string;
        };

        simplejs_function_t function;

        struct
        {
            uint16_t reserved_reference;
            simplejs_variable_t *reference;
        };
    } value;
};

struct simplejs_function_header
{
    pvoid vm;
    uint32_t header_size;
    uint32_t argument_count;
    simplejs_variable_t *real_return_variable;
    simplejs_variable_t return_variable;
    simplejs_variable_t this_variable, super_variable;
    simplejs_variable_t arguments[];
};

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable);
bool SIMPLEJS_API simplejs_variable_get_double(simplejs_variable_t *variable, double *out);
bool SIMPLEJS_API simplejs_variable_get_object(simplejs_variable_t *variable, void **out, uint16_t *out_value);
bool SIMPLEJS_API simplejs_variable_get_hash_string(simplejs_variable_t *variable, simplejs_hash_string_ctx_t *out, const char *fallback_string);

void SIMPLEJS_API simplejs_variable_to_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, const char **out, simplejs_proxy_get_string_mode_t mode);
void SIMPLEJS_API simplejs_variable_to_hash_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, simplejs_hash_string_ctx_t *out, simplejs_proxy_get_string_mode_t mode);

bool SIMPLEJS_API simplejs_variable_dereference_ex(void *parent_pointer, const simplejs_variable_t *variable);
bool SIMPLEJS_API simplejs_variable_reference_ex(void *parent_pointer, const simplejs_variable_t *variable);

bool SIMPLEJS_API simplejs_variable_dereference(const simplejs_variable_t *variable);
bool SIMPLEJS_API simplejs_variable_reference(const simplejs_variable_t *variable);

void SIMPLEJS_API simplejs_variable_lock_gc(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_unlock_gc(simplejs_variable_t *variable);

void SIMPLEJS_API simplejs_variable_init_undefined(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_init_nan(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_init_null(simplejs_variable_t *variable);
void SIMPLEJS_API simplejs_variable_init_number(simplejs_variable_t *variable, simplejs_number_t *number);
void SIMPLEJS_API simplejs_variable_init_object(simplejs_variable_t *variable, void *object, uint16_t object_value);
void SIMPLEJS_API simplejs_variable_init_function(simplejs_variable_t *variable, simplejs_function_t *function);
void SIMPLEJS_API simplejs_variable_init_hash_string(simplejs_variable_t *variable, const simplejs_hash_string_t *hash_string);
void SIMPLEJS_API simplejs_variable_init_hash_string_ctx(simplejs_variable_t *variable, const simplejs_hash_string_ctx_t *hash_string_ctx);
void SIMPLEJS_API simplejs_variable_init_fast_string(simplejs_variable_t *variable, const char *fast_string);
void SIMPLEJS_API simplejs_variable_init_reference(simplejs_variable_t *variable, simplejs_variable_t *reference);

bool SIMPLEJS_API simplejs_variable_assign_ex(void *parent_pointer, simplejs_variable_t *variable, const simplejs_variable_t *new_variable);

bool SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, const simplejs_variable_t *new_variable);
