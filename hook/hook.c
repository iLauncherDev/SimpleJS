#include <hook.h>

int memoryAmount = 0;

int SIMPLEJS_HOOK_API get_memory_amount()
{
    return memoryAmount;
}

pvoid SIMPLEJS_HOOK_API simplejs_hook_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr)
        memoryAmount++;

    return ptr;
}

void SIMPLEJS_HOOK_API simplejs_hook_mfree(void *ptr)
{
    if (ptr)
        memoryAmount--;

    free(ptr);
}
