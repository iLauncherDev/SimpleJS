#include <mm/gc.h>

simplejs_gc_t *simplejs_gc = NULL;

void simplejs_gc_lock_object_list()
{
    SIMPLEJS_ASSERT(simplejs_gc != NULL);

    while (true)
    {
        bool expected_value = false;

        if (atomic_compare_exchange_weak_explicit(&simplejs_gc->object_list_lock, &expected_value, true, memory_order_acquire, memory_order_relaxed))
            break;
    }
}

void simplejs_gc_release_object_list()
{
    SIMPLEJS_ASSERT(simplejs_gc != NULL);

    atomic_store_explicit(&simplejs_gc->object_list_lock, false, memory_order_release);
}

void SIMPLEJS_API simplejs_gc_event()
{
    SIMPLEJS_ASSERT(simplejs_gc != NULL);

    simplejs_gc_lock_object_list();

    simplejs_list_entry_t *end_object = &simplejs_gc->object_list;
    simplejs_list_entry_t *current_object = end_object->next;

    while (current_object != end_object)
    {
        simplejs_list_entry_t *next_object = current_object->next;

        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
        simplejs_object_t *object = simplejs_get_list_entry_structure(current_object);

        bool gc_lock = atomic_load_explicit(&object->gc_lock, memory_order_acquire);
        int reference_count = atomic_load_explicit(&object->reference_count, memory_order_acquire);

        if (gc_lock)
            goto skip;

        if (reference_count < 1)
        {
            status = simplejs_proxy_release(object->proxy, object->pointer);
            if (!SIMPLEJS_SUCCESS(status))
            {
                simplejs_printf("the memory maybe leaked!\n");
                goto skip;
            }

            simplejs_remove_entry_list(&object->gc_list_entry);
            simplejs_hook_mfree(object);

            simplejs_printf("released object because it have no reference!\n");
        }

    skip:
        current_object = next_object;
    }

    simplejs_gc_release_object_list();
}

void SIMPLEJS_API simplejs_gc_add_object(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(simplejs_gc != NULL);
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_gc_lock_object_list();

    simplejs_insert_tail_list(&simplejs_gc->object_list, &object->gc_list_entry);

    simplejs_gc_release_object_list();
}

simplejs_status_t simplejs_init_gc()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_gc_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->object_list, ret);

    simplejs_gc = ret;

result:
    return status;
}
