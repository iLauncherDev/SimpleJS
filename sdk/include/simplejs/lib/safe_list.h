#pragma once
#include "spinlock.h"
#include "list.h"

#define SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK (1 << 0)

#define simplejs_init_safe_list(safe_list, struct_pointer, flags) \
    _simplejs_init_safe_list(safe_list, (uintptr_t)struct_pointer - (uintptr_t)safe_list, flags)

#define simplejs_init_safe_list_entry(safe_list_entry, struct_pointer) \
    _simplejs_init_safe_list_entry(safe_list_entry, (uintptr_t)struct_pointer - (uintptr_t)safe_list_entry)

typedef struct simplejs_safe_list_entry simplejs_safe_list_entry_t;
typedef struct simplejs_safe_list simplejs_safe_list_t;

struct simplejs_safe_list_entry
{
    simplejs_list_entry_t list_entry;
    simplejs_safe_list_t *attached_list;
};

struct simplejs_safe_list
{
    simplejs_list_entry_t list;
    simplejs_spinlock_t list_lock;
    int list_count;
    uint32_t flags;
};

void SIMPLEJS_API _simplejs_init_safe_list(simplejs_safe_list_t *safe_list, intptr_t struct_offset, uint32_t flags);
void SIMPLEJS_API _simplejs_init_safe_list_entry(simplejs_safe_list_entry_t *safe_list_entry, intptr_t struct_offset);

bool SIMPLEJS_API simplejs_safe_list_acquire_lock(simplejs_safe_list_t *safe_list, bool wait_flag);
void SIMPLEJS_API simplejs_safe_list_release_lock(simplejs_safe_list_t *safe_list);
bool SIMPLEJS_API simplejs_check_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock);
void SIMPLEJS_API simplejs_add_entry_to_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock);
void SIMPLEJS_API simplejs_remove_entry_from_safe_list(simplejs_safe_list_t *safe_list, simplejs_safe_list_entry_t *entry, bool avoid_lock);
