#include <lib/map_buffer_file.h>

#define GET_MAP_BUFFER_FILE(map_buffer) (simplejs_file_t *)map_buffer->context

static size_t map_buffer_read(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length)
{
    simplejs_file_t *file = GET_MAP_BUFFER_FILE(map_buffer);
    size_t bytes = 0;

    simplejs_file_seek(file, offset, SEEK_SET);
    bytes = simplejs_file_read(file, buffer, (uint32_t)length);

    return bytes;
}

static size_t map_buffer_write(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length)
{
    simplejs_file_t *file = GET_MAP_BUFFER_FILE(map_buffer);

    size_t bytes = 0;

    simplejs_file_seek(file, offset, SEEK_SET);
    bytes = simplejs_file_write(file, buffer, (uint32_t)length);

    return bytes;
}

static uint64_t map_buffer_get_size(simplejs_map_buffer_t *map_buffer)
{
    size_t bytes = 0;
    simplejs_file_t *file = GET_MAP_BUFFER_FILE(map_buffer);
    
    simplejs_file_seek(file, 0, SEEK_END);
    bytes = simplejs_file_tell(file);

    return bytes;
}

static void map_buffer_close(simplejs_map_buffer_t *map_buffer)
{
    simplejs_file_t *file = GET_MAP_BUFFER_FILE(map_buffer);

    simplejs_file_close(file);
}

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_create_map_buffer_file(simplejs_map_buffer_t **out, char *file_path)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_file_t *file = NULL;
    simplejs_map_buffer_t *map_buffer = NULL;

    file = simplejs_file_open(file_path, "r+");
    if (!file)
    {
        status = SIMPLEJS_STATUS_UNSUCCESSFUL;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_map_buffer(&map_buffer, file,
                                                       map_buffer_read, map_buffer_write,
                                                       map_buffer_get_size,
                                                       map_buffer_close),
                             result, status);

    *out = map_buffer;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (file)
            simplejs_file_close(file);

        if (map_buffer)
            simplejs_hook_mfree(map_buffer);
    }

    return status;
}
