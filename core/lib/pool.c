#include <lib/pool.h>

static void simplejs_free_pool_entry(simplejs_pool_entry_t *pool_entry)
{
    SIMPLEJS_ASSERT(pool_entry != NULL);

    if (pool_entry->data)
        simplejs_page_free(pool_entry->data, pool_entry->data_size, SIMPLEJS_PAGE_FREE_RELEASE_FLAG);

    if (pool_entry->data_map)
        simplejs_hook_mfree(pool_entry->data_map);

    simplejs_hook_mfree(pool_entry);
}

static void simplejs_remove_pool_entry(simplejs_safe_list_t *safe_list,simplejs_pool_entry_t *pool_entry)
{
    SIMPLEJS_ASSERT(pool_entry != NULL);

    simplejs_remove_entry_from_safe_list(safe_list, &pool_entry->safe_list_entry, false);

    if (pool_entry->data)
        simplejs_page_free(pool_entry->data, pool_entry->data_size, SIMPLEJS_PAGE_FREE_RELEASE_FLAG);

    if (pool_entry->data_map)
        simplejs_hook_mfree(pool_entry->data_map);

    simplejs_hook_mfree(pool_entry);
}

static bool simplejs_find_free_pool_map(simplejs_pool_entry_t *pool_entry, size_t block_alloc_size, size_t *out_block)
{
    SIMPLEJS_ASSERT(out_block != NULL);

    size_t i = 0;
    size_t free_entries = 0;

    while (i < pool_entry->data_map_entries)
    {
        size_t entry_value = pool_entry->data_map[i];
        size_t index_skip = !entry_value ? 1 : entry_value;

        free_entries = !entry_value ? (free_entries + 1) : 0;

        if (free_entries >= block_alloc_size)
        {
            *out_block = i - (free_entries - 1);

            return true;
        }

        i += index_skip;
    }

    return false;
}

static bool simplejs_alloc_pool_map(
    simplejs_pool_t *pool,
    simplejs_pool_entry_t *pool_entry, size_t block_aligned_alloc_size,
    pvoid *out)
{
    SIMPLEJS_ASSERT(out != NULL);

    size_t free_data_size = pool_entry->data_size - pool_entry->used_data_size;
    if (free_data_size < block_aligned_alloc_size)
        return false;

    size_t block_alloc_size = block_aligned_alloc_size / pool->block_size;

    size_t free_block;
    if (simplejs_find_free_pool_map(pool_entry, block_alloc_size, &free_block))
    {
        pool_entry->data_map[free_block] = block_alloc_size;
        pool_entry->used_data_size += block_aligned_alloc_size;

        pool_entry->modification_time = simplejs_get_timestamp_f64();

        *out = (uint8_t *)pool_entry->data + (free_block * pool->block_size);
        return true;
    }

    return false;
}

static bool simplejs_create_pool_map_on_pool_entry(
    simplejs_pool_t *pool, simplejs_safe_list_t *safe_list,
    size_t block_aligned_alloc_size, bool create_new_pool_entry, pvoid *out)
{
    bool status = false;

    size_t page_size = simplejs_get_page_size();
    size_t page_aligned_alloc_size = ((block_aligned_alloc_size + (page_size - 1)) / page_size) * page_size;

    simplejs_list_entry_t *end_pool_entry = &safe_list->list;
    simplejs_list_entry_t *current_pool_entry = end_pool_entry->next;

    simplejs_pool_entry_t *selected_pool_entry = NULL;

    while (current_pool_entry != end_pool_entry)
    {
        simplejs_pool_entry_t *pool_entry = simplejs_get_list_entry_structure(current_pool_entry);

        if (simplejs_alloc_pool_map(pool, pool_entry, block_aligned_alloc_size, out))
        {
            selected_pool_entry = pool_entry;

            status = true;
            goto result;
        }

        current_pool_entry = current_pool_entry->next;
    }

    if (!selected_pool_entry && create_new_pool_entry)
    {
        selected_pool_entry = simplejs_hook_malloc(sizeof(*selected_pool_entry));
        if (!selected_pool_entry)
        {
            goto selected_pool_error;
        }
        memclr(selected_pool_entry, sizeof(*selected_pool_entry));

        simplejs_init_safe_list_entry(&selected_pool_entry->safe_list_entry, selected_pool_entry);

        selected_pool_entry->data_size = page_aligned_alloc_size;
        selected_pool_entry->data = simplejs_page_alloc(
            NULL, selected_pool_entry->data_size,
            SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG,
            SIMPLEJS_PAGE_PROTECT_READ_FLAG | SIMPLEJS_PAGE_PROTECT_WRITE_FLAG);
        if (!selected_pool_entry->data)
            goto selected_pool_error;

        selected_pool_entry->data_map_entries = selected_pool_entry->data_size / pool->block_size;
        selected_pool_entry->data_map = simplejs_hook_malloc(selected_pool_entry->data_map_entries * sizeof(*selected_pool_entry->data_map));
        if (!selected_pool_entry->data_map)
            goto selected_pool_error;
        memclr(selected_pool_entry->data_map, selected_pool_entry->data_map_entries * sizeof(*selected_pool_entry->data_map));

        if (!simplejs_alloc_pool_map(pool, selected_pool_entry, block_aligned_alloc_size, out))
            goto selected_pool_error;

        status = true;
        goto result;

    selected_pool_error:
        if (selected_pool_entry)
        {
            simplejs_free_pool_entry(selected_pool_entry);
        }

        selected_pool_entry = NULL;
    }

result:
    if (selected_pool_entry)
    {
        simplejs_remove_entry_from_safe_list(safe_list, &selected_pool_entry->safe_list_entry, false);
        simplejs_add_entry_to_safe_list(&pool->used_alloc_list, &selected_pool_entry->safe_list_entry, false);
    }

    return status;
}

