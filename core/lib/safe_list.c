#include <lib/safe_list.h>

void SIMPLEJS_API _simplejs_init_safe_list(simplejs_safe_list_t *safe_list, intptr_t struct_offset)
{
    memclr(safe_list, sizeof(*safe_list));

    simplejs_init_spinlock(&safe_list->list_lock);
    _simplejs_init_list_entry(&safe_list->list, struct_offset);
}

bool SIMPLEJS_API simplejs_safe_list_acquire_lock(simplejs_safe_list_t *safe_list, bool wait_flag)
{
    return simplejs_spinlock_acquire(&safe_list->list_lock, wait_flag);
}

void SIMPLEJS_API simplejs_safe_list_release_lock(simplejs_safe_list_t *safe_list)
{
    simplejs_spinlock_release(&safe_list->list_lock);
}

bool SIMPLEJS_API simplejs_check_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock)
{
    bool status = false;

    if (!avoid_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    simplejs_list_entry_t *end_list = &safe_list->list;
    simplejs_list_entry_t *current_list = end_list->next;

    while (current_list != end_list)
    {
        if (current_list == entry)
        {
            status = true;
            goto result;
        }

        current_list = current_list->next;
    }

result:
    if (!avoid_lock)
        simplejs_safe_list_release_lock(safe_list);

    return status;
}

void SIMPLEJS_API simplejs_add_entry_to_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock)
{
    if (!avoid_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    simplejs_insert_tail_list(&safe_list->list, entry);

    if (!avoid_lock)
        simplejs_safe_list_release_lock(safe_list);
}

void SIMPLEJS_API simplejs_remove_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock)
{
    if (!avoid_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    simplejs_remove_entry_list(entry);

    if (!avoid_lock)
        simplejs_safe_list_release_lock(safe_list);
}
