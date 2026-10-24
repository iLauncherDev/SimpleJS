#include <variable.h>
#include <object.h>
#include <lib/float_utils.h>
#include <lib/utf8.h>

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uint64_t is_number_mask = -(uint64_t)(variable->type == SIMPLEJS_VARIABLE_TYPE_NUMBER);
    uint64_t is_object_mask = -(uint64_t)(variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT);

    uint64_t number_value = simplejs_number_get_int64(&variable->value.number);
    uint64_t object_value = (uintptr_t)variable->value.object;

    return (number_value & is_number_mask) + (object_value & is_object_mask);
}

bool SIMPLEJS_API simplejs_variable_get_double(simplejs_variable_t *variable, double *out)
{
    SIMPLEJS_ASSERT(variable != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
        return false;

    *out = simplejs_number_get_float64(&variable->value.number);
    return true;
}

bool SIMPLEJS_API simplejs_variable_get_object(simplejs_variable_t *variable, void **out, uint16_t *out_value)
{
    SIMPLEJS_ASSERT(variable != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    bool is_object = variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT;

    *out = (void *)((uintptr_t)variable->value.object * (uintptr_t)is_object);
    if (out_value)
        *out_value = (uint16_t)variable->value.object_value * (uint16_t)is_object;

    return is_object;
}

bool SIMPLEJS_API simplejs_variable_get_hash_string(simplejs_variable_t *variable, simplejs_hash_string_ctx_t *out, const char *fallback_string)
{
    SIMPLEJS_ASSERT(variable != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    bool result = true;

    if (variable->type == SIMPLEJS_VARIABLE_TYPE_HASH_STRING)
    {
        simplejs_hash_string_get_context(out, variable->value.hash_string);
    }
    else if (variable->type == SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX)
    {
        *out = *variable->value.hash_string_ctx;
    }
    else if (variable->type == SIMPLEJS_VARIABLE_TYPE_FAST_STRING)
    {
        simplejs_init_hash_string_ctx(out, variable->value.fast_string);
    }
    else if (fallback_string)
    {
        simplejs_init_hash_string_ctx(out, fallback_string);
    }
    else
    {
        result = false;
    }

    return result;
}

void SIMPLEJS_API simplejs_variable_to_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, const char **out, simplejs_proxy_get_string_mode_t mode)
{
    SIMPLEJS_ASSERT(variable != NULL);

    switch (mode)
    {
    case SIMPLEJS_PROXY_GET_STRING_MODE_OPEN:
        switch (variable->type)
        {
        case SIMPLEJS_VARIABLE_TYPE_UNDEFINED:
            *out = "undefined";
            break;

        case SIMPLEJS_VARIABLE_TYPE_NAN:
            *out = "NaN";
            break;

        case SIMPLEJS_VARIABLE_TYPE_NUMBER:
        {
            SIMPLEJS_ASSERT(tempBufferSize > 1);

            simplejs_number_t *number = &variable->value.number;

            switch (number->type)
            {
            case SIMPLEJS_NUMBER_TYPE_BOOLEAN:
                *out = number->value.boolean ? "true" : "false";
                break;

            case SIMPLEJS_NUMBER_TYPE_UNICODE:
            {
                uint8_t len;
                simplejs_utf8_encode_char(number->value.unicode, tempBuffer, &tempBuffer[tempBufferSize - 1], &len);
                if (len > tempBufferSize - 1)
                    len = tempBufferSize - 1;

                tempBuffer[len] = '\0';
                *out = tempBuffer;
                break;
            }

            default:
                char tempString[512] = {0};

                simplejs_convert_double_to_string(tempBuffer, tempBufferSize - 1, simplejs_number_get_float64(&variable->value.number));

                *out = tempBuffer;
                break;
            }

            break;
        }

        case SIMPLEJS_VARIABLE_TYPE_OBJECT:
        {
            if (variable->value.object == NULL)
            {
                *out = "null";
                break;
            }

            const char *object_type = "[Unknown Object]";
            simplejs_object_get_string(variable->value.object, variable->value.object_value, &object_type, mode);

            *out = object_type;
            break;
        }

        case SIMPLEJS_VARIABLE_TYPE_FUNCTION:
            *out = "[Function]";
            break;

        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING:
            *out = variable->value.hash_string->string;
            break;

        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX:
            *out = variable->value.hash_string_ctx->string;
            break;

        case SIMPLEJS_VARIABLE_TYPE_FAST_STRING:
            *out = variable->value.fast_string;
            break;

        case SIMPLEJS_VARIABLE_TYPE_REFERENCE:
            simplejs_variable_to_string(variable->value.reference, tempBuffer, tempBufferSize, out, mode);
            break;

        default:
            SIMPLEJS_ASSERT("unknown variable->type for string conversion" && false);
            break;
        }
        break;

    case SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE:
        switch (variable->type)
        {
        case SIMPLEJS_VARIABLE_TYPE_OBJECT:
        {
            if (variable->value.object == NULL)
                break;

            simplejs_object_get_string(variable->value.object, variable->value.object_value, NULL, mode);
            break;
        }

        default:
            break;
        }
        break;

    default:
        break;
    }
}

void SIMPLEJS_API simplejs_variable_to_hash_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, simplejs_hash_string_ctx_t *out, simplejs_proxy_get_string_mode_t mode)
{
    SIMPLEJS_ASSERT(variable != NULL);

    bool status = true;

    switch (mode)
    {
    case SIMPLEJS_PROXY_GET_STRING_MODE_OPEN:
        switch (variable->type)
        {
        case SIMPLEJS_VARIABLE_TYPE_OBJECT:
        {
            if (variable->value.object == NULL)
            {
                status = false;
                break;
            }

            status &= SIMPLEJS_SUCCESS(simplejs_object_get_hash_string(variable->value.object, variable->value.object_value, out, mode));
            break;
        }

        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING:
            simplejs_hash_string_get_context(out, variable->value.hash_string);
            break;

        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX:
            *out = *variable->value.hash_string_ctx;
            break;

        default:
            status = false;
            break;
        }
        break;

    case SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE:
        switch (variable->type)
        {
        case SIMPLEJS_VARIABLE_TYPE_OBJECT:
        {
            if (variable->value.object == NULL)
            {
                status = false;
                break;
            }

            status &= SIMPLEJS_SUCCESS(simplejs_object_get_hash_string(variable->value.object, variable->value.object_value, NULL, mode));
            break;
        }

        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING:
        case SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX:
            break;

        default:
            status = false;
            break;
        }
        break;

    default:
        break;
    }

    if (!status)
    {
        const char *out_string;
        simplejs_variable_to_string(variable, tempBuffer, tempBufferSize, &out_string, mode);

        simplejs_init_hash_string_ctx(out, out_string);
    }
}

typedef bool (*simplejs_variable_ref_jumptable_f)(void *parent_pointer, const simplejs_variable_t *variable);
typedef void (*simplejs_variable_gc_jumptable_f)(simplejs_variable_t *variable);

bool simplejs_variable_ref_jumptable_undef(void *parent_pointer, simplejs_variable_t *variable)
{
    return true;
}

void simplejs_variable_gc_jumptable_undef(simplejs_variable_t *variable)
{
    return;
}

bool simplejs_variable_dereference_object(void *parent_pointer, simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    return simplejs_object_dereference(parent_pointer, variable->value.object);
}

bool simplejs_variable_reference_object(void *parent_pointer, simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    return simplejs_object_reference(parent_pointer, variable->value.object);
}

bool simplejs_variable_dereference_reference(void *parent_pointer, const simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.reference != NULL);

    return simplejs_variable_dereference_ex(parent_pointer, variable->value.reference);
}

