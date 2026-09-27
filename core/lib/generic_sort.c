#include <lib/generic_sort.h>

#define _simplejs_generic_sort_define_name_callback_impl(name) \
    _simplejs_generic_sort_define_name_callback_decl(name)     \
    {                                                          \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                     \
                                                               \
        sort_ctx->f_##name = callback;                         \
    }

#define _simplejs_generic_sort_diff_impl(type, type_t)                           \
    _simplejs_generic_sort_diff_decl(type, type_t)                               \
    {                                                                            \
        SIMPLEJS_ASSERT(sort_a_ctx != NULL);                                     \
        SIMPLEJS_ASSERT(sort_b_ctx != NULL);                                     \
                                                                                 \
        SIMPLEJS_ASSERT(sort_a_ctx->f_diff_##type != NULL);                      \
        SIMPLEJS_ASSERT(sort_b_ctx->f_diff_##type != NULL);                      \
                                                                                 \
        SIMPLEJS_ASSERT(sort_a_ctx->f_diff_##type == sort_b_ctx->f_diff_##type); \
                                                                                 \
        return sort_a_ctx->f_diff_##type(sort_a_ctx, sort_b_ctx);                \
    }

size_t SIMPLEJS_API simplejs_generic_sort_get_struct_size()
{
    return sizeof(simplejs_generic_sort_ctx_t);
}

void SIMPLEJS_API simplejs_init_generic_sort(simplejs_generic_sort_ctx_t *sort_ctx, const_pvoid context, size_t context_size)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sizeof(*sort_ctx) == simplejs_generic_sort_get_struct_size());

    memclr(sort_ctx, sizeof(*sort_ctx));

    sort_ctx->context = context;
    sort_ctx->context_size = context_size;
}

simplejs_status_t SIMPLEJS_API simplejs_alloc_generic_sort(simplejs_generic_sort_ctx_t **out, const_pvoid context, size_t context_size)
{
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_generic_sort_ctx_t *sort_ctx = simplejs_hook_malloc(sizeof(*sort_ctx));
    if (!sort_ctx)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    simplejs_init_generic_sort(sort_ctx, context, context_size);

    *out = sort_ctx;
result:
    return status;
}

void SIMPLEJS_API simplejs_free_generic_sort(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    simplejs_hook_mfree(sort_ctx);
}

void SIMPLEJS_API simplejs_generic_sort_copy(simplejs_generic_sort_ctx_t *dest, const simplejs_generic_sort_ctx_t *src)
{
    memcpy(dest, src, simplejs_generic_sort_get_struct_size());
}

_simplejs_generic_sort_define_name_callback_impl(get_entry_count);
_simplejs_generic_sort_define_name_callback_impl(goto_pointer);
_simplejs_generic_sort_define_name_callback_impl(check_pointer);
_simplejs_generic_sort_define_name_callback_impl(prev_pointer);
_simplejs_generic_sort_define_name_callback_impl(next_pointer);
_simplejs_generic_sort_define_name_callback_impl(diff_f32);
_simplejs_generic_sort_define_name_callback_impl(swap_entries);

size_t SIMPLEJS_API simplejs_generic_sort_get_context_size(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    return sort_ctx->context_size;
}

const_pvoid SIMPLEJS_API simplejs_generic_sort_get_context(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    return sort_ctx->context;
}

pvoid SIMPLEJS_API simplejs_generic_sort_get_temp_context(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    return sort_ctx->temp_context;
}

void SIMPLEJS_API simplejs_generic_sort_set_temp_context(simplejs_generic_sort_ctx_t *sort_ctx, pvoid temp_context)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    sort_ctx->temp_context = temp_context;
}

size_t SIMPLEJS_API simplejs_generic_sort_get_entry_count(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_get_entry_count != NULL);

    return sort_ctx->f_get_entry_count(sort_ctx);
}

void SIMPLEJS_API simplejs_generic_sort_goto_pointer(simplejs_generic_sort_ctx_t *sort_ctx, bool direction)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_goto_pointer != NULL);

    sort_ctx->f_goto_pointer(sort_ctx, direction);
}

bool SIMPLEJS_API simplejs_generic_sort_check_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_check_pointer != NULL);

    sort_ctx->f_check_pointer(sort_ctx);
}

void SIMPLEJS_API simplejs_generic_sort_prev_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_prev_pointer != NULL);

    sort_ctx->f_prev_pointer(sort_ctx);
}

void SIMPLEJS_API simplejs_generic_sort_next_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_next_pointer != NULL);

    sort_ctx->f_next_pointer(sort_ctx);
}

