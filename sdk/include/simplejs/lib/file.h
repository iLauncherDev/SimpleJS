#pragma once
#include "../default.h"

typedef struct simplejs_file simplejs_file_t, *psimplejs_file_t;

psimplejs_file_t SIMPLEJS_PLATFORM_API simplejs_file_open(const char *file_path, const char *modes);
int64_t SIMPLEJS_PLATFORM_API simplejs_file_tell(simplejs_file_t *file);
int SIMPLEJS_PLATFORM_API simplejs_file_seek(simplejs_file_t *file, int64_t offset, int whence);
uint32_t SIMPLEJS_PLATFORM_API simplejs_file_read(simplejs_file_t *file, void *buffer, uint32_t length);
uint32_t SIMPLEJS_PLATFORM_API simplejs_file_write(simplejs_file_t *file, void *buffer, uint32_t length);
void SIMPLEJS_PLATFORM_API simplejs_file_close(simplejs_file_t *file);
