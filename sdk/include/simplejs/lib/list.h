#pragma once
#include "../default.h"

#define simplejs_init_list_entry(entry, struct_pointer) \
    _simplejs_init_list_entry(entry, (uintptr_t)struct_pointer - (uintptr_t)entry)

typedef struct simplejs_list_entry
{
    intptr_t struct_offset;
    void *prev;
    void *next;
} simplejs_list_entry_t;

void SIMPLEJS_API _simplejs_init_list_entry(simplejs_list_entry_t *entry, intptr_t struct_offset);
pvoid SIMPLEJS_API simplejs_get_list_entry_structure(simplejs_list_entry_t *entry);
void SIMPLEJS_API simplejs_insert_tail_list(simplejs_list_entry_t *head, simplejs_list_entry_t *list_to_insert);
void SIMPLEJS_API simplejs_insert_head_list(simplejs_list_entry_t *head, simplejs_list_entry_t *list_to_insert);
void SIMPLEJS_API simplejs_remove_entry_list(simplejs_list_entry_t *entry);
