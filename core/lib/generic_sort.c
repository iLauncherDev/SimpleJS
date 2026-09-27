#include <lib/generic_sort.h>

#define _simplejs_selection_sort_define_name_callback_impl(name) \
    _simplejs_selection_sort_define_name_callback_decl(name)     \
    {                                                            \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                       \
                                                                 \
        sort_ctx->f_##name = callback;                           \
    }

#define _simplejs_selection_sort_diff_impl(type, type_t)                         \
    _simplejs_selection_sort_diff_decl(type, type_t)                             \
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

void SIMPLEJS_API simplejs_init_generic_sort(simplejs_generic_sort_ctx_t *sort_ctx, const void *context, size_t context_size)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);

    memclr(sort_ctx, sizeof(*sort_ctx));

    sort_ctx->context = context;
    sort_ctx->context_size = context_size;
}

_simplejs_selection_sort_define_name_callback_impl(get_entry_count);
_simplejs_selection_sort_define_name_callback_impl(goto_pointer);
_simplejs_selection_sort_define_name_callback_impl(prev_pointer);
_simplejs_selection_sort_define_name_callback_impl(next_pointer);
_simplejs_selection_sort_define_name_callback_impl(diff_f32);
_simplejs_selection_sort_define_name_callback_impl(swap_entries);

size_t SIMPLEJS_API simplejs_selection_sort_get_entry_count(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_get_entry_count != NULL);

    return sort_ctx->f_get_entry_count(sort_ctx);
}

void SIMPLEJS_API simplejs_selection_sort_goto_pointer(simplejs_generic_sort_ctx_t *sort_ctx, bool direction)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_goto_pointer != NULL);

    sort_ctx->f_goto_pointer(sort_ctx, direction);
}

bool SIMPLEJS_API simplejs_selection_sort_prev_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_prev_pointer != NULL);

    return sort_ctx->f_prev_pointer(sort_ctx);
}

bool SIMPLEJS_API simplejs_selection_sort_next_pointer(simplejs_generic_sort_ctx_t *sort_ctx)
{
    SIMPLEJS_ASSERT(sort_ctx != NULL);
    SIMPLEJS_ASSERT(sort_ctx->f_next_pointer != NULL);

    return sort_ctx->f_next_pointer(sort_ctx);
}

_simplejs_selection_sort_diff_impl(f32, float);

void SIMPLEJS_API simplejs_selection_sort_swap_entries(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx)
{
    SIMPLEJS_ASSERT(sort_a_ctx != NULL);
    SIMPLEJS_ASSERT(sort_b_ctx != NULL);

    SIMPLEJS_ASSERT(sort_a_ctx->f_swap_entries != NULL);
    SIMPLEJS_ASSERT(sort_b_ctx->f_swap_entries != NULL);

    SIMPLEJS_ASSERT(sort_a_ctx->f_swap_entries == sort_b_ctx->f_swap_entries);

    sort_a_ctx->f_swap_entries(sort_a_ctx, sort_b_ctx);
}

