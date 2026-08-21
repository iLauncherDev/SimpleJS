#include <lib/safe_list.h>

void SIMPLEJS_API _simplejs_init_safe_list(simplejs_safe_list_t *safe_list, intptr_t struct_offset, uint32_t flags)
{
    memclr(safe_list, sizeof(*safe_list));

    safe_list->flags = flags;
    _simplejs_init_list_entry(&safe_list->list, struct_offset);
    simplejs_init_spinlock(&safe_list->list_lock);
}

void SIMPLEJS_API _simplejs_init_safe_list_entry(simplejs_safe_list_entry_t *safe_list_entry, intptr_t struct_offset)
{
    memclr(safe_list_entry, sizeof(*safe_list_entry));

    _simplejs_init_list_entry(&safe_list_entry->list_entry, struct_offset);
}

bool SIMPLEJS_API simplejs_safe_list_acquire_lock(simplejs_safe_list_t *safe_list, bool wait_flag)
{
    return simplejs_spinlock_acquire(&safe_list->list_lock, wait_flag);
}

void SIMPLEJS_API simplejs_safe_list_release_lock(simplejs_safe_list_t *safe_list)
{
    simplejs_spinlock_release(&safe_list->list_lock);
}

bool SIMPLEJS_API simplejs_check_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock)
{
    bool will_use_lock = !(safe_list->flags & SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK) && !avoid_lock;

    bool status = false;

    if (will_use_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    simplejs_list_entry_t *end_list = &safe_list->list;
    simplejs_list_entry_t *current_list = end_list->next;

    while (current_list != end_list)
    {
        if (current_list == &entry->list_entry)
        {
            status = true;
            goto result;
        }

        current_list = current_list->next;
    }

result:
    if (will_use_lock)
        simplejs_safe_list_release_lock(safe_list);

    return status;
}

void SIMPLEJS_API simplejs_add_entry_to_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock)
{
    bool will_use_lock = !(safe_list->flags & SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK) && !avoid_lock;

    if (will_use_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    if (entry->attached_list)
        goto result;

    entry->attached_list = safe_list;
    simplejs_insert_tail_list(&safe_list->list, &entry->list_entry);

    safe_list->list_count++;

result:
    if (will_use_lock)
        simplejs_safe_list_release_lock(safe_list);
}

void SIMPLEJS_API simplejs_remove_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock)
{
    bool will_use_lock = !(safe_list->flags & SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK) && !avoid_lock;

    if (will_use_lock)
        simplejs_safe_list_acquire_lock(safe_list, true);

    if (entry->attached_list != safe_list)
        goto result;

    entry->attached_list = NULL;
    simplejs_remove_entry_list(&entry->list_entry);

    safe_list->list_count--;

result:
    if (will_use_lock)
        simplejs_safe_list_release_lock(safe_list);
}
