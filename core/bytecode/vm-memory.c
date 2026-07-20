#include <vm.h>
#include <simplejs/lib/page.h>

simplejs_status_t SIMPLEJS_API simplejs_create_vm_memory(size_t size, simplejs_vm_memory_t **out)
{
    size_t page_size = simplejs_get_page_size();
    size_t size_in_pages = (size + (page_size - 1)) / page_size;
    size_t size_in_bytes = size_in_pages * page_size;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_vm_memory_t *vm_memory = simplejs_hook_malloc(sizeof(*vm_memory));
    if (!vm_memory)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(vm_memory, sizeof(*vm_memory));

    vm_memory->memory_size_in_bytes = size_in_bytes;
    vm_memory->memory_size_in_pages = size_in_pages;

    simplejs_init_spinlock(&vm_memory->memory_pages_lock);

    vm_memory->memory_pages = simplejs_hook_malloc(sizeof(*vm_memory->memory_pages) * size_in_pages);
    if (!vm_memory->memory_pages)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }
    memclr(vm_memory->memory_pages, sizeof(*vm_memory->memory_pages) * size_in_pages);

    vm_memory->default_permissions = SIMPLEJS_PAGE_PROTECT_READ_FLAG | SIMPLEJS_PAGE_PROTECT_WRITE_FLAG;

    vm_memory->memory_data = simplejs_page_alloc(
        NULL,
        size_in_bytes,
        SIMPLEJS_PAGE_ALLOC_COMMIT_FLAG,
        vm_memory->default_permissions);
    if (!vm_memory->memory_data)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    *out = vm_memory;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (vm_memory)
        {
            if (vm_memory->memory_pages)
                simplejs_hook_mfree(vm_memory->memory_pages);

            if (vm_memory->memory_data)
                simplejs_page_free(vm_memory->memory_data, vm_memory->memory_size_in_bytes, SIMPLEJS_PAGE_FREE_RELEASE_FLAG);

            simplejs_hook_mfree(vm_memory);
        }
    }

    return status;
}

void SIMPLEJS_API simplejs_destroy_vm_memory(simplejs_vm_memory_t *vm_memory)
{
    SIMPLEJS_ASSERT(vm_memory != NULL);

    if (vm_memory->memory_pages)
        simplejs_hook_mfree(vm_memory->memory_pages);

    if (vm_memory->memory_data)
        simplejs_page_free(vm_memory->memory_data, vm_memory->memory_size_in_bytes, SIMPLEJS_PAGE_FREE_RELEASE_FLAG);

    simplejs_hook_mfree(vm_memory);
}

bool SIMPLEJS_API simplejs_vm_memory_find_alloc_start(simplejs_vm_memory_t *vm_memory, pvoid address, void **out, size_t *out_size)
{
    bool ret = false;

    size_t page_size = simplejs_get_page_size();

    SIMPLEJS_ASSERT(vm_memory != NULL);
    SIMPLEJS_ASSERT(out != NULL);
    SIMPLEJS_ASSERT(out_size != NULL);

    simplejs_spinlock_acquire(&vm_memory->memory_pages_lock, true);

    for (size_t i = 0; i < vm_memory->memory_size_in_pages; i++)
    {
        simplejs_vm_memory_page_t *page = &vm_memory->memory_pages[i];

        if ((page->flags & SIMPLEJS_VM_MEMORY_PAGE_ALLOCATED_FLAG) && page->allocated_pages != 0)
        {
            void *alloc_start = &((uint8_t *)vm_memory->memory_data)[i * page_size];
            void *alloc_end = &((uint8_t *)alloc_start)[(size_t)page->allocated_pages * page_size];

            if (address >= alloc_start &&
                address < alloc_end)
            {
                *out = alloc_start;
                *out_size = (uintptr_t)alloc_end - (uintptr_t)alloc_start;

                ret = true;
                goto result;
            }
        }
    }

result:
    simplejs_spinlock_release(&vm_memory->memory_pages_lock);

    return ret;
}

pvoid SIMPLEJS_API simplejs_vm_memory_alloc(simplejs_vm_memory_t *vm_memory, size_t size_in_bytes)
{
    void *ptr = NULL;
    bool found_memory = false;

    size_t page_size = simplejs_get_page_size();
    size_t size_in_pages = (size_in_bytes + (page_size - 1)) / page_size;

    SIMPLEJS_ASSERT(vm_memory != NULL);
    SIMPLEJS_ASSERT(size_in_pages != 0);

    size_t start_page = 0;
    size_t current_free_pages = 0;

    simplejs_spinlock_acquire(&vm_memory->memory_pages_lock, true);

    for (size_t i = 0; i < vm_memory->memory_size_in_pages; i++)
    {
        simplejs_vm_memory_page_t *page = &vm_memory->memory_pages[i];

        current_free_pages++;

        if (page->flags & SIMPLEJS_VM_MEMORY_PAGE_ALLOCATED_FLAG)
            current_free_pages = 0;

        if (current_free_pages >= size_in_pages)
        {
            start_page = i - (current_free_pages - 1);
            found_memory = true;
            goto result;
        }
    }

result:
    if (found_memory)
    {
        simplejs_vm_memory_page_t *alloc_page = &vm_memory->memory_pages[start_page];
        alloc_page->allocated_pages = size_in_pages;

        for (size_t i = 0; i < alloc_page->allocated_pages; i++)
        {
            simplejs_vm_memory_page_t *page = &alloc_page[i];

            page->flags = SIMPLEJS_VM_MEMORY_PAGE_ALLOCATED_FLAG;
        }

        ptr = &vm_memory->memory_data[start_page * page_size];
    }

    simplejs_spinlock_release(&vm_memory->memory_pages_lock);

    return ptr;
}

void SIMPLEJS_API simplejs_vm_memory_free(simplejs_vm_memory_t *vm_memory, void *ptr)
{
    size_t page_size = simplejs_get_page_size();

    SIMPLEJS_ASSERT(vm_memory != NULL);

    void *vm_memory_start = vm_memory->memory_data;
    void *vm_memory_end = vm_memory->memory_data + vm_memory->memory_size_in_bytes;

    SIMPLEJS_ASSERT(ptr >= vm_memory_start && ptr < vm_memory_end);

    uintptr_t relative_ptr = (uintptr_t)ptr - (uintptr_t)vm_memory->memory_data;
    size_t start_page = relative_ptr / page_size;

    simplejs_vm_memory_page_t *alloc_page = &vm_memory->memory_pages[start_page];
    size_t allocated_pages = alloc_page->allocated_pages;

    simplejs_spinlock_acquire(&vm_memory->memory_pages_lock, true);

    SIMPLEJS_ASSERT(alloc_page->allocated_pages != 0);
    SIMPLEJS_ASSERT(alloc_page->flags & SIMPLEJS_VM_MEMORY_PAGE_ALLOCATED_FLAG);

    for (size_t i = 0; i < allocated_pages; i++)
    {
        simplejs_vm_memory_page_t *page = &alloc_page[i];

        memclr(page, sizeof(*page));
    }

    uint8_t *start_ptr = &vm_memory->memory_data[start_page * page_size];
    uint8_t *end_ptr = &start_ptr[allocated_pages * page_size];
    uintptr_t ptr_size = (uintptr_t)end_ptr - (uintptr_t)start_ptr;

    SIMPLEJS_ASSERT(simplejs_page_alloc(start_ptr, ptr_size, SIMPLEJS_PAGE_ALLOC_RESET_FLAG, vm_memory->default_permissions) != NULL);

    simplejs_spinlock_release(&vm_memory->memory_pages_lock);
}
