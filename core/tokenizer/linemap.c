#include <linemap.h>

static simplejs_status_t simplejs_add_linemap(simplejs_linemap_ctx_t *linemap_ctx, uint64_t line, uint64_t start, uint64_t end)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_linemap_t *linemap = simplejs_hook_malloc(sizeof(*linemap));
    if (!linemap)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(linemap, sizeof(*linemap));

    linemap->line = line;
    linemap->offset.start = start;
    linemap->offset.end = end;
    simplejs_init_list_entry(&linemap->list_entry, linemap);

    simplejs_insert_tail_list(&linemap_ctx->linemap_list, &linemap->list_entry);

result:
    return status;
}

static psimplejs_linemap_t simplejs_get_linemap_entry_ex(simplejs_linemap_ctx_t *linemap_ctx, void *context, simplejs_linemap_callback_t linemap_callback)
{
    simplejs_list_entry_t *end_linemap = &linemap_ctx->linemap_list;
    simplejs_list_entry_t *current_linemap = end_linemap->next;

    while (current_linemap != end_linemap)
    {
        simplejs_list_entry_t *next_linemap = current_linemap->next;
        simplejs_linemap_t *linemap = simplejs_get_list_entry_structure(current_linemap);
        if (linemap_callback && linemap_callback(linemap, context))
            return linemap;

        current_linemap = next_linemap;
    }

    return NULL;
}

typedef struct linemap_offset_context
{
    uint64_t offset;
    simplejs_linemap_position_t *out_position;
} linemap_offset_context_t;

static bool simplejs_get_linemap_offset_callback(simplejs_linemap_t *linemap, void *context)
{
    linemap_offset_context_t *offset_context = context;

    bool result = offset_context->offset >= linemap->offset.start && offset_context->offset < linemap->offset.end;
    if (result)
    {
        offset_context->out_position->line = linemap->line;
        offset_context->out_position->column = offset_context->offset - linemap->offset.start;
    }

    return result;
}

static bool simplejs_get_linemap_line_callback(simplejs_linemap_t *linemap, void *context)
{
    uint64_t line = *(uint64_t *)context;

    return linemap->line == line;
}

psimplejs_linemap_t SIMPLEJS_API simplejs_get_linemap_by_offset(
    simplejs_linemap_ctx_t *linemap_ctx, uint64_t offset, simplejs_linemap_position_t *out_position)
{
    linemap_offset_context_t offset_context = {
        .offset = offset,
        .out_position = out_position,
    };

    return simplejs_get_linemap_entry_ex(linemap_ctx, &offset_context, simplejs_get_linemap_offset_callback);
}

psimplejs_linemap_t SIMPLEJS_API simplejs_get_linemap_by_line(
    simplejs_linemap_ctx_t *linemap_ctx, uint64_t line)
{
    return simplejs_get_linemap_entry_ex(linemap_ctx, &line, simplejs_get_linemap_line_callback);
}

simplejs_char_cache_fetch_t *simplejs_get_linemap_cache_fetch(simplejs_linemap_ctx_t *linemap_ctx)
{
    SIMPLEJS_ASSERT(linemap_ctx != NULL);

    return &linemap_ctx->cache_fetch;
}

void SIMPLEJS_API simplejs_free_linemap_ctx(simplejs_linemap_ctx_t *linemap_ctx)
{
    SIMPLEJS_ASSERT(linemap_ctx != NULL);

    simplejs_list_entry_t *end_linemap = &linemap_ctx->linemap_list;
    simplejs_list_entry_t *current_linemap = end_linemap->next;

    while (current_linemap != end_linemap)
    {
        simplejs_list_entry_t *next_linemap = current_linemap->next;
        simplejs_linemap_t *linemap = simplejs_get_list_entry_structure(current_linemap);

        simplejs_hook_mfree(linemap);

        current_linemap = next_linemap;
    }

    if (linemap_ctx->file_path)
        free(linemap_ctx->file_path);

    simplejs_hook_mfree(linemap_ctx);
}

simplejs_status_t SIMPLEJS_API simplejs_generate_linemap(char *file_path, simplejs_map_buffer_t *source_code, simplejs_linemap_ctx_t **out)
{
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_linemap_ctx_t *linemap_ctx = simplejs_hook_malloc(sizeof(*linemap_ctx));
    if (!linemap_ctx)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(linemap_ctx, sizeof(*linemap_ctx));

    linemap_ctx->source_code = source_code;
    simplejs_init_char_cache_fetch(&linemap_ctx->cache_fetch, source_code);
    simplejs_init_list_entry(&linemap_ctx->linemap_list, linemap_ctx);

    if (file_path)
    {
        linemap_ctx->file_path = strdup(file_path);
        if (!linemap_ctx->file_path)
        {
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
            goto result;
        }
    }

    uint64_t line = 0;
    uint64_t index = 0, line_start_index = 0;

    while (true)
    {
        char chr1;
        bool is_valid = simplejs_fetch_char_from_cache(&linemap_ctx->cache_fetch, index, &chr1);

        if (chr1 == '\n' || !is_valid)
        {
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_linemap(linemap_ctx, line, line_start_index, index), result, status);

            line_start_index = index + 1;
            line++;
        }

        if (!is_valid)
            break;

        index++;
    }

    *out = linemap_ctx;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (linemap_ctx)
            simplejs_free_linemap_ctx(linemap_ctx);
    }

    return status;
}
