#include <number.h>

#define GEN_NUMBER_FUNC_TABLE(type, sign_type, name)                                                         \
    type simplejs_number_get_##name##_undef(simplejs_number_t *number)                                       \
    {                                                                                                        \
        return (type)0;                                                                                      \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_boolean(simplejs_number_t *number)                                     \
    {                                                                                                        \
        return (type)number->value.boolean;                                                                  \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_iptr(simplejs_number_t *number)                                        \
    {                                                                                                        \
        return (sign_type)number->value.iptr;                                                                \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_i32(simplejs_number_t *number)                                         \
    {                                                                                                        \
        return (sign_type)number->value.i32;                                                                 \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_i64(simplejs_number_t *number)                                         \
    {                                                                                                        \
        return (sign_type)number->value.i64;                                                                 \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_uiptr(simplejs_number_t *number)                                       \
    {                                                                                                        \
        return (type)number->value.uiptr;                                                                    \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_ui32(simplejs_number_t *number)                                        \
    {                                                                                                        \
        return (type)number->value.ui32;                                                                     \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_ui64(simplejs_number_t *number)                                        \
    {                                                                                                        \
        return (type)number->value.ui64;                                                                     \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_f32(simplejs_number_t *number)                                         \
    {                                                                                                        \
        return (sign_type)number->value.f32;                                                                 \
    }                                                                                                        \
                                                                                                             \
    type simplejs_number_get_##name##_f64(simplejs_number_t *number)                                         \
    {                                                                                                        \
        return (sign_type)number->value.f64;                                                                 \
    }                                                                                                        \
                                                                                                             \
    type (*simplejs_number_get_##name##_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t * number) = { \
        [SIMPLEJS_NUMBER_TYPE_BOOLEAN] = simplejs_number_get_##name##_boolean,                               \
                                                                                                             \
        [SIMPLEJS_NUMBER_TYPE_IPTR] = simplejs_number_get_##name##_iptr,                                     \
        [SIMPLEJS_NUMBER_TYPE_I32] = simplejs_number_get_##name##_i32,                                       \
        [SIMPLEJS_NUMBER_TYPE_I64] = simplejs_number_get_##name##_i64,                                       \
                                                                                                             \
        [SIMPLEJS_NUMBER_TYPE_UIPTR] = simplejs_number_get_##name##_uiptr,                                   \
        [SIMPLEJS_NUMBER_TYPE_UI32] = simplejs_number_get_##name##_ui32,                                     \
        [SIMPLEJS_NUMBER_TYPE_UI64] = simplejs_number_get_##name##_ui64,                                     \
                                                                                                             \
        [SIMPLEJS_NUMBER_TYPE_F32] = simplejs_number_get_##name##_f32,                                       \
        [SIMPLEJS_NUMBER_TYPE_F64] = simplejs_number_get_##name##_f64,                                       \
    }

GEN_NUMBER_FUNC_TABLE(uint32_t, int32_t, int32);
GEN_NUMBER_FUNC_TABLE(uint64_t, int64_t, int64);
GEN_NUMBER_FUNC_TABLE(uintptr_t, intptr_t, intptr);
GEN_NUMBER_FUNC_TABLE(float, float, float32);
GEN_NUMBER_FUNC_TABLE(double, double, float64);

float SIMPLEJS_API simplejs_number_get_float32(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_float32_jumptable[number->type] != NULL);

    return (uint32_t)simplejs_number_get_float32_jumptable[number->type](number);
}

double SIMPLEJS_API simplejs_number_get_float64(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_float64_jumptable[number->type] != NULL);

    return simplejs_number_get_float64_jumptable[number->type](number);
}

uint32_t SIMPLEJS_API simplejs_number_get_int32(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_int32_jumptable[number->type] != NULL);

    return simplejs_number_get_int32_jumptable[number->type](number);
}

uint64_t SIMPLEJS_API simplejs_number_get_int64(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_int64_jumptable[number->type] != NULL);

    return simplejs_number_get_int64_jumptable[number->type](number);
}

uintptr_t SIMPLEJS_API simplejs_number_get_intptr(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_intptr_jumptable[number->type] != NULL);

    return simplejs_number_get_intptr_jumptable[number->type](number);
}
