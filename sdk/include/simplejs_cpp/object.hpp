#pragma once
#include "default.hpp"
#include <simplejs/object.h>

namespace simplejs
{
class object
{
  private:
    simplejs_object_t *simplejs_object;
    uint16_t simplejs_object_value;

  public:
    SIMPLEJS_CPP_API object();
    SIMPLEJS_CPP_API object(simplejs_object_t *object, uint16_t object_value);
    SIMPLEJS_CPP_API ~object();

    void SIMPLEJS_CPP_API change_object(simplejs_object_t *object, uint16_t object_value);
    void SIMPLEJS_CPP_API init_object_variable(simplejs_variable_t *out);
};
} // namespace simplejs
