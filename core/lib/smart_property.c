#include <lib/smart_property.h>

bool SIMPLEJS_API simplejs_smart_property_reference(simplejs_object_t *object, simplejs_proxy_property_t *property)
{
    return simplejs_variable_reference_ex(object, &property->value);
}

bool SIMPLEJS_API simplejs_smart_property_dereference(simplejs_object_t *object, simplejs_proxy_property_t *property)
{
    return simplejs_variable_dereference_ex(object, &property->value);
}

bool SIMPLEJS_API simplejs_smart_property_get(simplejs_object_t *object, simplejs_proxy_property_t *property, simplejs_variable_t *out)
{
    return simplejs_variable_assign(out, &property->value);
}

bool SIMPLEJS_API simplejs_smart_property_set(simplejs_object_t *object, simplejs_proxy_property_t *property, simplejs_variable_t *in)
{
    return simplejs_variable_assign_ex(object, &property->value, in);
}
