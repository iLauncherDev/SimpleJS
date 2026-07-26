#include <lib/map_buffer.h>

pvoid SIMPLEJS_PLATFORM_API simplejs_get_map_buffer_context(simplejs_map_buffer_t *map_buffer)
{
    SIMPLEJS_ASSERT(map_buffer != NULL);

    return map_buffer->context;
}

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_alloc_map_buffer(
    simplejs_map_buffer_t **out, void *context,
    simplejs_map_buffer_read_f read_callback,
    simplejs_map_buffer_write_f write_callback,
    simplejs_map_buffer_get_size_f get_size_callback,
    simplejs_map_buffer_close_f close_callback)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_map_buffer_t *map_buffer = simplejs_hook_malloc(sizeof(*map_buffer));
    if (!map_buffer)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(map_buffer, sizeof(*map_buffer));

    map_buffer->context = context;
    map_buffer->read_callback = read_callback;
    map_buffer->write_callback = write_callback;
    map_buffer->get_size_callback = get_size_callback;
    map_buffer->close_callback = close_callback;

    *out = map_buffer;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (map_buffer)
            simplejs_hook_mfree(map_buffer);
    }

    return status;
}

size_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_read(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length)
{
    SIMPLEJS_ASSERT(map_buffer != NULL);

    if (map_buffer->read_callback)
        return map_buffer->read_callback(map_buffer, buffer, offset, length);

    return 0;
}

size_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_write(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length)
{
    SIMPLEJS_ASSERT(map_buffer != NULL);

    if (map_buffer->write_callback)
        return map_buffer->write_callback(map_buffer, buffer, offset, length);

    return 0;
}

uint64_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_get_size(simplejs_map_buffer_t *map_buffer)
{
    SIMPLEJS_ASSERT(map_buffer != NULL);

    if (map_buffer->get_size_callback)
        return map_buffer->get_size_callback(map_buffer);

    return 0;
}

void SIMPLEJS_PLATFORM_API simplejs_destroy_map_buffer(simplejs_map_buffer_t *map_buffer)
{
    SIMPLEJS_ASSERT(map_buffer != NULL);

    if (map_buffer->close_callback)
        map_buffer->close_callback(map_buffer);

    simplejs_hook_mfree(map_buffer);
}
