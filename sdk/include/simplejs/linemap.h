#pragma once
#include "default.h"
#include "lib/map_buffer.h"

typedef struct simplejs_linemap simplejs_linemap_t, *psimplejs_linemap_t;
typedef struct simplejs_linemap_ctx simplejs_linemap_ctx_t, *psimplejs_linemap_ctx_t;

typedef struct simplejs_linemap_position
{
    uint64_t line, column;
} simplejs_linemap_position_t;

typedef struct simplejs_linemap_offset
{
    uint64_t start, end;
} simplejs_linemap_offset_t;

psimplejs_linemap_t SIMPLEJS_API simplejs_get_linemap_by_offset(
    simplejs_linemap_ctx_t *linemap_ctx, uint64_t offset, simplejs_linemap_position_t *out_position);
psimplejs_linemap_t SIMPLEJS_API simplejs_get_linemap_by_line(
    simplejs_linemap_ctx_t *linemap_ctx, uint64_t line);

void SIMPLEJS_API simplejs_free_linemap_ctx(simplejs_linemap_ctx_t *linemap_ctx);
simplejs_status_t SIMPLEJS_API simplejs_generate_linemap(char *file_path, simplejs_map_buffer_t *source_code, simplejs_linemap_ctx_t **out);
