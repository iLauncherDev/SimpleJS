#include <builtin_object/dynamic_object.h>

typedef struct simplejs_dynamic_object_property
{
    simplejs_proxy_property_t property;

    simplejs_list_entry_t list_entry;
} simplejs_dynamic_object_property_t;

typedef struct simplejs_dynamic_object_raw
{
    simplejs_safe_list_t property_list;
    bool read_only;
} simplejs_dynamic_object_raw_t;

simplejs_status_t simplejs_dynamic_object_release(simplejs_proxy_context_t context);

simplejs_status_t simplejs_dynamic_object_lock_property_list(simplejs_proxy_context_t context);
simplejs_status_t simplejs_dynamic_object_unlock_property_list(simplejs_proxy_context_t context);
simplejs_status_t simplejs_dynamic_object_query_property(simplejs_proxy_context_t context, simplejs_proxy_property_query_t *out);

simplejs_status_t simplejs_dynamic_object_get_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out);
simplejs_status_t simplejs_dynamic_object_set_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in);
simplejs_status_t simplejs_dynamic_object_delete_property(simplejs_proxy_context_t context, simplejs_variable_t *property);

simplejs_status_t simplejs_dynamic_object_get_string(simplejs_proxy_context_t context, char **out);

simplejs_proxy_t dynamic_object_proxy = {
    .f_release = simplejs_dynamic_object_release,

    .f_lock_property_list = simplejs_dynamic_object_lock_property_list,
    .f_unlock_property_list = simplejs_dynamic_object_unlock_property_list,
    .f_query_property = simplejs_dynamic_object_query_property,

    .f_get_property_value = simplejs_dynamic_object_get_property_value,
    .f_set_property_value = simplejs_dynamic_object_set_property_value,
    .f_delete_property = simplejs_dynamic_object_delete_property,

    .f_get_string = simplejs_dynamic_object_get_string,
};

simplejs_status_t simplejs_dynamic_object_release(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;

    simplejs_list_entry_t *end_property = &object->property_list.list;
    simplejs_list_entry_t *current_property = end_property->next;

    while (current_property != end_property)
    {
        simplejs_list_entry_t *next_property = current_property->next;
        simplejs_dynamic_object_property_t *object_property = simplejs_get_list_entry_structure(current_property);

        simplejs_variable_dereference(&object_property->property.value);
        simplejs_hook_mfree(object_property);

        current_property = next_property;
    }

    simplejs_hook_mfree(object);
    return status;
}

simplejs_status_t simplejs_dynamic_object_lock_property_list(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;

    simplejs_safe_list_acquire_lock(&object->property_list, true);

    return status;
}

simplejs_status_t simplejs_dynamic_object_unlock_property_list(simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;

    simplejs_safe_list_release_lock(&object->property_list);

    return status;
}

simplejs_status_t simplejs_dynamic_object_query_property(simplejs_proxy_context_t context, simplejs_proxy_property_query_t *out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;

    if (!out->_current_pointer)
        out->_current_pointer = (&object->property_list.list)->next;

    if (out->_current_pointer == &object->property_list.list)
    {
        out->query_ended = true;
        goto result;
    }

    simplejs_proxy_property_t *out_property = &out->property;
    simplejs_proxy_property_t *in_property = simplejs_get_list_entry_structure(out->_current_pointer);

    out_property->name = in_property->name;
    simplejs_variable_assign(&out_property->value, &in_property->value);

    out->_current_pointer = ((simplejs_list_entry_t *)out->_current_pointer)->next;

result:
    return status;
}

simplejs_dynamic_object_property_t *simplejs_dynamic_object_find_property(simplejs_proxy_context_t context, char *name, bool create_if_necessary)
{
    simplejs_dynamic_object_property_t *ret = NULL;

    simplejs_dynamic_object_raw_t *object = context.pointer;

    simplejs_list_entry_t *end_property = &object->property_list.list;
    simplejs_list_entry_t *current_property = end_property->next;

    simplejs_dynamic_object_lock_property_list(context);

    while (current_property != end_property)
    {
        simplejs_dynamic_object_property_t *object_property = simplejs_get_list_entry_structure(current_property);
        if (!strcmp(object_property->property.name, name))
        {
            ret = object_property;
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

        ret->property.name = allocated_name;

        simplejs_init_list_entry(&ret->list_entry, ret);
        simplejs_add_entry_to_safe_list(&object->property_list, &ret->list_entry, true);
    }

result:
    if (!ret)
        simplejs_dynamic_object_unlock_property_list(context);

    return ret;
}

simplejs_status_t simplejs_dynamic_object_get_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out)
{
    char tempString[4096];
    char *name;

    simplejs_variable_to_string(property, tempString, sizeof(tempString), &name);

    // printf("simplejs_dynamic_object_get_property_value: %s\n", name);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;

    simplejs_dynamic_object_property_t *object_property = simplejs_dynamic_object_find_property(context, name, false);
    if (!object_property)
    {
        simplejs_variable_t tmp_var;
        simplejs_variable_init_undefined(&tmp_var);

        simplejs_variable_assign(out, &tmp_var);

        // status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
        goto result;
    }

    simplejs_variable_assign(out, &object_property->property.value);

    simplejs_dynamic_object_unlock_property_list(context);
result:
    return status;
}

simplejs_status_t simplejs_dynamic_object_set_property_value(simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in)
{
    char tempString[4096];
    char *name;

    simplejs_variable_to_string(property, tempString, sizeof(tempString), &name);

    // printf("simplejs_dynamic_object_set_property_value: %s\n", name);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;
    if (object->read_only)
    {
        simplejs_printf("called simplejs_dynamic_object_set_property_value on read-only mode!\n");
        goto result;
    }

    simplejs_dynamic_object_property_t *object_property = simplejs_dynamic_object_find_property(context, name, true);
    if (!object_property)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    simplejs_variable_assign(&object_property->property.value, in);

    simplejs_dynamic_object_unlock_property_list(context);
result:
    return status;
}

simplejs_status_t simplejs_dynamic_object_delete_property(simplejs_proxy_context_t context, simplejs_variable_t *property)
{
    char tempString[4096];
    char *name;

    simplejs_variable_to_string(property, tempString, sizeof(tempString), &name);

    // printf("simplejs_dynamic_object_set_property_value: %s\n", name);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_dynamic_object_raw_t *object = context.pointer;
    if (object->read_only)
    {
        simplejs_printf("called simplejs_dynamic_object_set_property_value on read-only mode!\n");
        goto result;
    }

    simplejs_dynamic_object_property_t *object_property = simplejs_dynamic_object_find_property(context, name, false);
    if (!object_property)
    {
        status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
        goto result;
    }

    simplejs_remove_entry_from_safe_list(&object->property_list, &object_property->list_entry, true);

    simplejs_variable_dereference(&object_property->property.value);
    simplejs_hook_mfree(object_property);

    simplejs_dynamic_object_unlock_property_list(context);
result:
    return status;
}

simplejs_status_t simplejs_dynamic_object_get_string(simplejs_proxy_context_t context, char **out)
{
    *out = "[Dynamic Object]";

    return SIMPLEJS_STATUS_SUCCESS;
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
    simplejs_init_safe_list(&dynamic_object->property_list, dynamic_object);

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

void SIMPLEJS_API simplejs_builtin_set_dynamic_object_read_only(simplejs_object_t *object, bool read_only)
{
    SIMPLEJS_ASSERT(object != NULL);
    SIMPLEJS_ASSERT(object->pointer != NULL);

    simplejs_dynamic_object_raw_t *dynamic_object = object->pointer;

    dynamic_object->read_only = read_only;
}
