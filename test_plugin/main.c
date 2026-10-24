#include <default.h>
#include <simplejs/builtin_object/dynamic_string.h>

simplejs_status_t test_plugin_function(simplejs_function_header_t *function_header)
{
    char tempBuffer[4096];
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_printf("hello from test_plugin_function! (%u args)\n", function_header->argument_count);

    for (uint32_t i = 0; i < function_header->argument_count; i++)
    {
        const char *out;
        simplejs_variable_to_string(&function_header->arguments[i], tempBuffer, sizeof(tempBuffer), &out, SIMPLEJS_PROXY_GET_STRING_MODE_OPEN);

        simplejs_printf("argument %u from test_plugin_function! (%s value)\n", i, out);
        simplejs_variable_to_string(&function_header->arguments[i], tempBuffer, sizeof(tempBuffer), &out, SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE);
    }

    return status;
}

simplejs_status_t print(simplejs_function_header_t *function_header)
{
    char tempBuffer[4096];
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    for (uint32_t i = 0; i < function_header->argument_count; i++)
    {
        const char *out;
        simplejs_variable_to_string(&function_header->arguments[i], tempBuffer, sizeof(tempBuffer), &out, SIMPLEJS_PROXY_GET_STRING_MODE_OPEN);
        simplejs_printf("%s", out);
        simplejs_variable_to_string(&function_header->arguments[i], tempBuffer, sizeof(tempBuffer), &out, SIMPLEJS_PROXY_GET_STRING_MODE_CLOSE);
    }

    return status;
}

simplejs_status_t create_dynamic_string(simplejs_function_header_t *function_header)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_object_t *dynamic_string_object = NULL;
    simplejs_create_builtin_dynamic_string(&dynamic_string_object);

    simplejs_variable_t tmp_var;
    simplejs_variable_init_object(&tmp_var, dynamic_string_object, 0);

    if (dynamic_string_object)
        simplejs_gc_add_object(dynamic_string_object);

    simplejs_variable_assign(function_header->return_variable, &tmp_var);
    return status;
}

int SIMPLEJS_CPP_API stub();

simplejs_status_t SIMPLEJS_PLUGIN_API plugin_main(simplejs_object_t *global_object)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t function_variable;
    function_variable.type = SIMPLEJS_VARIABLE_TYPE_FUNCTION;

    simplejs_function_t *function = &function_variable.value.function;
    function->type = SIMPLEJS_FUNCTION_TYPE_PROXY;

    simplejs_variable_t property_variable;
    property_variable.type = SIMPLEJS_VARIABLE_TYPE_FAST_STRING;

    function->value.proxy = test_plugin_function;
    property_variable.value.fast_string = "test_plugin_function";

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_object_set_property_value(global_object, 0, &property_variable, &function_variable), result, status);

    function->value.proxy = print;
    property_variable.value.fast_string = "print";

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_object_set_property_value(global_object, 0, &property_variable, &function_variable), result, status);

    function->value.proxy = create_dynamic_string;
    property_variable.value.fast_string = "create_dynamic_string";

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_object_set_property_value(global_object, 0, &property_variable, &function_variable), result, status);

    simplejs_printf("hello from plugin_main!\n");

    stub();

result:
    return status;
}
