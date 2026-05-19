#pragma once
#include "default.h"
#include <simplejs/lib/page.h>

size_t simplejs_platform_get_page_size();
pvoid simplejs_platform_page_alloc(pvoid ptr, size_t size, uint32_t allocation_flags, uint32_t protection_flags);
bool simplejs_platform_page_free(pvoid ptr, size_t size, uint32_t flags);
