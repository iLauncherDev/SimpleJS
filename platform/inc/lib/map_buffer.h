#pragma once
#include "default.h"
#include <simplejs/lib/map_buffer.h>

struct simplejs_map_buffer
{
    void *context;

    simplejs_map_buffer_read_f read_callback;
    simplejs_map_buffer_write_f write_callback;
    simplejs_map_buffer_get_size_f get_size_callback;
    simplejs_map_buffer_close_f close_callback;
};