pvoid SIMPLEJS_API simplejs_pool_malloc(simplejs_pool_t *pool, size_t size)
{
    SIMPLEJS_ASSERT(pool != NULL);
    SIMPLEJS_ASSERT(size != 0);

    pvoid ptr = NULL;
    size_t block_aligned_alloc_size = ((size + (pool->block_size - 1)) / pool->block_size) * pool->block_size;

    simplejs_spinlock_acquire(&pool->main_lock, true);

    if (simplejs_create_pool_map_on_pool_entry(pool, &pool->free_alloc_list, block_aligned_alloc_size, false, &ptr))
        goto result;

    if (simplejs_create_pool_map_on_pool_entry(pool, &pool->used_alloc_list, block_aligned_alloc_size, true, &ptr))
        goto result;

result:
    simplejs_spinlock_release(&pool->main_lock);
    return ptr;
}

void SIMPLEJS_API simplejs_pool_mfree(simplejs_pool_t *pool, void *ptr)
{
    SIMPLEJS_ASSERT(pool != NULL);
    SIMPLEJS_ASSERT(ptr != NULL);

    simplejs_spinlock_acquire(&pool->main_lock, true);

    simplejs_list_entry_t *end_pool_entry = &pool->used_alloc_list.list;
    simplejs_list_entry_t *current_pool_entry = end_pool_entry->next;

    while (current_pool_entry != end_pool_entry)
    {
        simplejs_pool_entry_t *pool_entry = simplejs_get_list_entry_structure(current_pool_entry);
        void *pool_data_start = pool_entry->data;
        void *pool_data_end = (uint8_t *)pool_entry->data + pool_entry->data_size;

        if (pool_data_start >= ptr && ptr < pool_data_end)
        {
            uintptr_t ptr_offset = (uintptr_t)ptr - (uintptr_t)pool_data_start;

            size_t ptr_block = ptr_offset / pool->block_size;
            size_t ptr_size_in_blocks = pool_entry->data_map[ptr_block];
            size_t ptr_size = ptr_size_in_blocks * pool->block_size;

            SIMPLEJS_ASSERT(ptr_size_in_blocks != 0);

            if (pool_entry->used_data_size <= ptr_size)
            {
                pool_entry->used_data_size = 0;

                simplejs_remove_entry_from_safe_list(&pool->used_alloc_list, &pool_entry->safe_list_entry, false);
                simplejs_add_entry_to_safe_list(&pool->free_alloc_list, &pool_entry->safe_list_entry, false);
            }
            else
            {
                pool_entry->used_data_size -= ptr_size;
            }

            pool_entry->data_map[ptr_block] = 0;

            pool_entry->modification_time = simplejs_get_timestamp_f64();
            break;
        }

        current_pool_entry = current_pool_entry->next;
    }

    simplejs_spinlock_release(&pool->main_lock);
}

static void simplejs_pool_collect_garbage(simplejs_pool_t *pool, bool ignore_expiration_time)
{
    simplejs_spinlock_acquire(&pool->main_lock, true);

    double current_time = simplejs_get_timestamp_f64();

    simplejs_list_entry_t *end_pool_entry = &pool->free_alloc_list.list;
    simplejs_list_entry_t *current_pool_entry = end_pool_entry->next;

    while (current_pool_entry != end_pool_entry)
    {
        simplejs_list_entry_t *next_pool_entry = current_pool_entry->next;
        simplejs_pool_entry_t *pool_entry = simplejs_get_list_entry_structure(current_pool_entry);

        double diff_time_seconds = current_time - pool_entry->modification_time;

        if (ignore_expiration_time ||
            diff_time_seconds >= pool->allocation_expiration_time)
        {
            simplejs_remove_pool_entry(&pool->free_alloc_list, pool_entry);
        }

        current_pool_entry = next_pool_entry;
    }

    simplejs_spinlock_release(&pool->main_lock);
}

static uint64_t simplejs_pool_thread(simplejs_thread_t *thread)
{
    simplejs_pool_t *pool = simplejs_get_thread_arg(thread);

    SIMPLEJS_ASSERT(pool != NULL);

    while (pool->kill_thread_request)
    {
        simplejs_pool_collect_garbage(pool, false);

        simplejs_sleep(10);
    }

    simplejs_pool_collect_garbage(pool, true);

    return 0;
}

void SIMPLEJS_API simplejs_destroy_pool(simplejs_pool_t *pool)
{
    SIMPLEJS_ASSERT(pool != NULL);
    SIMPLEJS_ASSERT(pool->used_alloc_list.list_count != 0);

    pool->kill_thread_request = true;

    if (pool->thread)
        simplejs_destroy_thread(pool->thread);

    simplejs_hook_mfree(pool);
}

simplejs_status_t SIMPLEJS_API simplejs_create_pool(size_t block_size, simplejs_pool_t **out)
{
    SIMPLEJS_ASSERT(block_size != 0);
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_pool_t *pool = NULL;
    simplejs_thread_t *pool_thread = NULL;

    pool = simplejs_hook_malloc(sizeof(*pool));
    if (!pool)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(pool, sizeof(*pool));

    pool->allocation_expiration_time = 5.0;

    simplejs_init_spinlock(&pool->main_lock);
    simplejs_init_safe_list(&pool->used_alloc_list, pool, SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK);
    simplejs_init_safe_list(&pool->free_alloc_list, pool, SIMPLEJS_SAFE_LIST_FLAG_DISABLE_LOCK);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_create_thread(simplejs_pool_thread, pool, &pool->thread), result, status);

    *out = pool;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (pool)
            simplejs_destroy_pool(pool);
    }

    return status;
}
