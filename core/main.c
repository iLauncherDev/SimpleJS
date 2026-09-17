#include <default.h>
#include <mm/gc.h>
#include <builtin_object/dynamic_object.h>

void simplejs_initialize_composite_list();

simplejs_status_t SIMPLEJS_API simplejs_init()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_platform_init(), result, status);

    simplejs_initialize_composite_list();

    status = simplejs_init_gc();
    if (!SIMPLEJS_SUCCESS(status))
        goto result;

result:
    return status;
}

void SIMPLEJS_API simplejs_uninit()
{
    return;
}
