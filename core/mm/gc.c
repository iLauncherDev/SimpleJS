#include <mm/gc.h>

simplejs_gc_t simplejs_gc = {0};

void SIMPLEJS_API simplejs_gc_event()
{
    simplejs_safe_list_acquire_lock(&simplejs_gc.object_list, true);

    uintptr_t iterations = 1;

    while (iterations--)
    {
        simplejs_list_entry_t *end_object = &simplejs_gc.object_list.list;
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

            // simplejs_printf("reference_count = %d\n", reference_count);

            if (reference_count < 1)
            {
                simplejs_proxy_context_t context = {
                    .pointer = object->pointer,
                };

                status = simplejs_proxy_release(object->proxy, context);
                if (!SIMPLEJS_SUCCESS(status))
                {
                    simplejs_printf("the memory maybe leaked!\n");
                    goto skip;
                }

                simplejs_remove_entry_list(&object->gc_list_entry);
                simplejs_hook_mfree(object);

                simplejs_printf("released object because it have no reference!\n");

                iterations++;
            }

        skip:
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

    simplejs_init_safe_list(&simplejs_gc.object_list, &simplejs_gc);

result:
    return status;
}
