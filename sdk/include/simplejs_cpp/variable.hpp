#pragma once
#include "default.hpp"
#include "object.hpp"
#include <simplejs/variable.h>
#include <iostream>

#ifdef SIMPLEJS_VARIABLE_CPP
#define SIMPLEJS_VARIABLE_DECL_PREFIX
#else
#define SIMPLEJS_VARIABLE_DECL_PREFIX extern
#endif

#define SIMPLEJS_VARIABLE_TEMP_IMPL(value_type)                                                                                      \
    SIMPLEJS_VARIABLE_DECL_PREFIX template value_type SIMPLEJS_CPP_API variable::get_variable_value<value_type>(bool *);             \
    SIMPLEJS_VARIABLE_DECL_PREFIX template const value_type SIMPLEJS_CPP_API variable::get_variable_value<const value_type>(bool *); \
    SIMPLEJS_VARIABLE_DECL_PREFIX template value_type SIMPLEJS_CPP_API variable::get_variable_value<value_type>();                   \
    SIMPLEJS_VARIABLE_DECL_PREFIX template const value_type SIMPLEJS_CPP_API variable::get_variable_value<const value_type>();       \
    SIMPLEJS_VARIABLE_DECL_PREFIX template void SIMPLEJS_CPP_API variable::set_variable_value<value_type>(value_type);               \
    SIMPLEJS_VARIABLE_DECL_PREFIX template void SIMPLEJS_CPP_API variable::set_variable_value<const value_type>(const value_type)

namespace simplejs
{
class variable
{
  private:
    simplejs_variable_t simplejs_variable;

  public:
    variable(auto value)
    {
        simplejs_variable_init_undefined(&simplejs_variable);
        set_variable_value(value);
    }

    ~variable()
    {
        simplejs_variable_t tmp_out = {0};

        simplejs_variable_assign(&simplejs_variable, &tmp_out);
    }

    variable &operator=(const variable &value)
    {
        simplejs_variable_assign(&simplejs_variable, &value.simplejs_variable);

        return *this;
    }

    variable &operator=(auto value)
    {
        set_variable_value(value);

        return *this;
    }

    template <typename value_type>
    value_type SIMPLEJS_CPP_API get_variable_value(bool *result)
    {
        if (result)
            *result = true;

        value_type fallback_value = value_type{};

        if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), int8_t> ||
                      std::is_same_v<simplejs_striptype_strict(value_type), uint8_t> ||
                      std::is_same_v<simplejs_striptype_strict(value_type), int16_t> ||
                      std::is_same_v<simplejs_striptype_strict(value_type), uint16_t> ||
                      std::is_same_v<simplejs_striptype_strict(value_type), int32_t> ||
                      std::is_same_v<simplejs_striptype_strict(value_type), uint32_t>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
                goto error;

            return (value_type)simplejs_number_get_int32(&simplejs_variable.value.number);
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), int64_t> ||
                           std::is_same_v<simplejs_striptype_strict(value_type), uint64_t>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
                goto error;