#define _simplejs_selection_sort_impl(type, type_t, type_min, type_max, type_epsilon)                     \
    _simplejs_selection_sort_decl(f32)                                                                    \
    {                                                                                                     \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                                                                \
                                                                                                          \
        uint64_t iterations = 0;                                                                          \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                               \
                                                                                                          \
        size_t temp_heap_size = sort_ctx->context_size * 3;                                               \
        char *temp_heap = simplejs_hook_malloc(temp_heap_size);                                           \
        if (!temp_heap)                                                                                   \
        {                                                                                                 \
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;                                                    \
            goto result;                                                                                  \
        }                                                                                                 \
                                                                                                          \
        char *main_low_buffer = &temp_heap[sort_ctx->context_size * 0];                                   \
        memclr(main_low_buffer, sort_ctx->context_size);                                                  \
                                                                                                          \
        simplejs_generic_sort_ctx_t main_low_ctx = *sort_ctx;                                             \
        main_low_ctx._temp_context = main_low_buffer;                                                     \
                                                                                                          \
        while (simplejs_selection_sort_next_pointer(&main_low_ctx))                                       \
        {                                                                                                 \
            char *temp_buffer = &temp_heap[sort_ctx->context_size * 1];                                   \
            memcpy(temp_buffer, main_low_ctx._temp_context, sort_ctx->context_size);                      \
                                                                                                          \
            char *selected_low_buffer = &temp_heap[sort_ctx->context_size * 2];                           \
            memcpy(selected_low_buffer, temp_buffer, sort_ctx->context_size);                             \
                                                                                                          \
            simplejs_generic_sort_ctx_t temp_sort_ctx = main_low_ctx;                                     \
            temp_sort_ctx._temp_context = temp_buffer;                                                    \
                                                                                                          \
            type_t low_diff = type_min;                                                                   \
            simplejs_generic_sort_ctx_t selected_low_ctx = temp_sort_ctx;                                 \
                                                                                                          \
            do                                                                                            \
            {                                                                                             \
                type_t current_diff = simplejs_selection_sort_diff_##type(&main_low_ctx, &temp_sort_ctx); \
                                                                                                          \
                if (current_diff > low_diff)                                                              \
                {                                                                                         \
                    low_diff = current_diff;                                                              \
                                                                                                          \
                    memcpy(selected_low_buffer, temp_buffer, sort_ctx->context_size);                     \
                    selected_low_ctx._temp_context = selected_low_buffer;                                 \
                }                                                                                         \
                                                                                                          \
                iterations++;                                                                             \
            } while (simplejs_selection_sort_next_pointer(&temp_sort_ctx));                               \
                                                                                                          \
            if (low_diff > type_epsilon &&                                                                \
                selected_low_ctx._temp_context != temp_sort_ctx._temp_context)                            \
                simplejs_selection_sort_swap_entries(&main_low_ctx, &selected_low_ctx);                   \
        }                                                                                                 \
                                                                                                          \
        if (out_iterations)                                                                               \
            *out_iterations = iterations;                                                                 \
                                                                                                          \
    result:                                                                                               \
        if (temp_heap)                                                                                    \
            simplejs_hook_mfree(temp_heap);                                                               \
                                                                                                          \
        return status;                                                                                    \
    }

