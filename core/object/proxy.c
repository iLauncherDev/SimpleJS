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

    *out = proxy;

result:
    return status;
}

void SIMPLEJS_API simplejs_free_proxy(simplejs_proxy_t *proxy)
{
    simplejs_hook_mfree(proxy);
}

void SIMPLEJS_API simplejs_init_proxy_property_query(simplejs_proxy_property_query_t *out)
{
    memclr(out, sizeof(*out));
}

void SIMPLEJS_API simplejs_delete_proxy_property_query(simplejs_proxy_property_query_t *out)
{
    simplejs_variable_dereference(&out->property.value);

    simplejs_init_proxy_property_query(out);
}

#define _simplejs_proxy_define_name_callback_impl(name) \
    _simplejs_proxy_define_name_callback_decl(name)     \
    {                                                \
        SIMPLEJS_ASSERT(proxy != NULL);              \
                                                     \
        proxy->f_##name = callback;                  \
    }

_simplejs_proxy_define_name_callback_impl(release);

_simplejs_proxy_define_name_callback_impl(lock_property_list);
_simplejs_proxy_define_name_callback_impl(unlock_property_list);
_simplejs_proxy_define_name_callback_impl(query_property);

_simplejs_proxy_define_name_callback_impl(get_property_value);
_simplejs_proxy_define_name_callback_impl(set_property_value);
_simplejs_proxy_define_name_callback_impl(delete_property);

_simplejs_proxy_define_name_callback_impl(get_string);

simplejs_status_t SIMPLEJS_API simplejs_proxy_release(simplejs_proxy_t *proxy, simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_release, status, context);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_lock_property_list(simplejs_proxy_t *proxy, simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_lock_property_list, status, context);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_unlock_property_list(simplejs_proxy_t *proxy, simplejs_proxy_context_t context)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    simplejs_proxy_call_if_exists(proxy->f_unlock_property_list, status, context);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_query_property(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_proxy_property_query_t *out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    SIMPLEJS_ASSERT(out != NULL);

    simplejs_proxy_call_if_exists(proxy->f_query_property, status, context, out);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_get_property_value(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_proxy_call_if_exists(proxy->f_get_property_value, status, context, property, out);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_set_property_value(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property, simplejs_variable_t *in)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);
    SIMPLEJS_ASSERT(in != NULL);

    simplejs_proxy_call_if_exists(proxy->f_set_property_value, status, context, property, in);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_delete_property(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, simplejs_variable_t *property)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    SIMPLEJS_ASSERT(property != NULL);

    simplejs_proxy_call_if_exists(proxy->f_delete_property, status, context, property);

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_proxy_get_string(simplejs_proxy_t *proxy, simplejs_proxy_context_t context, char **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    SIMPLEJS_ASSERT(proxy != NULL);
    SIMPLEJS_ASSERT(context.pointer != NULL);

    SIMPLEJS_ASSERT(out != NULL);

    simplejs_proxy_call_if_exists(proxy->f_get_string, status, context, out);

    return status;
}
