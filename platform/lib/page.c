#include <lib/page.h>

size_t SIMPLEJS_PLATFORM_API simplejs_get_page_size()
{
    return simplejs_platform_get_page_size();
}

pvoid SIMPLEJS_PLATFORM_API simplejs_page_alloc(pvoid ptr, size_t size, uint32_t allocation_flags, uint32_t protection_flags)
{
    return simplejs_platform_page_alloc(ptr, size, allocation_flags, protection_flags);
}

bool SIMPLEJS_PLATFORM_API simplejs_page_free(pvoid ptr, size_t size, uint32_t flags)
{
    simplejs_platform_page_free(ptr, size, flags);
}
