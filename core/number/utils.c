#include <number.h>

uint64_t simplejs_number_get_int_undef(simplejs_number_t *number)
{
    return 0;
}

uint64_t simplejs_number_get_int_boolean(simplejs_number_t *number)
{
    return number->value.boolean;
}

uint64_t simplejs_number_get_int_iptr(simplejs_number_t *number)
{
    return number->value.iptr;
}

uint64_t simplejs_number_get_int_i32(simplejs_number_t *number)
{
    return number->value.i32;
}

uint64_t simplejs_number_get_int_i64(simplejs_number_t *number)
{
    return number->value.i64;
}

uint64_t simplejs_number_get_int_uiptr(simplejs_number_t *number)
{
    return number->value.uiptr;
}

uint64_t simplejs_number_get_int_ui32(simplejs_number_t *number)
{
    return number->value.ui32;
}

uint64_t simplejs_number_get_int_ui64(simplejs_number_t *number)
{
    return number->value.ui64;
}

uint64_t simplejs_number_get_int_f32(simplejs_number_t *number)
{
    return (int64_t)number->value.f32;
}

uint64_t simplejs_number_get_int_f64(simplejs_number_t *number)
{
    return (int64_t)number->value.f64;
}

float simplejs_number_get_float32_undef(simplejs_number_t *number)
{
    return 0.0f;
}

float simplejs_number_get_float32_boolean(simplejs_number_t *number)
{
    return (float)number->value.boolean;
}

float simplejs_number_get_float32_i32(simplejs_number_t *number)
{
    return (float)number->value.i32;
}

float simplejs_number_get_float32_ui32(simplejs_number_t *number)
{
    return (float)number->value.ui32;
}

float simplejs_number_get_float32_f32(simplejs_number_t *number)
{
    return (float)number->value.f32;
}

double simplejs_number_get_float64_undef(simplejs_number_t *number)
{
    return 0.0;
}

double simplejs_number_get_float64_boolean(simplejs_number_t *number)
{
    return (float)number->value.boolean;
}

double simplejs_number_get_float64_iptr(simplejs_number_t *number)
{
    return (double)number->value.iptr;
}

double simplejs_number_get_float64_i32(simplejs_number_t *number)
{
    return (double)number->value.i32;
}

double simplejs_number_get_float64_i64(simplejs_number_t *number)
{
    return (double)number->value.i64;
}

double simplejs_number_get_float64_uiptr(simplejs_number_t *number)
{
    return (double)number->value.uiptr;
}

double simplejs_number_get_float64_ui32(simplejs_number_t *number)
{
    return (double)number->value.ui32;
}

double simplejs_number_get_float64_ui64(simplejs_number_t *number)
{
    return (double)number->value.ui64;
}

double simplejs_number_get_float64_f32(simplejs_number_t *number)
{
    return (double)number->value.f32;
}

double simplejs_number_get_float64_f64(simplejs_number_t *number)
{
    return (double)number->value.f64;
}

uint64_t (*simplejs_number_get_int_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number) = {
    [SIMPLEJS_NUMBER_TYPE_UNDEFINED] = simplejs_number_get_int_undef,
    [SIMPLEJS_NUMBER_TYPE_NULL] = simplejs_number_get_int_undef,

    [SIMPLEJS_NUMBER_TYPE_BOOLEAN] = simplejs_number_get_int_boolean,

    [SIMPLEJS_NUMBER_TYPE_IPTR] = simplejs_number_get_int_iptr,
    [SIMPLEJS_NUMBER_TYPE_I32] = simplejs_number_get_int_i32,
    [SIMPLEJS_NUMBER_TYPE_I64] = simplejs_number_get_int_i64,

    [SIMPLEJS_NUMBER_TYPE_UIPTR] = simplejs_number_get_int_uiptr,
    [SIMPLEJS_NUMBER_TYPE_UI32] = simplejs_number_get_int_ui32,
    [SIMPLEJS_NUMBER_TYPE_UI64] = simplejs_number_get_int_ui64,

    [SIMPLEJS_NUMBER_TYPE_F32] = simplejs_number_get_int_f32,
    [SIMPLEJS_NUMBER_TYPE_F64] = simplejs_number_get_int_f64,
};

float (*simplejs_number_get_float32_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number) = {
    [SIMPLEJS_NUMBER_TYPE_UNDEFINED] = simplejs_number_get_float32_undef,
    [SIMPLEJS_NUMBER_TYPE_NULL] = simplejs_number_get_float32_undef,

    [SIMPLEJS_NUMBER_TYPE_BOOLEAN] = simplejs_number_get_float32_boolean,

    [SIMPLEJS_NUMBER_TYPE_I32] = simplejs_number_get_float32_i32,
    [SIMPLEJS_NUMBER_TYPE_I64] = NULL,

    [SIMPLEJS_NUMBER_TYPE_UI32] = simplejs_number_get_float32_ui32,
    [SIMPLEJS_NUMBER_TYPE_UI64] = NULL,

    [SIMPLEJS_NUMBER_TYPE_F32] = simplejs_number_get_float32_f32,
    [SIMPLEJS_NUMBER_TYPE_F64] = NULL,
};

double (*simplejs_number_get_float64_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number) = {
    [SIMPLEJS_NUMBER_TYPE_UNDEFINED] = simplejs_number_get_float64_undef,
    [SIMPLEJS_NUMBER_TYPE_NULL] = simplejs_number_get_float64_undef,

    [SIMPLEJS_NUMBER_TYPE_BOOLEAN] = simplejs_number_get_float64_boolean,

    [SIMPLEJS_NUMBER_TYPE_IPTR] = simplejs_number_get_float64_iptr,
    [SIMPLEJS_NUMBER_TYPE_I32] = simplejs_number_get_float64_i32,
    [SIMPLEJS_NUMBER_TYPE_I64] = simplejs_number_get_float64_i64,

    [SIMPLEJS_NUMBER_TYPE_UIPTR] = simplejs_number_get_float64_uiptr,
    [SIMPLEJS_NUMBER_TYPE_UI32] = simplejs_number_get_float64_ui32,
    [SIMPLEJS_NUMBER_TYPE_UI64] = simplejs_number_get_float64_ui64,

    [SIMPLEJS_NUMBER_TYPE_F32] = simplejs_number_get_float64_f32,
    [SIMPLEJS_NUMBER_TYPE_F64] = simplejs_number_get_float64_f64,
};

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
    SIMPLEJS_ASSERT(simplejs_number_get_int_jumptable[number->type] != NULL);

    return (uint32_t)simplejs_number_get_int_jumptable[number->type](number);
}

uint64_t SIMPLEJS_API simplejs_number_get_int64(simplejs_number_t *number)
{
    SIMPLEJS_ASSERT(number->type < SIMPLEJS_NUMBER_TYPE_END);
    SIMPLEJS_ASSERT(simplejs_number_get_int_jumptable[number->type] != NULL);

    return simplejs_number_get_int_jumptable[number->type](number);
}
