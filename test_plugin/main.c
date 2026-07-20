#include <default.h>

simplejs_status_t test_plugin_function(simplejs_function_header_t *function_header)
{
    char tempBuffer[4096];
    tempBuffer[sizeof(tempBuffer) - 1] = '\0';

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_printf("hello from test_plugin_function! (%u args)\n", function_header->argument_count);

    for (uint32_t i = 0; i < function_header->argument_count; i++)
    {
        char *out;
        simplejs_variable_to_string(&function_header->arguments[i], tempBuffer, sizeof(tempBuffer), &out);

        simplejs_printf("argument %u from test_plugin_function! (%s value)\n", i, out);
    }

    return status;
}

simplejs_status_t SIMPLEJS_PLUGIN_API plugin_main(simplejs_object_t *global_object)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t function_variable;
    function_variable.type = SIMPLEJS_VARIABLE_TYPE_FUNCTION;

    simplejs_function_t *function = &function_variable.value.function;
    function->type = SIMPLEJS_FUNCTION_TYPE_PROXY;
    function->value.proxy = test_plugin_function;

    simplejs_variable_t property_variable;
    property_variable.type = SIMPLEJS_VARIABLE_TYPE_FAST_STRING;
    property_variable.value.fast_string = "test_plugin_function";

    status = simplejs_object_set_property_value(global_object, &property_variable, &function_variable);

    simplejs_printf("hello from plugin_main!\n");

    return status;
}