bool simplejs_variable_reference_reference(void *parent_pointer, const simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.reference != NULL);

    return simplejs_variable_reference_ex(parent_pointer, variable->value.reference);
}

void simplejs_variable_lock_gc_object(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_lock_gc(variable->value.object);
}

void simplejs_variable_unlock_gc_object(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_unlock_gc(variable->value.object);
}

void simplejs_variable_lock_gc_reference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.reference != NULL);

    simplejs_variable_lock_gc(variable->value.reference);
}

void simplejs_variable_unlock_gc_reference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.reference != NULL);

    simplejs_variable_unlock_gc(variable->value.reference);
}

#define IS_VALID_OBJECT(variable) (variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT && variable->value.object != NULL)
#define IS_VALID_REFERENCE(variable) (variable->type == SIMPLEJS_VARIABLE_TYPE_REFERENCE && variable->value.reference != NULL)

bool SIMPLEJS_API simplejs_variable_dereference_ex(void *parent_pointer, const simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t object_mask = -IS_VALID_OBJECT(variable);
    uintptr_t reference_mask = -IS_VALID_REFERENCE(variable);
    uintptr_t else_mask = -(object_mask == 0 & reference_mask == 0);

    uintptr_t object_f = (uintptr_t)simplejs_variable_dereference_object;
    uintptr_t reference_f = (uintptr_t)simplejs_variable_dereference_reference;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_ref_jumptable_undef;
    simplejs_variable_ref_jumptable_f ptr_f =
        (void *)((object_f & object_mask) | (reference_f & reference_mask) | (undef_f & else_mask));

    return ptr_f(parent_pointer, variable);
}

