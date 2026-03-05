#include <default.h>
#include <mm/gc.h>

void simplejs_initialize_composite_list();

simplejs_status_t SIMPLEJS_API simplejs_init()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_initialize_composite_list();

    status = simplejs_init_gc();
    if (!SIMPLEJS_SUCCESS(status))
        goto result;

result:
    return status;
}
