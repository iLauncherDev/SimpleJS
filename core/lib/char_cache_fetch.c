#include <lib/char_cache_fetch.h>

void simplejs_init_char_cache_fetch(simplejs_char_cache_fetch_t *char_cache_fetch, simplejs_map_buffer_t *map_buffer)
{
    memclr(char_cache_fetch, sizeof(*char_cache_fetch));

    char_cache_fetch->map_buffer = map_buffer;
}

bool simplejs_fetch_char_from_cache(simplejs_char_cache_fetch_t *char_cache_fetch, uint64_t offset, char *out)
{
    size_t buffer_size = sizeof(char_cache_fetch->buffer);

    uint64_t file_offset_start, file_offset_end;
    bool is_cache_hit;

retry_cache:
    file_offset_start = char_cache_fetch->file_offset;
    file_offset_end = file_offset_start + char_cache_fetch->used_buffer_size;
    is_cache_hit = offset >= file_offset_start && offset < file_offset_end;

    if (is_cache_hit)
    {
        if (out)
            *out = char_cache_fetch->buffer[offset - file_offset_start];

        return true;
    }

    uint64_t new_file_offset = (offset / buffer_size) * buffer_size;
    if (char_cache_fetch->used_buffer_size < buffer_size &&
        new_file_offset >= file_offset_start &&
        char_cache_fetch->has_valid_data)
    {
        if (out)
            *out = '\0';

        return false;
    }

    char_cache_fetch->file_offset = new_file_offset;
    char_cache_fetch->used_buffer_size = simplejs_map_buffer_read(
        char_cache_fetch->map_buffer,
        char_cache_fetch->buffer, new_file_offset, buffer_size);
    char_cache_fetch->has_valid_data = true;

    goto retry_cache;
}
