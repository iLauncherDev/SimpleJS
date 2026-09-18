#pragma once
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*simplejs_map_list_check_entry_f)(uint64_t key, pvoid entry, void *context);

typedef struct simplejs_map_list
{
    const size_t map_size, map_size_shift, map_size_mask, map_levels;

    uintptr_t map_table;
} simplejs_map_list_t;

void SIMPLEJS_API simplejs_init_map_list(simplejs_map_list_t *map_list, size_t map_size, size_t map_levels);
void SIMPLEJS_API simplejs_uninit_map_list(simplejs_map_list_t *map_list);

simplejs_status_t SIMPLEJS_API simplejs_add_entry_to_map_list(simplejs_map_list_t *map_list, uint64_t key, simplejs_list_entry_t *list_entry);
pvoid SIMPLEJS_API simplejs_find_entry_on_map_list(simplejs_map_list_t *map_list, uint64_t key, simplejs_map_list_check_entry_f check_entry_f, void *context, size_t *out_iterations);

#ifdef __cplusplus
}
#endif
