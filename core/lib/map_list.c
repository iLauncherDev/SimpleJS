#include <lib/map_list.h>

static inline bool is_last_level(simplejs_map_list_t *map_list, size_t current_level)
{
    return (current_level + 1) < map_list->map_levels;
}

static void free_map_list_recursive(simplejs_map_list_t *map_list, size_t current_level, uintptr_t *current_table)
{
    SIMPLEJS_ASSERT(current_table != NULL);

    if (!*current_table)
        return;

    if (is_last_level(map_list, current_level))
    {
        uintptr_t *current_list = (uintptr_t *)*current_table;

        for (size_t i = 0; i < map_list->map_size; i++)
            free_map_list_recursive(map_list, current_level + 1, &current_list[i]);

        simplejs_hook_mfree(current_list);
    }
    else
    {
        simplejs_list_entry_t *list_table = (pvoid)*current_table;

        for (size_t i = 0; i < map_list->map_size; i++)
        {
            simplejs_list_entry_t *end_list = &list_table[i];
            simplejs_list_entry_t *current_list = end_list->next;

            while (current_list != end_list)
            {
                simplejs_list_entry_t *next_list = current_list->next;

                simplejs_remove_entry_list(current_list);

                current_list = next_list;
            }
        }

        simplejs_hook_mfree(list_table);
    }

    *current_table = 0;
}

static simplejs_status_t alloc_map_list_recursive(simplejs_map_list_t *map_list, size_t current_level, uintptr_t *current_table, uint64_t key, simplejs_list_entry_t *list_entry)
{
    SIMPLEJS_ASSERT(current_table != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    size_t table_inv_level = map_list->map_levels - (current_level + 1);
    size_t table_index = (key >> (table_inv_level * map_list->map_size_shift)) & map_list->map_size_mask;

    if (is_last_level(map_list, current_level))
    {
        uintptr_t *current_list = (uintptr_t *)*current_table;
        if (!current_list)
        {
            current_list = simplejs_hook_malloc(sizeof(*current_list) * map_list->map_size);
            if (!current_list)
            {
                status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
                goto result;
            }
            memclr(current_list, sizeof(*current_list) * map_list->map_size);
        }

        status = alloc_map_list_recursive(map_list, current_level + 1, &current_list[table_index], key, list_entry);
        if (!SIMPLEJS_SUCCESS(status))
        {
            simplejs_hook_mfree(current_list);
            goto result;
        }

        *current_table = (uintptr_t)current_list;
    }
    else
    {
        simplejs_list_entry_t *list_table = (pvoid)*current_table;
        if (!list_table)
        {
            list_table = simplejs_hook_malloc(sizeof(*list_table) * map_list->map_size);
            if (!list_table)
            {
                status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
                goto result;
            }

            for (size_t i = 0; i < map_list->map_size; i++)
                simplejs_init_list_entry(&list_table[i], &list_table[i]);
        }

        simplejs_insert_tail_list(&list_table[table_index], list_entry);

        *current_table = (uintptr_t)list_table;
    }

result:
    return status;
}

static pvoid lookup_map_list_recursive(simplejs_map_list_t *map_list, size_t current_level, uintptr_t *current_table, uint64_t key, simplejs_map_list_check_entry_f check_entry_f, void *context, size_t *out_iterations)
{
    SIMPLEJS_ASSERT(current_table != NULL);
    if (!*current_table)
        return NULL;

    size_t table_inv_level = map_list->map_levels - (current_level + 1);
    size_t table_index = (key >> (table_inv_level * map_list->map_size_shift)) & map_list->map_size_mask;

    if (is_last_level(map_list, current_level))
    {
        uintptr_t *current_list = (uintptr_t *)*current_table;

        return lookup_map_list_recursive(map_list, current_level + 1, &current_list[table_index], key, check_entry_f, context, out_iterations);
    }
    else
    {
        simplejs_list_entry_t *list_table = (pvoid)*current_table;

        simplejs_list_entry_t *end_list = &list_table[table_index];
        simplejs_list_entry_t *current_list = end_list->next;

        while (current_list != end_list)
        {
            simplejs_list_entry_t *next_list = current_list->next;
            pvoid entry = simplejs_get_list_entry_structure(current_list);
            if (check_entry_f(key, entry, context))
                return entry;

            current_list = next_list;
            if (out_iterations)
                (*out_iterations)++;
        }
    }

    return NULL;
}

void SIMPLEJS_API simplejs_init_map_list(simplejs_map_list_t *map_list, size_t map_size, size_t map_levels)
{
    if (map_levels < 1)
        map_levels = 1;

    int out_map_size_shift = 31;
    while (out_map_size_shift > 3)
    {
        if (map_size & (size_t)(1 << out_map_size_shift))
            break;

        out_map_size_shift--;
    }

    int out_map_size = 1 << out_map_size_shift;
    int out_map_size_mask = map_size - 1;

    simplejs_map_list_t out_map = {
        .map_size = out_map_size,
        .map_size_shift = out_map_size_shift,
        .map_size_mask = out_map_size_mask,

        .map_levels = map_levels,
    };

    memcpy((pvoid)map_list, &out_map, sizeof(out_map));
}

void SIMPLEJS_API simplejs_uninit_map_list(simplejs_map_list_t *map_list)
{
    free_map_list_recursive(map_list, 0, &map_list->map_table);
}

simplejs_status_t SIMPLEJS_API simplejs_add_entry_to_map_list(simplejs_map_list_t *map_list, uint64_t key, simplejs_list_entry_t *list_entry)
{
    SIMPLEJS_ASSERT(map_list != NULL);
    SIMPLEJS_ASSERT(list_entry != NULL);

    return alloc_map_list_recursive(map_list, 0, &map_list->map_table, key, list_entry);
}

pvoid SIMPLEJS_API simplejs_find_entry_on_map_list(simplejs_map_list_t *map_list, uint64_t key, simplejs_map_list_check_entry_f check_entry_f, void *context, size_t *out_iterations)
{
    SIMPLEJS_ASSERT(map_list != NULL);
    SIMPLEJS_ASSERT(check_entry_f != NULL);

    if (out_iterations)
        *out_iterations = 0;

    return lookup_map_list_recursive(map_list, 0, &map_list->map_table, key, check_entry_f, context, out_iterations);
}