            return (value_type)simplejs_number_get_int64(&simplejs_variable.value.number);
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), intptr_t> ||
                           std::is_same_v<simplejs_striptype_strict(value_type), uintptr_t>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
                goto error;

            return (value_type)simplejs_number_get_intptr(&simplejs_variable.value.number);
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), float>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
                goto error;

            return (value_type)simplejs_number_get_float32(&simplejs_variable.value.number);
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), double>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_NUMBER)
                goto error;

            return (value_type)simplejs_number_get_float64(&simplejs_variable.value.number);
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), char *> ||
                           std::is_same_v<simplejs_striptype_strict(value_type), const char *>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_FAST_STRING)
                goto error;

            return (value_type)simplejs_variable.value.fast_string;
        }
        else if constexpr (std::is_same_v<simplejs_striptype_strict(value_type), simplejs::object>)
        {
            if (simplejs_variable.type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
                goto error;

            return simplejs::object((simplejs_object_t *)simplejs_variable.value.object, simplejs_variable.value.object_value);
        }
        else
        {
            static_assert(false, "get_variable_value: unknown type");
        }

    error:
        if (result)
            *result = false;

        return fallback_value;
    }

    template <typename value_type>
    value_type SIMPLEJS_CPP_API get_variable_value()
    {
        return get_variable_value<value_type>(nullptr);
    }

    template <typename value_type>
    void SIMPLEJS_CPP_API set_variable_value(value_type value)
    {
        simplejs_variable_t tmp_out = {0};
        simplejs_number_t number = {0};

        if constexpr (std::is_same_v<simplejs_decltype_strict(value), int8_t> ||
                      std::is_same_v<simplejs_decltype_strict(value), uint8_t> ||
                      std::is_same_v<simplejs_decltype_strict(value), int16_t> ||
                      std::is_same_v<simplejs_decltype_strict(value), uint16_t> ||
                      std::is_same_v<simplejs_decltype_strict(value), int32_t> ||
                      std::is_same_v<simplejs_decltype_strict(value), uint32_t>)
        {
            bool is_signed = std::is_same_v<simplejs_decltype_strict(value), int8_t> ||
                             std::is_same_v<simplejs_decltype_strict(value), int16_t> ||
                             std::is_same_v<simplejs_decltype_strict(value), int32_t>;

            number.type = is_signed ? SIMPLEJS_NUMBER_TYPE_I32 : SIMPLEJS_NUMBER_TYPE_UI32;
            number.value.i32 = (int32_t)value;

            simplejs_variable_init_number(&tmp_out, &number);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_strict(value), int64_t> ||
                           std::is_same_v<simplejs_decltype_strict(value), uint64_t>)
        {
            bool is_signed = std::is_same_v<simplejs_decltype_strict(value), int64_t>;

            number.type = is_signed ? SIMPLEJS_NUMBER_TYPE_I64 : SIMPLEJS_NUMBER_TYPE_UI64;
            number.value.i32 = (int64_t)value;

            simplejs_variable_init_number(&tmp_out, &number);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_strict(value), intptr_t> ||
                           std::is_same_v<simplejs_decltype_strict(value), uintptr_t>)
        {
            bool is_signed = std::is_same_v<simplejs_decltype_strict(value), intptr_t>;

            number.type = is_signed ? SIMPLEJS_NUMBER_TYPE_IPTR : SIMPLEJS_NUMBER_TYPE_UIPTR;
            number.value.i32 = (intptr_t)value;

            simplejs_variable_init_number(&tmp_out, &number);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_strict(value), float>)
        {
            number.type = SIMPLEJS_NUMBER_TYPE_F32;
            number.value.f32 = (float)value;

            simplejs_variable_init_number(&tmp_out, &number);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_strict(value), double>)
        {
            number.type = SIMPLEJS_NUMBER_TYPE_F64;
            number.value.f64 = (double)value;

            simplejs_variable_init_number(&tmp_out, &number);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_strict(value), char *> ||
                           std::is_same_v<simplejs_decltype_strict(value), const char *>)
        {
            simplejs_variable_init_fast_string(&tmp_out, (const char *)value);
        }
        else if constexpr (std::is_same_v<simplejs_decltype_soft(value), simplejs::object>)
        {
            simplejs::object value_object = value;

            value_object.init_object_variable(&tmp_out);
        }
        else
        {
            static_assert(false, "set_variable_value: unknown type");
        }

        simplejs_variable_assign(&simplejs_variable, &tmp_out);
    }

    simplejs_variable_t *get_variable()
    {
        return &simplejs_variable;
    }
};

SIMPLEJS_VARIABLE_TEMP_IMPL(int8_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(uint8_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(int16_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(uint16_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(int32_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(uint32_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(int64_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(uint64_t);
// SIMPLEJS_VARIABLE_TEMP_IMPL(intptr_t);
// SIMPLEJS_VARIABLE_TEMP_IMPL(uintptr_t);
SIMPLEJS_VARIABLE_TEMP_IMPL(float);
SIMPLEJS_VARIABLE_TEMP_IMPL(double);
SIMPLEJS_VARIABLE_TEMP_IMPL(pchar);
SIMPLEJS_VARIABLE_TEMP_IMPL(simplejs::object);
} // namespace simplejs
