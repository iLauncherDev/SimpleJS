#include <lib/shared_lib.h>

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_load_shared_lib(char *name, simplejs_shared_lib_t **out)
{
    return simplejs_platform_load_shared_lib(name, out);
}

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_shared_lib_get_proc_address(simplejs_shared_lib_t *shared_lib, char *name, void **out)
{
    return simplejs_platform_shared_lib_get_proc_address(shared_lib, name, out);
}

void SIMPLEJS_PLATFORM_API simplejs_close_shared_lib(simplejs_shared_lib_t *shared_lib)
{
    simplejs_platform_close_shared_lib(shared_lib);
}
