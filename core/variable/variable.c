#include <variable.h>
#include <object.h>
#include <lib/float_utils.h>

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uint64_t is_number_mask = -(uint64_t)(variable->type == SIMPLEJS_VARIABLE_TYPE_NUMBER);
    uint64_t number_value = simplejs_number_get_int64(&variable->value.number);
    uint64_t object_value = (uintptr_t)variable->value.object;

    return (number_value & is_number_mask) + (object_value & ~is_number_mask);
}

void SIMPLEJS_API simplejs_variable_to_string(simplejs_variable_t *variable, char *tempBuffer, size_t tempBufferSize, char **out)
{
    SIMPLEJS_ASSERT(variable != NULL);

    switch (variable->type)
    {
    case SIMPLEJS_VARIABLE_TYPE_NUMBER:
    {
        SIMPLEJS_ASSERT(tempBufferSize > 1);

        simplejs_number_t *number = &variable->value.number;

        switch (number->type)
        {
        case SIMPLEJS_NUMBER_TYPE_UNDEFINED:
            *out = "undefined";
            break;

        case SIMPLEJS_NUMBER_TYPE_NULL:
            *out = "null";
            break;

        case SIMPLEJS_NUMBER_TYPE_BOOLEAN:
            *out = number->value.boolean ? "true" : "false";
            break;

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
        char *object_type = "[Unknown Object]";
        simplejs_object_get_string(variable->value.object, &object_type);

        *out = object_type;
        break;
    }

    case SIMPLEJS_VARIABLE_TYPE_FUNCTION:
        *out = "[Function]";
        break;

    case SIMPLEJS_VARIABLE_TYPE_FAST_STRING:
        *out = variable->value.fast_string;
        break;

    default:
        SIMPLEJS_ASSERT("unknown variable->type for string conversion" && false);
        break;
    }
}

typedef void (*simplejs_variable_jumptable_f)(simplejs_variable_t *variable);

void simplejs_variable_jumptable_undef(simplejs_variable_t *variable)
{
    return;
}

void simplejs_variable_dereference_object(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_dereference(variable->value.object);
}

void simplejs_variable_reference_object(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_reference(variable->value.object);
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

void SIMPLEJS_API simplejs_variable_dereference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t mask = -(variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT);

    uintptr_t object_f = (uintptr_t)simplejs_variable_dereference_object;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_jumptable_undef;
    simplejs_variable_jumptable_f ptr_f = (void *)((object_f & mask) | (undef_f & ~mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_reference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t mask = -(variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT);

    uintptr_t object_f = (uintptr_t)simplejs_variable_reference_object;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_jumptable_undef;
    simplejs_variable_jumptable_f ptr_f = (void *)((object_f & mask) | (undef_f & ~mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_lock_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t mask = -(variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT);

    uintptr_t object_f = (uintptr_t)simplejs_variable_lock_gc_object;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_jumptable_undef;
    simplejs_variable_jumptable_f ptr_f = (void *)((object_f & mask) | (undef_f & ~mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_unlock_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uintptr_t mask = -(variable->type == SIMPLEJS_VARIABLE_TYPE_OBJECT);

    uintptr_t object_f = (uintptr_t)simplejs_variable_unlock_gc_object;
    uintptr_t undef_f = (uintptr_t)simplejs_variable_jumptable_undef;
    simplejs_variable_jumptable_f ptr_f = (void *)((object_f & mask) | (undef_f & ~mask));

    ptr_f(variable);
}

void SIMPLEJS_API simplejs_variable_init_undefined(simplejs_variable_t *variable)
{
    variable->value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;
    variable->type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
}

void SIMPLEJS_API simplejs_variable_init_null(simplejs_variable_t *variable)
{
    variable->value.number.type = SIMPLEJS_NUMBER_TYPE_NULL;
    variable->type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
}

void SIMPLEJS_API simplejs_variable_init_number(simplejs_variable_t *variable, simplejs_number_t *number)
{
    variable->value.number = *number;
    variable->type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
}

void SIMPLEJS_API simplejs_variable_init_object(simplejs_variable_t *variable, void *object)
{
    variable->value.object = object;
    variable->type = SIMPLEJS_VARIABLE_TYPE_OBJECT;
}

void SIMPLEJS_API simplejs_variable_init_function(simplejs_variable_t *variable, simplejs_function_t *function)
{
    variable->value.function = *function;
    variable->type = SIMPLEJS_VARIABLE_TYPE_FUNCTION;
}

void SIMPLEJS_API simplejs_variable_init_fast_string(simplejs_variable_t *variable, char *fast_string)
{
    variable->value.fast_string = fast_string;
    variable->type = SIMPLEJS_VARIABLE_TYPE_FAST_STRING;
}

void SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, simplejs_variable_t *new_variable)
{
    simplejs_variable_t old_variable = *variable;

    simplejs_variable_lock_gc(&old_variable);
    simplejs_variable_dereference(&old_variable);

    *variable = *new_variable;

    simplejs_variable_reference(variable);
    simplejs_variable_unlock_gc(&old_variable);
}
