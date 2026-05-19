#include <default.h>
#include <mm/gc.h>
#include <simplejs/lib/stdout_buffer.h>

void simplejs_initialize_composite_list();

simplejs_status_t SIMPLEJS_API simplejs_init()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_initialize_composite_list();

    status = simplejs_init_gc();
    if (!SIMPLEJS_SUCCESS(status))
        goto result;

    simplejs_platform_enable_stdout_buffer();

result:
    return status;
}

void SIMPLEJS_API simplejs_uninit()
{
    return;
}
