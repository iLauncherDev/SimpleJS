#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _simplejs_selection_sort_define_name_callback_decl(name) \
    void SIMPLEJS_API simplejs_generic_sort_define_##name##_callback(simplejs_generic_sort_ctx_t *sort_ctx, simplejs_generic_sort_##name##_f callback)

#define _simplejs_selection_sort_diff_decl(type, type_t) \
    type_t SIMPLEJS_API simplejs_selection_sort_diff_##type(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx)

#define _simplejs_selection_sort_decl(type) \
    simplejs_status_t SIMPLEJS_API simplejs_selection_sort_##type(simplejs_generic_sort_ctx_t *sort_ctx, uint64_t *out_iterations)

#define _simplejs_selection_sort_half_decl(type) \
    simplejs_status_t SIMPLEJS_API simplejs_selection_sort_half_##type(simplejs_generic_sort_ctx_t *sort_ctx, uint64_t *out_iterations)

typedef struct simplejs_generic_sort_ctx simplejs_generic_sort_ctx_t;

typedef size_t (*simplejs_generic_sort_get_entry_count_f)(simplejs_generic_sort_ctx_t *sort_ctx);
typedef void (*simplejs_generic_sort_goto_pointer_f)(simplejs_generic_sort_ctx_t *sort_ctx, bool direction);
typedef bool (*simplejs_generic_sort_prev_pointer_f)(simplejs_generic_sort_ctx_t *sort_ctx);
typedef bool (*simplejs_generic_sort_next_pointer_f)(simplejs_generic_sort_ctx_t *sort_ctx);
typedef float (*simplejs_generic_sort_diff_f32_f)(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx);
typedef void (*simplejs_generic_sort_swap_entries_f)(simplejs_generic_sort_ctx_t *sort_dest_ctx, simplejs_generic_sort_ctx_t *sort_src_ctx);

struct simplejs_generic_sort_ctx
{
    const void *context;
    size_t context_size;

    simplejs_generic_sort_get_entry_count_f f_get_entry_count;
    simplejs_generic_sort_goto_pointer_f f_goto_pointer;
    simplejs_generic_sort_prev_pointer_f f_prev_pointer;
    simplejs_generic_sort_next_pointer_f f_next_pointer;
    simplejs_generic_sort_diff_f32_f f_diff_f32;
    simplejs_generic_sort_swap_entries_f f_swap_entries;

    void *_temp_context;
};

void SIMPLEJS_API simplejs_init_generic_sort(simplejs_generic_sort_ctx_t *sort_ctx, const void *context, size_t context_size);

_simplejs_selection_sort_define_name_callback_decl(get_entry_count);
_simplejs_selection_sort_define_name_callback_decl(goto_pointer);
_simplejs_selection_sort_define_name_callback_decl(prev_pointer);
_simplejs_selection_sort_define_name_callback_decl(next_pointer);
_simplejs_selection_sort_define_name_callback_decl(diff_f32);
_simplejs_selection_sort_define_name_callback_decl(swap_entries);

size_t SIMPLEJS_API simplejs_selection_sort_get_entry_count(simplejs_generic_sort_ctx_t *sort_ctx);
void SIMPLEJS_API simplejs_selection_sort_goto_pointer(simplejs_generic_sort_ctx_t *sort_ctx, bool direction);
bool SIMPLEJS_API simplejs_selection_sort_prev_pointer(simplejs_generic_sort_ctx_t *sort_ctx);
bool SIMPLEJS_API simplejs_selection_sort_next_pointer(simplejs_generic_sort_ctx_t *sort_ctx);
_simplejs_selection_sort_diff_decl(f32, float);
void SIMPLEJS_API simplejs_selection_sort_swap_entries(simplejs_generic_sort_ctx_t *sort_a_ctx, simplejs_generic_sort_ctx_t *sort_b_ctx);

_simplejs_selection_sort_decl(f32);
_simplejs_selection_sort_half_decl(f32);

#ifdef __cplusplus
}
#endif
