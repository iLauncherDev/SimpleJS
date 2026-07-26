#pragma once
#include "../default.h"
#include <simplejs/lib/map_buffer.h>

typedef struct simplejs_char_cache_fetch
{
    simplejs_map_buffer_t *map_buffer;

    bool has_valid_data;

    int used_buffer_size;

    uint64_t file_offset;
    uint8_t buffer[4096];
} simplejs_char_cache_fetch_t;

void simplejs_init_char_cache_fetch(simplejs_char_cache_fetch_t *cache_fetch, simplejs_map_buffer_t *map_buffer);
bool simplejs_fetch_char_from_cache(simplejs_char_cache_fetch_t *cache_fetch, uint64_t offset, char *out);
