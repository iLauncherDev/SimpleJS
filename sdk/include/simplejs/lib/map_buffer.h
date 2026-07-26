#pragma once
#include "../default.h"

typedef struct simplejs_map_buffer simplejs_map_buffer_t;

typedef size_t (*simplejs_map_buffer_read_f)(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length);
typedef size_t (*simplejs_map_buffer_write_f)(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length);
typedef uint64_t (*simplejs_map_buffer_get_size_f)(simplejs_map_buffer_t *map_buffer);
typedef void (*simplejs_map_buffer_close_f)(simplejs_map_buffer_t *map_buffer);

pvoid SIMPLEJS_PLATFORM_API simplejs_get_map_buffer_context(simplejs_map_buffer_t *map_buffer);

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_alloc_map_buffer(
    simplejs_map_buffer_t **out, void *context,
    simplejs_map_buffer_read_f read_callback,
    simplejs_map_buffer_write_f write_callback,
    simplejs_map_buffer_get_size_f get_size_callback,
    simplejs_map_buffer_close_f close_callback);

size_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_read(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length);
size_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_write(simplejs_map_buffer_t *map_buffer, void *buffer, uint64_t offset, size_t length);
uint64_t SIMPLEJS_PLATFORM_API simplejs_map_buffer_get_size(simplejs_map_buffer_t *map_buffer);
void SIMPLEJS_PLATFORM_API simplejs_destroy_map_buffer(simplejs_map_buffer_t *map_buffer);
