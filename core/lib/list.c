#include <lib/list.h>

void SIMPLEJS_API _simplejs_init_list_entry(simplejs_list_entry_t *entry, intptr_t struct_offset)
{
    SIMPLEJS_ASSERT(entry != NULL);

    memclr(entry, sizeof(*entry));

    entry->struct_offset = struct_offset;
    entry->prev = entry;
    entry->next = entry;
}

pvoid SIMPLEJS_API simplejs_get_list_entry_structure(simplejs_list_entry_t *entry)
{
    SIMPLEJS_ASSERT(entry != NULL);
    SIMPLEJS_ASSERT(entry->prev != NULL);
    SIMPLEJS_ASSERT(entry->next != NULL);

    return (char *)entry + entry->struct_offset;
}

void SIMPLEJS_API simplejs_insert_tail_list(simplejs_list_entry_t *head, simplejs_list_entry_t *list_to_insert)
{
    SIMPLEJS_ASSERT(list_to_insert != NULL);
    SIMPLEJS_ASSERT(list_to_insert->next != NULL);
    SIMPLEJS_ASSERT(list_to_insert->prev != NULL);

    SIMPLEJS_ASSERT(head != NULL);
    SIMPLEJS_ASSERT(head->next != NULL);
    SIMPLEJS_ASSERT(head->prev != NULL);

    list_to_insert->next = head;
    list_to_insert->prev = head->prev;

    simplejs_list_entry_t *list_to_insert_prev = list_to_insert->prev;

    head->prev = list_to_insert;
    list_to_insert_prev->next = list_to_insert;
}

void SIMPLEJS_API simplejs_insert_head_list(simplejs_list_entry_t *head, simplejs_list_entry_t *list_to_insert)
{
    SIMPLEJS_ASSERT(list_to_insert != NULL);
    SIMPLEJS_ASSERT(list_to_insert->next != NULL);
    SIMPLEJS_ASSERT(list_to_insert->prev != NULL);

    SIMPLEJS_ASSERT(head != NULL);
    SIMPLEJS_ASSERT(head->next != NULL);
    SIMPLEJS_ASSERT(head->prev != NULL);

    list_to_insert->next = head->next;
    list_to_insert->prev = head;

    simplejs_list_entry_t *list_to_insert_next = list_to_insert->next;

    head->next = list_to_insert;
    list_to_insert_next->prev = list_to_insert;
}

void SIMPLEJS_API simplejs_remove_entry_list(simplejs_list_entry_t *entry)
{
    simplejs_list_entry_t *prev;
    simplejs_list_entry_t *next;

    SIMPLEJS_ASSERT(entry != NULL);

    prev = entry->prev;
    next = entry->next;

    SIMPLEJS_ASSERT(prev != NULL);
    SIMPLEJS_ASSERT(next != NULL);

    if (prev && next)
    {
        prev->next = next;
        next->prev = prev;
    }

    entry->prev = entry;
    entry->next = entry;
}
