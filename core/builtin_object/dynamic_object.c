#include <builtin_object/dynamic_object.h>

typedef struct simplejs_dynamic_object_raw
{
    atomic_bool property_lock;
    simplejs_list_entry_t property_list;
} simplejs_dynamic_object_raw_t;

simplejs_status_t simplejs_dynamic_object_lock_property_list(simplejs_raw_object_t *pointer);
simplejs_status_t simplejs_dynamic_object_unlock_property_list(simplejs_raw_object_t *pointer);
simplejs_status_t simplejs_dynamic_object_get_property_list(simplejs_raw_object_t *pointer, simplejs_list_entry_t **property_list);

simplejs_status_t simplejs_dynamic_object_get_property_value(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *out);
simplejs_status_t simplejs_dynamic_object_set_property_value(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *in);

simplejs_proxy_t dynamic_object_proxy = {
    .f_lock_property_list = simplejs_dynamic_object_lock_property_list,
    .f_unlock_property_list = simplejs_dynamic_object_unlock_property_list,
    .f_get_property_list = simplejs_dynamic_object_get_property_list,

    .f_get_property_value = simplejs_dynamic_object_get_property_value,
    .f_set_property_value = simplejs_dynamic_object_set_property_value,
};

simplejs_status_t simplejs_dynamic_object_lock_property_list(simplejs_raw_object_t *pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = pointer;

    while (true)
    {
        bool expected_value = false;

        if (atomic_compare_exchange_weak_explicit(&object->property_lock, &expected_value, true, memory_order_acquire, memory_order_relaxed))
            break;
    }

    return status;
}

simplejs_status_t simplejs_dynamic_object_unlock_property_list(simplejs_raw_object_t *pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = pointer;

    atomic_store_explicit(&object->property_lock, false, memory_order_release);

    return status;
}

simplejs_status_t simplejs_dynamic_object_get_property_list(simplejs_raw_object_t *pointer, simplejs_list_entry_t **property_list)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = pointer;

    *property_list = &object->property_list;

    return status;
}

simplejs_object_property_t *simplejs_dynamic_object_find_property(simplejs_dynamic_object_raw_t *object, char *name, bool create_if_necessary)
{
    simplejs_object_property_t *ret = NULL;
    simplejs_list_entry_t *end_property = &object->property_list;
    simplejs_list_entry_t *current_property = end_property->next;

    simplejs_dynamic_object_lock_property_list(object);

    while (current_property != end_property)
    {
        simplejs_object_property_t *property = simplejs_get_list_entry_structure(current_property);
        if (!strcmp(property->name, name))
        {
            ret = property;
            goto result;
        }

        current_property = current_property->next;
    }

    if (create_if_necessary)
    {
        size_t name_size = strlen(name) + 1;

        ret = simplejs_hook_malloc(sizeof(*ret) + name_size);
        if (!ret)
        {
            goto result;
        }

        char *allocated_name = (char *)((uintptr_t)ret + sizeof(*ret));
    
        memclr(ret, sizeof(*ret));
        memcpy(allocated_name, name, name_size);

        ret->name = allocated_name;

        simplejs_init_list_entry(&ret->list_entry, ret);
        simplejs_insert_tail_list(&object->property_list, &ret->list_entry);
    }

result:
    if (!ret)
        simplejs_dynamic_object_unlock_property_list(object);

    return ret;
}

simplejs_status_t simplejs_dynamic_object_get_property_value(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = pointer;

    SIMPLEJS_ASSERT(property->type == SIMPLEJS_VARIABLE_TYPE_FAST_STRING);

    simplejs_object_property_t *object_property = simplejs_dynamic_object_find_property(object, property->value.fast_string, false);
    if (!object_property)
    {
        status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
        goto result;
    }

    simplejs_variable_assign(out, &object_property->value);

    simplejs_dynamic_object_unlock_property_list(object);
result:
    return status;
}

simplejs_status_t simplejs_dynamic_object_set_property_value(simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *in)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = pointer;

    SIMPLEJS_ASSERT(property->type == SIMPLEJS_VARIABLE_TYPE_FAST_STRING);

    simplejs_object_property_t *object_property = simplejs_dynamic_object_find_property(object, property->value.fast_string, true);
    if (!object_property)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    simplejs_variable_assign(&object_property->value, in);

    simplejs_dynamic_object_unlock_property_list(object);
result:
    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_builtin_create_dynamic_object(simplejs_object_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_dynamic_object_raw_t *dynamic_object = simplejs_hook_malloc(sizeof(*dynamic_object));
    if (!dynamic_object)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(dynamic_object, sizeof(*dynamic_object));
    simplejs_init_list_entry(&dynamic_object->property_list, dynamic_object);

    simplejs_object_t *object = NULL;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_object(dynamic_object, &dynamic_object_proxy, &object), result, status);

    *out = object;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (dynamic_object)
            simplejs_hook_mfree(dynamic_object);

        if (object)
            simplejs_hook_mfree(object);
    }

    return status;
}
