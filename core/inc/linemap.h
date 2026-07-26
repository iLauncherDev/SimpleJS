#pragma once
#include "default.h"
#include "lib/char_cache_fetch.h"
#include <simplejs/linemap.h>

typedef bool (*simplejs_linemap_callback_t)(simplejs_linemap_t *linemap, void *context);

struct simplejs_linemap
{
    uint64_t line;

    simplejs_linemap_offset_t offset;

    simplejs_list_entry_t list_entry;
};

struct simplejs_linemap_ctx
{
    char *file_path;
    simplejs_map_buffer_t *source_code;
    simplejs_char_cache_fetch_t cache_fetch;

    simplejs_list_entry_t linemap_list;
};

simplejs_char_cache_fetch_t *simplejs_get_linemap_cache_fetch(simplejs_linemap_ctx_t *linemap_ctx);
