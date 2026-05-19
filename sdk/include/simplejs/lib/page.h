#pragma once
#include "../default.h"

#define SIMPLEJS_PAGE_ALLOC_RESERVE_FLAG (1 << 0)
#define SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG (1 << 1)
#define SIMPLEJS_PAGE_ALLOC_RESET_FLAG (1 << 2)

#define SIMPLEJS_PAGE_PROTECT_READ_FLAG (1 << 0)
#define SIMPLEJS_PAGE_PROTECT_WRITE_FLAG (1 << 1)

#define SIMPLEJS_PAGE_FREE_DECOMMIT_FLAG (1 << 0)
#define SIMPLEJS_PAGE_FREE_RELEASE_FLAG (1 << 1)

size_t SIMPLEJS_PLATFORM_API simplejs_get_page_size();
pvoid SIMPLEJS_PLATFORM_API simplejs_page_alloc(pvoid ptr, size_t size, uint32_t allocation_flags, uint32_t protection_flags);
bool SIMPLEJS_PLATFORM_API simplejs_page_free(pvoid ptr, size_t size, uint32_t free_flags);
