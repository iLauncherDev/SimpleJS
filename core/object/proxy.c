#include <object.h>

#define simplejs_proxy_call_if_exists(func, status, ...) \
    if (func)                                            \
        status = func(__VA_ARGS__);

simplejs_status_t SIMPLEJS_API simplejs_alloc_proxy(simplejs_proxy_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_proxy_t *proxy = simplejs_hook_malloc(sizeof(*proxy));
    if (!proxy)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(proxy, sizeof(*proxy));

result:
    return status;
}

void SIMPLEJS_API simplejs_free_proxy(simplejs_proxy_t *proxy)
{
    simplejs_hook_mfree(proxy);
}

#define _simplejs_proxy_set_name_callback(name)                                                                         \
    void SIMPLEJS_API simplejs_proxy_set_##name##_callback(simplejs_proxy_t *proxy, simplejs_proxy_##name##_f callback) \
    {                                                                                                                   \
        SIMPLEJS_ASSERT(proxy != NULL);                                                                                 \
                                                                                                                        \
        proxy->f_##name = callback;                                                                                     \
    }

_simplejs_proxy_set_name_callback(release);

_simplejs_proxy_set_name_callback(lock_property_list);
_simplejs_proxy_set_name_callback(unlock_property_list);
_simplejs_proxy_set_name_callback(get_property_list);

_simplejs_proxy_set_name_callback(get_property_value);
_simplejs_proxy_set_name_callback(set_property_value);
_simplejs_proxy_set_name_callback(delete_property);

_simplejs_proxy_set_name_callback(get_string);

simplejs_status_t simplejs_proxy_release(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_release, status, pointer);

    return status;
}

simplejs_status_t simplejs_proxy_lock_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_lock_property_list, status, pointer);

    return status;
}

simplejs_status_t simplejs_proxy_unlock_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_unlock_property_list, status, pointer);

    return status;
}

simplejs_status_t simplejs_proxy_get_property_list(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_list_entry_t **property_list)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    SIMPLEJS_ASSERT(property_list != NULL);

    simplejs_proxy_call_if_exists(proxy->f_get_property_list, status, pointer, property_list);

    return status;
}

simplejs_status_t simplejs_proxy_get_property_value(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_proxy_call_if_exists(proxy->f_get_property_value, status, pointer, property, out);

    return status;
}

simplejs_status_t simplejs_proxy_set_property_value(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property, simplejs_variable_t *in)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);
    SIMPLEJS_ASSERT(in != NULL);

    simplejs_proxy_call_if_exists(proxy->f_set_property_value, status, pointer, property, in);

    return status;
}

simplejs_status_t simplejs_proxy_delete_property(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, simplejs_variable_t *property)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);

    simplejs_proxy_call_if_exists(proxy->f_delete_property, status, pointer, property);

    return status;
}

simplejs_status_t simplejs_proxy_get_string(simplejs_proxy_t *proxy, simplejs_raw_object_t *pointer, char **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(pointer != NULL);

    SIMPLEJS_ASSERT(out != NULL);

    simplejs_proxy_call_if_exists(proxy->f_get_string, status, pointer, out);

    return status;
}
