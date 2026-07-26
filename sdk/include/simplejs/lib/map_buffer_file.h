#pragma once
#include "map_buffer.h"

#define SIMPLEJS_MAP_BUFFER_FILE_FLAG_READ (1 << 0)
#define SIMPLEJS_MAP_BUFFER_FILE_FLAG_WRITE (1 << 1)

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_create_map_buffer_file(simplejs_map_buffer_t **out, char *file_path, uint32_t flags);
