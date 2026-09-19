#include <object.hpp>

namespace simplejs
{
SIMPLEJS_CPP_API object::object()
{
    simplejs_object = nullptr;
    simplejs_object_value = 0;
}

SIMPLEJS_CPP_API object::object(simplejs_object_t *object, uint16_t object_value)
{
    simplejs_object = object;
    simplejs_object_value = object_value;

    simplejs_object_reference(NULL, simplejs_object);
}

SIMPLEJS_CPP_API object::~object()
{
    simplejs_object_dereference(NULL, simplejs_object);
}

void SIMPLEJS_CPP_API object::change_object(simplejs_object_t *object, uint16_t object_value)
{
    if (simplejs_object)
        simplejs_object_dereference(NULL, object);

    simplejs_object = object;
    simplejs_object_value = object_value;

    if (simplejs_object)
        simplejs_object_reference(NULL, simplejs_object);
}

void SIMPLEJS_CPP_API object::init_object_variable(simplejs_variable_t *out)
{
    simplejs_variable_init_object(out, simplejs_object, simplejs_object_value);
}
} // namespace simplejs
