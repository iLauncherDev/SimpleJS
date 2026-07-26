#pragma once
#include "default.h"
#include <simplejs/lib/file.h>

int64_t simplejs_platform_file_tell(simplejs_file_t *file);
int simplejs_platform_file_seek(simplejs_file_t *file, int64_t offset, int whence);
