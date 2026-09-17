#include <default.h>
#include <lib/stdout.h>

static bool finished_initialization = false;

simplejs_status_t SIMPLEJS_PLATFORM_API simplejs_platform_init()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (finished_initialization)
        goto result;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_init_stdout(), result, status);

    finished_initialization = true;
result:
    return status;
}

void SIMPLEJS_PLATFORM_API simplejs_platform_uninit()
{
    return;
}
