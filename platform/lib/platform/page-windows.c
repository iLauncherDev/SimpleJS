#include <lib/page.h>
#include <windows.h>

size_t simplejs_platform_get_page_size()
{
    SYSTEM_INFO SystemInfo;
    GetSystemInfo(&SystemInfo);

    return (size_t)SystemInfo.dwPageSize;
}

pvoid simplejs_platform_page_alloc(pvoid ptr, size_t size, uint32_t allocation_flags, uint32_t protection_flags)
{
    DWORD mm_protection_flags = 0;
    if (protection_flags & SIMPLEJS_PAGE_PROTECT_WRITE_FLAG)
        mm_protection_flags |= PAGE_READWRITE;
    else if (protection_flags & SIMPLEJS_PAGE_PROTECT_READ_FLAG)
        mm_protection_flags |= PAGE_READONLY;

    DWORD mm_allocation_flags = 0;
    if (allocation_flags & SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG)
        mm_allocation_flags |= MEM_COMMIT;
    if (allocation_flags & SIMPLEJS_PAGE_ALLOC_RESERVE_FLAG)
        mm_allocation_flags |= MEM_RESERVE;
    if (allocation_flags & SIMPLEJS_PAGE_ALLOC_RESET_FLAG)
        mm_allocation_flags |= MEM_RESET;

    return VirtualAlloc(ptr, size, mm_allocation_flags, mm_protection_flags);
}

bool simplejs_platform_page_free(pvoid ptr, size_t size, uint32_t flags)
{
    bool result = false;

    if (flags & SIMPLEJS_PAGE_FREE_RELEASE_FLAG)
    {
        result = VirtualFree(ptr, 0, MEM_RELEASE);

        goto result;
    }

    if (flags & SIMPLEJS_PAGE_FREE_DECOMMIT_FLAG)
    {
        result = VirtualFree(ptr, size, MEM_DECOMMIT);

        goto result;
    }

result:
    return result;
}
