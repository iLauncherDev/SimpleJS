#include <mm/gc.h>

simplejs_gc_t simplejs_gc = {0};

void SIMPLEJS_API simplejs_gc_event(bool ignore_expiration_time)
{
    simplejs_safe_list_acquire_lock(&simplejs_gc.object_list, true);

    double object_expiration_time = simplejs_gc.object_expiration_time;
    double current_time = simplejs_get_timestamp_f64();
    uintptr_t iterations = 1;

    while (iterations--)
    {
        simplejs_list_entry_t *end_object = &simplejs_gc.object_list.list;
        simplejs_list_entry_t *current_object = end_object->next;

        while (current_object != end_object)
        {
            simplejs_list_entry_t *next_object = current_object->next;

            simplejs_status_t status = SIMPLEJS_STATUS_UNSUCCESSFUL;
            simplejs_object_t *object = simplejs_get_list_entry_structure(current_object);

            bool got_gc_lock = simplejs_spinlock_acquire(&object->gc_lock, false);
            if (!got_gc_lock)
                goto skip;

            uint32_t flags = object->flags;
            double diff_time_seconds = current_time - object->modification_time;

            if (!ignore_expiration_time && !(flags & SIMPLEJS_OBJECT_FLAG_GC_IMMEDIATE_RELEASE) &&
                diff_time_seconds < object_expiration_time)
                goto skip;

            int reference_count = atomic_load_explicit(&object->reference_count, memory_order_acquire);

            // simplejs_printf("reference_count = %d\n", reference_count);

            if (reference_count < 1)
            {
                simplejs_proxy_context_t context = {
                    .pointer = object->pointer,
                };

                status = simplejs_proxy_release(object->proxy, context);
                if (!SIMPLEJS_SUCCESS(status))
                {
                    simplejs_printf("memory leak warning!\n");
                    goto skip;
                }

                simplejs_remove_entry_from_safe_list(&simplejs_gc.object_list, &object->gc_list_entry, true);
                simplejs_hook_mfree(object);

                simplejs_printf("released object because it have no reference!\n");

                iterations++;
            }

        skip:
            if (!SIMPLEJS_SUCCESS(status) && got_gc_lock)
                simplejs_spinlock_release(&object->gc_lock);

            current_object = next_object;
        }
    }

    simplejs_safe_list_release_lock(&simplejs_gc.object_list);
}

void SIMPLEJS_API simplejs_gc_add_object(simplejs_object_t *object)
{
    SIMPLEJS_ASSERT(object != NULL);

    simplejs_add_entry_to_safe_list(&simplejs_gc.object_list, &object->gc_list_entry, false);
}

simplejs_status_t simplejs_init_gc()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_gc.object_expiration_time = 5.0;
    simplejs_init_safe_list(&simplejs_gc.object_list, &simplejs_gc, 0);

result:
    return status;
}