bool SIMPLEJS_API simplejs_variable_reference_ex(void *parent_pointer, const simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t object_mask = -IS_VALID_OBJECT(variable);
    uintptr_t reference_mask = -IS_VALID_REFERENCE(variable);
    uintptr_t else_mask = -(object_mask == 0 & reference_mask == 0);

    uintptr_t object_f = (uintptr_t)simplejs_variable_reference_object;
    uintptr_t reference_f = (uintptr_t)simplejs_variable_reference_reference;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_ref_jumptable_undef;
    simplejs_variable_ref_jumptable_f ptr_f =
        (void *)((object_f & object_mask) | (reference_f & reference_mask) | (undef_f & else_mask));

    return ptr_f(parent_pointer, variable);
}

bool SIMPLEJS_API simplejs_variable_dereference(const simplejs_variable_t *variable)
{
    return simplejs_variable_dereference_ex(NULL, variable);
}

bool SIMPLEJS_API simplejs_variable_reference(const simplejs_variable_t *variable)
{
    return simplejs_variable_reference_ex(NULL, variable);
}

void SIMPLEJS_API simplejs_variable_lock_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t object_mask = -IS_VALID_OBJECT(variable);
    uintptr_t reference_mask = -IS_VALID_REFERENCE(variable);
    uintptr_t else_mask = -(object_mask == 0 & reference_mask == 0);

    uintptr_t object_f = (uintptr_t)simplejs_variable_lock_gc_object;
    uintptr_t reference_f = (uintptr_t)simplejs_variable_lock_gc_reference;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_gc_jumptable_undef;
    simplejs_variable_gc_jumptable_f ptr_f =
        (void *)((object_f & object_mask) | (reference_f & reference_mask) | (undef_f & else_mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_unlock_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t object_mask = -IS_VALID_OBJECT(variable);
    uintptr_t reference_mask = -IS_VALID_REFERENCE(variable);
    uintptr_t else_mask = -(object_mask == 0 & reference_mask == 0);

    uintptr_t object_f = (uintptr_t)simplejs_variable_unlock_gc_object;
    uintptr_t reference_f = (uintptr_t)simplejs_variable_unlock_gc_reference;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_gc_jumptable_undef;
    simplejs_variable_gc_jumptable_f ptr_f =
        (void *)((object_f & object_mask) | (reference_f & reference_mask) | (undef_f & else_mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_init_undefined(simplejs_variable_t *variable)
{
    variable->type = SIMPLEJS_VARIABLE_TYPE_UNDEFINED;
}

void SIMPLEJS_API simplejs_variable_init_nan(simplejs_variable_t *variable)
{
    variable->type = SIMPLEJS_VARIABLE_TYPE_NAN;
}

void SIMPLEJS_API simplejs_variable_init_null(simplejs_variable_t *variable)
{
    variable->value.object = NULL;
    variable->type = SIMPLEJS_VARIABLE_TYPE_OBJECT;
}

void SIMPLEJS_API simplejs_variable_init_number(simplejs_variable_t *variable, simplejs_number_t *number)
{
    variable->value.number = *number;
    variable->type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
}

void SIMPLEJS_API simplejs_variable_init_object(simplejs_variable_t *variable, void *object, uint16_t object_value)
{
    variable->value.object_value = object_value;
    variable->value.object = object;
    variable->type = SIMPLEJS_VARIABLE_TYPE_OBJECT;
}

void SIMPLEJS_API simplejs_variable_init_function(simplejs_variable_t *variable, simplejs_function_t *function)
{
    variable->value.function = *function;
    variable->type = SIMPLEJS_VARIABLE_TYPE_FUNCTION;
}

void SIMPLEJS_API simplejs_variable_init_hash_string(simplejs_variable_t *variable, const simplejs_hash_string_t *hash_string)
{
    variable->value.hash_string = hash_string;
    variable->type = SIMPLEJS_VARIABLE_TYPE_HASH_STRING;
}

void SIMPLEJS_API simplejs_variable_init_hash_string_ctx(simplejs_variable_t *variable, const simplejs_hash_string_ctx_t *hash_string_ctx)
{
    variable->value.hash_string_ctx = hash_string_ctx;
    variable->type = SIMPLEJS_VARIABLE_TYPE_HASH_STRING_CTX;
}

void SIMPLEJS_API simplejs_variable_init_fast_string(simplejs_variable_t *variable, const char *fast_string)
{
    variable->value.fast_string = fast_string;
    variable->type = SIMPLEJS_VARIABLE_TYPE_FAST_STRING;
}

void SIMPLEJS_API simplejs_variable_init_reference(simplejs_variable_t *variable, simplejs_variable_t *reference)
{
    variable->value.reference = reference;
    variable->type = SIMPLEJS_VARIABLE_TYPE_REFERENCE;
}

static const simplejs_variable_t fallback_variable = {
    .type = SIMPLEJS_VARIABLE_TYPE_UNDEFINED,
};

static bool is_circular_reference(simplejs_variable_t *start_variable, const simplejs_variable_t *current_variable)
{
    while (current_variable->type == SIMPLEJS_VARIABLE_TYPE_REFERENCE)
    {
        simplejs_variable_t *reference_variable = current_variable->value.reference;

        SIMPLEJS_ASSERT(reference_variable != NULL);

        if (reference_variable == start_variable)
            return true;

        current_variable = reference_variable;
    }

    return false;
}

bool SIMPLEJS_API simplejs_variable_assign_ex(void *parent_pointer, simplejs_variable_t *variable, const simplejs_variable_t *new_variable)
{
    bool status = false;

    bool is_invalid =
        ((parent_pointer != NULL) & (new_variable->type == SIMPLEJS_VARIABLE_TYPE_REFERENCE)) |
        is_circular_reference(variable, new_variable);
    uintptr_t is_invalid_mask = -(uintptr_t)is_invalid;

    const simplejs_variable_t *input_variable =
        (void *)(((uintptr_t)&fallback_variable & is_invalid_mask) | ((uintptr_t)new_variable & ~is_invalid_mask));

    simplejs_variable_t old_variable = *variable;

    simplejs_variable_lock_gc(&old_variable);

    if (simplejs_variable_reference_ex(parent_pointer, input_variable))
    {
        SIMPLEJS_ASSERT(simplejs_variable_dereference_ex(parent_pointer, &old_variable) == true);

        *variable = *input_variable;

        status = true;
    }

    simplejs_variable_unlock_gc(&old_variable);

    return status;
}

bool SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, const simplejs_variable_t *new_variable)
{
    return simplejs_variable_assign_ex(NULL, variable, new_variable);
}
