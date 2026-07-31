#pragma once
#include "spinlock.h"
#include "list.h"

#define simplejs_init_safe_list(safe_list, struct_pointer) \
    _simplejs_init_safe_list(safe_list, (uintptr_t)struct_pointer - (uintptr_t)safe_list)

typedef struct simplejs_safe_list
{
    simplejs_spinlock_t list_lock;
    simplejs_list_entry_t list;
} simplejs_safe_list_t;

void SIMPLEJS_API _simplejs_init_safe_list(simplejs_safe_list_t *safe_list, intptr_t struct_offset);
bool SIMPLEJS_API simplejs_safe_list_acquire_lock(simplejs_safe_list_t *safe_list, bool wait_flag);
void SIMPLEJS_API simplejs_safe_list_release_lock(simplejs_safe_list_t *safe_list);
bool SIMPLEJS_API simplejs_check_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock);
void SIMPLEJS_API simplejs_add_entry_to_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock);
void SIMPLEJS_API simplejs_remove_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_list_entry_t *entry, bool avoid_lock);
