#include <lib/shared_lib.h>
#include <windows.h>

simplejs_status_t simplejs_platform_load_shared_lib(char *name, simplejs_shared_lib_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    HMODULE ret = LoadLibraryA(name);
    if (!ret)
    {
        status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
        goto result;
    }

    *out = (simplejs_shared_lib_t *)ret;

result:
    return status;
}

simplejs_status_t simplejs_platform_shared_lib_get_proc_address(simplejs_shared_lib_t *shared_lib, char *name, void **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    void *ret = (void *)GetProcAddress((HMODULE)shared_lib, name);
    if (!ret)
    {
        status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
        goto result;
    }

    *out = ret;

result:
    return status;
}

void simplejs_platform_close_shared_lib(simplejs_shared_lib_t *shared_lib)
{
    FreeLibrary((HMODULE)shared_lib);
}