_simplejs_generic_sort_diff_impl(f32, float);

void SIMPLEJS_API simplejs_generic_sort_swap_entries(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx)
{
    SIMPLEJS_ASSERT(sort_a_ctx != NULL);
    SIMPLEJS_ASSERT(sort_b_ctx != NULL);

    SIMPLEJS_ASSERT(sort_a_ctx->f_swap_entries != NULL);
    SIMPLEJS_ASSERT(sort_b_ctx->f_swap_entries != NULL);

    SIMPLEJS_ASSERT(sort_a_ctx->f_swap_entries == sort_b_ctx->f_swap_entries);

    sort_a_ctx->f_swap_entries(sort_a_ctx, sort_b_ctx);
}

#define _simplejs_selection_sort_impl(type, type_t, type_min, type_max, type_epsilon)                                              \
    _simplejs_selection_sort_decl(f32)                                                                                             \
    {                                                                                                                              \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                                                                                         \
                                                                                                                                   \
        uint64_t iterations = 0;                                                                                                   \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                                        \
                                                                                                                                   \
        size_t sort_size = simplejs_generic_sort_get_struct_size();                                                              \
        size_t context_size = simplejs_generic_sort_get_context_size(sort_ctx);                                                    \
                                                                                                                                   \
        size_t temp_heap_count = 3;                                                                                                \
        size_t temp_heap_size = (sort_size + context_size) * temp_heap_count;                                                    \
        char *temp_heap = simplejs_hook_malloc(temp_heap_size);                                                                    \
        if (!temp_heap)                                                                                                            \
        {                                                                                                                          \
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;                                                                             \
            goto result;                                                                                                           \
        }                                                                                                                          \
        memclr(temp_heap, temp_heap_size);                                                                                         \
                                                                                                                                   \
        char *temp_sort_heap = &temp_heap[0];                                                                                      \
        char *temp_context_heap = &temp_sort_heap[sort_size * temp_heap_count];                                                  \
                                                                                                                                   \
        simplejs_generic_sort_ctx_t *main_low_ctx = (void *)&temp_sort_heap[sort_size * 0];                                      \
        simplejs_generic_sort_copy(main_low_ctx, sort_ctx);                                                                        \
        simplejs_generic_sort_set_temp_context(main_low_ctx, &temp_context_heap[context_size * 0]);                                \
        simplejs_generic_sort_goto_pointer(main_low_ctx, false);                                                                   \
                                                                                                                                   \
        while (simplejs_generic_sort_check_pointer(main_low_ctx))                                                                  \
        {                                                                                                                          \
            char *temp_buffer = &temp_context_heap[context_size * 1];                                                              \
            memcpy(temp_buffer, simplejs_generic_sort_get_temp_context(main_low_ctx), context_size);                               \
                                                                                                                                   \
            char *selected_low_buffer = &temp_context_heap[context_size * 2];                                                      \
                                                                                                                                   \
            simplejs_generic_sort_ctx_t *temp_sort_ctx = (void *)&temp_sort_heap[sort_size * 1];                                 \
            simplejs_generic_sort_copy(temp_sort_ctx, main_low_ctx);                                                               \
            simplejs_generic_sort_set_temp_context(temp_sort_ctx, temp_buffer);                                                    \
                                                                                                                                   \
            type_t low_diff = type_min;                                                                                            \
            simplejs_generic_sort_ctx_t *selected_low_ctx = (void *)&temp_sort_heap[sort_size * 2];                              \
            simplejs_generic_sort_copy(selected_low_ctx, temp_sort_ctx);                                                           \
                                                                                                                                   \
            while (simplejs_generic_sort_check_pointer(temp_sort_ctx))                                                             \
            {                                                                                                                      \
                type_t current_diff = simplejs_generic_sort_diff_##type(main_low_ctx, temp_sort_ctx);                              \
                                                                                                                                   \
                if (current_diff > low_diff)                                                                                       \
                {                                                                                                                  \
                    low_diff = current_diff;                                                                                       \
                                                                                                                                   \
                    memcpy(selected_low_buffer, temp_buffer, context_size);                                                        \
                    simplejs_generic_sort_set_temp_context(selected_low_ctx, selected_low_buffer);                                 \
                }                                                                                                                  \
                                                                                                                                   \
                simplejs_generic_sort_next_pointer(temp_sort_ctx);                                                                 \
                iterations++;                                                                                                      \
            }                                                                                                                      \
                                                                                                                                   \
            if (low_diff > type_epsilon &&                                                                                         \
                simplejs_generic_sort_get_temp_context(selected_low_ctx) != simplejs_generic_sort_get_temp_context(temp_sort_ctx)) \
                simplejs_generic_sort_swap_entries(main_low_ctx, selected_low_ctx);                                                \
                                                                                                                                   \
            simplejs_generic_sort_next_pointer(main_low_ctx);                                                                      \
        }                                                                                                                          \
                                                                                                                                   \
        if (out_iterations)                                                                                                        \
            *out_iterations = iterations;                                                                                          \
                                                                                                                                   \
    result:                                                                                                                        \
        if (temp_heap)                                                                                                             \
            simplejs_hook_mfree(temp_heap);                                                                                        \
                                                                                                                                   \
        return status;                                                                                                             \
    }

