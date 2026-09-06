#include <object.h>

// object functions
simplejs_status_t SIMPLEJS_API simplejs_alloc_object(simplejs_raw_object_t *pointer, simplejs_proxy_t *proxy, simplejs_object_t **out)
{
    SIMPLEJS_ASSERT(pointer != NULL);
    SIMPLEJS_ASSERT(proxy != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_object_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(ret, sizeof(*ret));

    ret->pointer = pointer;
    ret->proxy = proxy;

    simplejs_init_spinlock(&ret->gc_lock);
    simplejs_init_safe_list_entry(&ret->gc_list_entry, ret);

    ret->modification_time = simplejs_get_timestamp_f64();

    *out = ret;

result:
    return status;
}

void SIMPLEJS_API simplejs_free_object(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_hook_mfree(object);
}

void SIMPLEJS_API simplejs_object_lock_gc(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_spinlock_acquire(&object->gc_lock, true);
}

void SIMPLEJS_API simplejs_object_unlock_gc(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_spinlock_release(&object->gc_lock);
}

uint32_t SIMPLEJS_API simplejs_object_get_flags(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    return object->flags;
}

void SIMPLEJS_API simplejs_object_set_flags(simplejs_object_t *object, uint32_t flags)
{
    SIMPLEJS_ASSERT(object != NULL);

    object->flags |= flags;
    object->modification_time = simplejs_get_timestamp_f64();
}

void SIMPLEJS_API simplejs_object_clear_flags(simplejs_object_t *object, uint32_t flags)
{
    SIMPLEJS_ASSERT(object != NULL);

    object->flags &= ~flags;
    object->modification_time = simplejs_get_timestamp_f64();
}

void SIMPLEJS_API simplejs_object_reference(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    atomic_fetch_add_explicit(&object->reference_count, 1, memory_order_relaxed);
    object->modification_time = simplejs_get_timestamp_f64();
}

void SIMPLEJS_API simplejs_object_dereference(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    atomic_fetch_sub_explicit(&object->reference_count, 1, memory_order_relaxed);
    object->modification_time = simplejs_get_timestamp_f64();
}

// object proxy functions
simplejs_status_t SIMPLEJS_API simplejs_object_release(simplejs_object_t *object, uint16_t object_value)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_release(object->proxy, context);
}

simplejs_status_t SIMPLEJS_API simplejs_object_set_std_flags(simplejs_object_t *object, uint16_t object_value, uint32_t std_flags)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_set_std_flags(object->proxy, context, std_flags);
}

simplejs_status_t SIMPLEJS_API simplejs_object_clear_std_flags(simplejs_object_t *object, uint16_t object_value, uint32_t std_flags)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_clear_std_flags(object->proxy, context, std_flags);
}

simplejs_status_t SIMPLEJS_API simplejs_object_lock_property_list(simplejs_object_t *object, uint16_t object_value)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_lock_property_list(object->proxy, context);
}

simplejs_status_t SIMPLEJS_API simplejs_object_unlock_property_list(simplejs_object_t *object, uint16_t object_value)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_unlock_property_list(object->proxy, context);
}

simplejs_status_t SIMPLEJS_API simplejs_object_query_property(simplejs_object_t *object, uint16_t object_value, simplejs_proxy_property_query_t *out)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_query_property(object->proxy, context, out);
}

simplejs_status_t SIMPLEJS_API simplejs_object_get_property_value(simplejs_object_t *object, uint16_t object_value, simplejs_variable_t *property, simplejs_variable_t *out)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_get_property_value(object->proxy, context, property, out);
}

simplejs_status_t SIMPLEJS_API simplejs_object_set_property_value(simplejs_object_t *object, uint16_t object_value, simplejs_variable_t *property, simplejs_variable_t *in)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_set_property_value(object->proxy, context, property, in);
}

simplejs_status_t SIMPLEJS_API simplejs_object_delete_property(simplejs_object_t *object, uint16_t object_value, simplejs_variable_t *property)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_delete_property(object->proxy, context, property);
}

simplejs_status_t SIMPLEJS_API simplejs_object_get_string(simplejs_object_t *object, uint16_t object_value, char **out)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_proxy_context_t context = {
        .pointer = object->pointer,
        .value = object_value,
    };

    return simplejs_proxy_get_string(object->proxy, context, out);
}
