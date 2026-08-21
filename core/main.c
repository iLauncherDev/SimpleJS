#include <default.h>
#include <mm/gc.h>
#include <simplejs/lib/stdout_buffer.h>
#include <builtin_object/dynamic_object.h>

void simplejs_initialize_composite_list();

simplejs_status_t SIMPLEJS_API simplejs_init()
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_initialize_composite_list();

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_init_gc(), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_builtin_init_dynamic_object(), result, status);

    simplejs_platform_enable_stdout_buffer();

result:
    return status;
}

void SIMPLEJS_API simplejs_uninit()
{
    simplejs_builtin_uninit_dynamic_object();
}
