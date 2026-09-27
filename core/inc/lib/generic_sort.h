#pragma once
#include "../default.h"
#include <simplejs/lib/generic_sort.h>

struct simplejs_generic_sort_ctx
{
    const_pvoid context;
    size_t context_size;
    pvoid temp_context;

    simplejs_generic_sort_get_entry_count_f f_get_entry_count;
    simplejs_generic_sort_goto_pointer_f f_goto_pointer;
    simplejs_generic_sort_check_pointer_f f_check_pointer;
    simplejs_generic_sort_prev_pointer_f f_prev_pointer;
    simplejs_generic_sort_next_pointer_f f_next_pointer;
    simplejs_generic_sort_diff_f32_f f_diff_f32;
    simplejs_generic_sort_swap_entries_f f_swap_entries;
};