#define _simplejs_selection_sort_half_impl(type, type_t, type_min, type_max, type_epsilon)                        \
    _simplejs_selection_sort_half_decl(f32)                                                                       \
    {                                                                                                             \
        SIMPLEJS_ASSERT(sort_ctx != NULL);                                                                        \
                                                                                                                  \
        uint64_t iterations = 0;                                                                                  \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                       \
                                                                                                                  \
        size_t temp_heap_size = sort_ctx->context_size * 5;                                                       \
        char *temp_heap = simplejs_hook_malloc(temp_heap_size);                                                   \
        if (!temp_heap)                                                                                           \
        {                                                                                                         \
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;                                                            \
            goto result;                                                                                          \
        }                                                                                                         \
                                                                                                                  \
        char *main_low_buffer = &temp_heap[sort_ctx->context_size * 0];                                           \
        memclr(main_low_buffer, sort_ctx->context_size);                                                          \
                                                                                                                  \
        char *main_high_buffer = &temp_heap[sort_ctx->context_size * 1];                                          \
        memclr(main_high_buffer, sort_ctx->context_size);                                                         \
                                                                                                                  \
        simplejs_generic_sort_ctx_t main_low_ctx = *sort_ctx;                                                     \
        main_low_ctx._temp_context = main_low_buffer;                                                             \
                                                                                                                  \
        simplejs_generic_sort_ctx_t main_high_ctx = *sort_ctx;                                                    \
        main_high_ctx._temp_context = main_high_buffer;                                                           \
        simplejs_selection_sort_goto_pointer(&main_high_ctx, true);                                               \
                                                                                                                  \
        size_t total_entries = simplejs_selection_sort_get_entry_count(&main_low_ctx);                            \
        size_t remaining_entries = total_entries;                                                                 \
                                                                                                                  \
        size_t main_low_ctx_index = 0;                                                                            \
        size_t main_high_ctx_index = remaining_entries - 1;                                                       \
                                                                                                                  \
        while (remaining_entries > 1 &&                                                                           \
               simplejs_selection_sort_next_pointer(&main_low_ctx))                                               \
        {                                                                                                         \
            char *temp_buffer = &temp_heap[sort_ctx->context_size * 2];                                           \
            memcpy(temp_buffer, main_low_ctx._temp_context, sort_ctx->context_size);                              \
                                                                                                                  \
            char *selected_low_buffer = &temp_heap[sort_ctx->context_size * 3];                                   \
            memcpy(selected_low_buffer, temp_buffer, sort_ctx->context_size);                                     \
                                                                                                                  \
            char *selected_high_buffer = &temp_heap[sort_ctx->context_size * 4];                                  \
            memcpy(selected_high_buffer, temp_buffer, sort_ctx->context_size);                                    \
                                                                                                                  \
            simplejs_generic_sort_ctx_t temp_sort_ctx = main_low_ctx;                                             \
            size_t temp_sort_ctx_index = main_low_ctx_index;                                                      \
            temp_sort_ctx._temp_context = temp_buffer;                                                            \
                                                                                                                  \
            type_t low_diff = type_min;                                                                           \
            simplejs_generic_sort_ctx_t selected_low_ctx = temp_sort_ctx;                                         \
            size_t selected_low_ctx_index = temp_sort_ctx_index;                                                  \
                                                                                                                  \
            type_t high_diff = type_max;                                                                          \
            simplejs_generic_sort_ctx_t selected_high_ctx = temp_sort_ctx;                                        \
            size_t selected_high_ctx_index = temp_sort_ctx_index;                                                 \
                                                                                                                  \
            size_t sub_iterations = 0;                                                                            \
                                                                                                                  \
            do                                                                                                    \
            {                                                                                                     \
                type_t current_low_diff = simplejs_selection_sort_diff_##type(&main_low_ctx, &temp_sort_ctx);     \
                type_t current_high_diff = simplejs_selection_sort_diff_##type(&main_high_ctx, &temp_sort_ctx);   \
                                                                                                                  \
                if (current_low_diff > low_diff)                                                                  \
                {                                                                                                 \
                    low_diff = current_low_diff;                                                                  \
                                                                                                                  \
                    memcpy(selected_low_buffer, temp_buffer, sort_ctx->context_size);                             \
                    selected_low_ctx._temp_context = selected_low_buffer;                                         \
                    selected_low_ctx_index = temp_sort_ctx_index + sub_iterations;                                \
                }                                                                                                 \
                                                                                                                  \
                if (current_high_diff < high_diff)                                                                \
                {                                                                                                 \
                    high_diff = current_high_diff;                                                                \
                                                                                                                  \
                    memcpy(selected_high_buffer, temp_buffer, sort_ctx->context_size);                            \
                    selected_high_ctx._temp_context = selected_high_buffer;                                       \
                    selected_high_ctx_index = temp_sort_ctx_index + sub_iterations;                               \
                }                                                                                                 \
                                                                                                                  \
                iterations++, sub_iterations++;                                                                   \
            } while (sub_iterations < remaining_entries && simplejs_selection_sort_next_pointer(&temp_sort_ctx)); \
                                                                                                                  \
            if (high_diff < type_epsilon &&                                                                       \
                selected_high_ctx._temp_context != temp_sort_ctx._temp_context)                                   \
            {                                                                                                     \
                if (main_high_ctx_index == selected_low_ctx_index)                                                \
                    memcpy(selected_low_buffer, selected_high_buffer, sort_ctx->context_size);                    \
                                                                                                                  \
                simplejs_selection_sort_swap_entries(&main_high_ctx, &selected_high_ctx);                         \
            }                                                                                                     \
                                                                                                                  \
            if (!(selected_high_ctx_index == main_low_ctx_index &&                                                \
                  selected_low_ctx_index == main_high_ctx_index) &&                                               \
                low_diff > type_epsilon &&                                                                        \
                selected_low_ctx._temp_context != temp_sort_ctx._temp_context)                                    \
                simplejs_selection_sort_swap_entries(&main_low_ctx, &selected_low_ctx);                           \
                                                                                                                  \
            simplejs_selection_sort_prev_pointer(&main_high_ctx);                                                 \
            main_low_ctx_index++;                                                                                 \
            main_high_ctx_index--;                                                                                \
            remaining_entries -= 2;                                                                               \
        }                                                                                                         \
                                                                                                                  \
        if (out_iterations)                                                                                       \
            *out_iterations = iterations;                                                                         \
                                                                                                                  \
    result:                                                                                                       \
        if (temp_heap)                                                                                            \
            simplejs_hook_mfree(temp_heap);                                                                       \
                                                                                                                  \
        return status;                                                                                            \
    }

_simplejs_selection_sort_impl(f32, float, -FLT_MAX, FLT_MAX, FLT_EPSILON);
_simplejs_selection_sort_half_impl(f32, float, -FLT_MAX, FLT_MAX, FLT_EPSILON);