#define _simplejs_selection_sort_half_impl(type, type_t, type_min, type_max, type_epsilon)                                          \
    _simplejs_selection_sort_half_decl(f32)                                                                                         \
    {                                                                                                                               \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                                                                                          \
                                                                                                                                    \
        uint64_t iterations = 0;                                                                                                    \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                                         \
                                                                                                                                    \
        size_t sort_size = simplejs_generic_sort_get_struct_size();                                                               \
        size_t context_size = simplejs_generic_sort_get_context_size(sort_ctx);                                                     \
                                                                                                                                    \
        size_t temp_heap_count = 5;                                                                                                 \
        size_t temp_heap_size = (sort_size + context_size) * temp_heap_count;                                                     \
        char *temp_heap = simplejs_hook_malloc(temp_heap_size);                                                                     \
        if (!temp_heap)                                                                                                             \
        {                                                                                                                           \
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;                                                                              \
            goto result;                                                                                                            \
        }                                                                                                                           \
        memclr(temp_heap, temp_heap_size);                                                                                          \
                                                                                                                                    \
        char *temp_sort_heap = &temp_heap[0];                                                                                       \
        char *temp_context_heap = &temp_sort_heap[sort_size * temp_heap_count];                                                   \
                                                                                                                                    \
        simplejs_generic_sort_ctx_t *main_low_ctx = (void *)&temp_sort_heap[sort_size * 0];                                       \
        simplejs_generic_sort_copy(main_low_ctx, sort_ctx);                                                                         \
        simplejs_generic_sort_set_temp_context(main_low_ctx, &temp_context_heap[context_size * 0]);                                 \
        simplejs_generic_sort_goto_pointer(main_low_ctx, false);                                                                    \
                                                                                                                                    \
        simplejs_generic_sort_ctx_t *main_high_ctx = (void *)&temp_sort_heap[sort_size * 1];                                      \
        simplejs_generic_sort_copy(main_high_ctx, sort_ctx);                                                                        \
        simplejs_generic_sort_set_temp_context(main_high_ctx, &temp_context_heap[context_size * 1]);                                \
        simplejs_generic_sort_goto_pointer(main_high_ctx, true);                                                                    \
                                                                                                                                    \
        size_t total_entries = simplejs_generic_sort_get_entry_count(main_low_ctx);                                                 \
        size_t remaining_entries = total_entries;                                                                                   \
                                                                                                                                    \
        size_t main_low_ctx_index = 0;                                                                                              \
        size_t main_high_ctx_index = remaining_entries - 1;                                                                         \
                                                                                                                                    \
        while (remaining_entries > 1 &&                                                                                             \
               simplejs_generic_sort_check_pointer(main_low_ctx))                                                                   \
        {                                                                                                                           \
            char *temp_buffer = &temp_context_heap[context_size * 2];                                                               \
            memcpy(temp_buffer, simplejs_generic_sort_get_temp_context(main_low_ctx), context_size);                                \
                                                                                                                                    \
            char *selected_low_buffer = &temp_context_heap[context_size * 3];                                                       \
            char *selected_high_buffer = &temp_context_heap[context_size * 4];                                                      \
                                                                                                                                    \
            simplejs_generic_sort_ctx_t *temp_sort_ctx = (void *)&temp_sort_heap[sort_size * 2];                                  \
            size_t temp_sort_ctx_index = main_low_ctx_index;                                                                        \
            simplejs_generic_sort_copy(temp_sort_ctx, main_low_ctx);                                                                \
            simplejs_generic_sort_set_temp_context(temp_sort_ctx, temp_buffer);                                                     \
                                                                                                                                    \
            type_t low_diff = type_min;                                                                                             \
            simplejs_generic_sort_ctx_t *selected_low_ctx = (void *)&temp_sort_heap[sort_size * 3];                               \
            size_t selected_low_ctx_index = temp_sort_ctx_index;                                                                    \
            simplejs_generic_sort_copy(selected_low_ctx, temp_sort_ctx);                                                            \
                                                                                                                                    \
            type_t high_diff = type_max;                                                                                            \
            simplejs_generic_sort_ctx_t *selected_high_ctx = (void *)&temp_sort_heap[sort_size * 4];                              \
            size_t selected_high_ctx_index = temp_sort_ctx_index;                                                                   \
            simplejs_generic_sort_copy(selected_high_ctx, temp_sort_ctx);                                                           \
                                                                                                                                    \
            size_t sub_iterations = 0;                                                                                              \
                                                                                                                                    \
            while (sub_iterations < remaining_entries && simplejs_generic_sort_check_pointer(temp_sort_ctx))                        \
            {                                                                                                                       \
                type_t current_low_diff = simplejs_generic_sort_diff_##type(main_low_ctx, temp_sort_ctx);                           \
                type_t current_high_diff = simplejs_generic_sort_diff_##type(main_high_ctx, temp_sort_ctx);                         \
                                                                                                                                    \
                if (current_low_diff > low_diff)                                                                                    \
                {                                                                                                                   \
                    low_diff = current_low_diff;                                                                                    \
                                                                                                                                    \
                    memcpy(selected_low_buffer, temp_buffer, context_size);                                                         \
                    simplejs_generic_sort_set_temp_context(selected_low_ctx, selected_low_buffer);                                  \
                    selected_low_ctx_index = temp_sort_ctx_index + sub_iterations;                                                  \
                }                                                                                                                   \
                                                                                                                                    \
                if (current_high_diff < high_diff)                                                                                  \
                {                                                                                                                   \
                    high_diff = current_high_diff;                                                                                  \
                                                                                                                                    \
                    memcpy(selected_high_buffer, temp_buffer, context_size);                                                        \
                    simplejs_generic_sort_set_temp_context(selected_high_ctx, selected_high_buffer);                                \
                    selected_high_ctx_index = temp_sort_ctx_index + sub_iterations;                                                 \
                }                                                                                                                   \
                                                                                                                                    \
                simplejs_generic_sort_next_pointer(temp_sort_ctx);                                                                  \
                iterations++, sub_iterations++;                                                                                     \
            }                                                                                                                       \
                                                                                                                                    \
            if (high_diff < type_epsilon &&                                                                                         \
                simplejs_generic_sort_get_temp_context(selected_high_ctx) != simplejs_generic_sort_get_temp_context(temp_sort_ctx)) \
            {                                                                                                                       \
                if (main_high_ctx_index == selected_low_ctx_index)                                                                  \
                    memcpy(selected_low_buffer, selected_high_buffer, context_size);                                                \
                                                                                                                                    \
                simplejs_generic_sort_swap_entries(main_high_ctx, selected_high_ctx);                                               \
            }                                                                                                                       \
                                                                                                                                    \
            if (!(selected_high_ctx_index == main_low_ctx_index &&                                                                  \
                  selected_low_ctx_index == main_high_ctx_index) &&                                                                 \
                low_diff > type_epsilon &&                                                                                          \
                simplejs_generic_sort_get_temp_context(selected_low_ctx) != simplejs_generic_sort_get_temp_context(temp_sort_ctx))  \
                simplejs_generic_sort_swap_entries(main_low_ctx, selected_low_ctx);                                                 \
                                                                                                                                    \
            simplejs_generic_sort_next_pointer(main_low_ctx);                                                                       \
            simplejs_generic_sort_prev_pointer(main_high_ctx);                                                                      \
            main_low_ctx_index++;                                                                                                   \
            main_high_ctx_index--;                                                                                                  \
            remaining_entries -= 2;                                                                                                 \
        }                                                                                                                           \
                                                                                                                                    \
        if (out_iterations)                                                                                                         \
            *out_iterations = iterations;                                                                                           \
                                                                                                                                    \
    result:                                                                                                                         \
        if (temp_heap)                                                                                                              \
            simplejs_hook_mfree(temp_heap);                                                                                         \
                                                                                                                                    \
        return status;                                                                                                              \
    }

_simplejs_selection_sort_impl(f32, float, -FLT_MAX, FLT_MAX, FLT_EPSILON);
_simplejs_selection_sort_half_impl(f32, float, -FLT_MAX, FLT_MAX, FLT_EPSILON);
