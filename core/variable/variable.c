#include <variable.h>
#include <object.h>

uint64_t SIMPLEJS_API simplejs_variable_get_int(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    uint64_t is_number_mask = -(uint64_t)(variable->type == SIMPLEJS_VARIABLE_TYPE_NUMBER);
    uint64_t number_value = simplejs_number_get_int64(&variable->value.number);
    uint64_t object_value = (uintptr_t)variable->value.object;

    return (number_value & is_number_mask) + (object_value & ~is_number_mask);
}

void SIMPLEJS_API simplejs_variable_dereference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
        return;

    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_dereference(variable->value.object);
}

void SIMPLEJS_API simplejs_variable_reference(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
        return;

    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_reference(variable->value.object);
}

void SIMPLEJS_API simplejs_variable_lock_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
        return;

    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_lock_gc(variable->value.object);
}

void SIMPLEJS_API simplejs_variable_release_gc(simplejs_variable_t *variable)
{
    SIMPLEJS_ASSERT(variable != NULL);

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
        return;

    SIMPLEJS_ASSERT(variable->value.object != NULL);

    simplejs_object_release_gc(variable->value.object);
}

void SIMPLEJS_API simplejs_variable_assign(simplejs_variable_t *variable, simplejs_variable_t *new_variable)
{
    simplejs_variable_t old_variable = *variable;

    simplejs_variable_lock_gc(&old_variable);
    simplejs_variable_dereference(&old_variable);

    *variable = *new_variable;

    simplejs_variable_reference(variable);
    simplejs_variable_release_gc(&old_variable);
}
