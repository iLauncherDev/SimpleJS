#include <lib/page.h>
#include <sys/mman.h>
#include <unistd.h>

size_t simplejs_platform_get_page_size()
{
    return (size_t)sysconf(_SC_PAGE_SIZE);
}

pvoid simplejs_platform_page_alloc(pvoid ptr, size_t size, uint32_t allocation_flags, uint32_t protection_flags)
{
    int mm_protection_flags = 0;
    if (protection_flags & SIMPLEJS_PAGE_PROTECT_READ_FLAG)
        mm_protection_flags |= PROT_READ;
    if (protection_flags & SIMPLEJS_PAGE_PROTECT_WRITE_FLAG)
        mm_protection_flags |= PROT_WRITE;

    int required_null_flags = SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG | SIMPLEJS_PAGE_ALLOC_RESET_FLAG;
    if ((~allocation_flags & required_null_flags) == required_null_flags)
        mm_protection_flags = PROT_NONE;

    int mm_map_flags = MAP_PRIVATE | MAP_ANONYMOUS;

    int required_populate_flags = SIMPLEJS_PAGE_ALLOC_RESERVE_FLAG | SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG;
    if ((allocation_flags & required_populate_flags) == required_populate_flags)
        mm_map_flags |= MAP_POPULATE;

    if (allocation_flags & SIMPLEJS_PAGE_ALLOC_RESET_FLAG)
    {
        if (mprotect(ptr, size, mm_protection_flags) != 0)
            return NULL;

        return madvise(ptr, size, MADV_DONTNEED) == 0 ? ptr : NULL;
    }

    return mmap(ptr, size, mm_protection_flags, mm_map_flags, -1, 0);
}

bool simplejs_platform_page_free(pvoid ptr, size_t size, uint32_t flags)
{
    if (flags & (SIMPLEJS_PAGE_FREE_DECOMMIT_FLAG | SIMPLEJS_PAGE_FREE_RELEASE_FLAG))
    {
        return munmap(ptr, size) == 0;
    }
}
