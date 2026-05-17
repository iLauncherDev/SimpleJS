#include <number.h>

static bool i32_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_IPTR] = true,
    [SIMPLEJS_NUMBER_TYPE_I32] = true,
    [SIMPLEJS_NUMBER_TYPE_I64] = true,

    [SIMPLEJS_NUMBER_TYPE_UIPTR] = true,
    [SIMPLEJS_NUMBER_TYPE_UI32] = true,
    [SIMPLEJS_NUMBER_TYPE_UI64] = true,

    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool i64_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_I64] = true,

    [SIMPLEJS_NUMBER_TYPE_UIPTR] = true,
    [SIMPLEJS_NUMBER_TYPE_UI32] = true,
    [SIMPLEJS_NUMBER_TYPE_UI64] = true,

    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool uiptr_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_UIPTR] = true,
    [SIMPLEJS_NUMBER_TYPE_UI32] = true,
    [SIMPLEJS_NUMBER_TYPE_UI64] = true,

    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool ui32_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_UI32] = true,
    [SIMPLEJS_NUMBER_TYPE_UI64] = true,

    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool ui64_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_UI64] = true,

    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool float_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_F32] = true,
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static bool double_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_F64] = true,
};

static void (*simplejs_number_execute_alu_type_table[8])(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b) = {
    simplejs_number_execute_alu_iptr,
    simplejs_number_execute_alu_i32,
    simplejs_number_execute_alu_i64,
    simplejs_number_execute_alu_uiptr,
    simplejs_number_execute_alu_ui32,
    simplejs_number_execute_alu_ui64,
    simplejs_number_execute_alu_f32,
    simplejs_number_execute_alu_f64};

void simplejs_number_execute_alu(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
{
    uint8_t is_i32 = i32_check[a->type] | i32_check[b->type];
    uint8_t is_i64 = i64_check[a->type] | i64_check[b->type];

    uint8_t is_uiptr = uiptr_check[a->type] | uiptr_check[b->type];
    uint8_t is_ui32 = ui32_check[a->type] | ui32_check[b->type];
    uint8_t is_ui64 = ui64_check[a->type] | ui64_check[b->type];

    uint8_t is_float = float_check[a->type] | float_check[b->type];
    uint8_t is_double = double_check[a->type] | double_check[b->type];

    uint8_t alu_type = is_i32 + is_i64 + is_uiptr + is_ui32 + is_ui64 + is_float + is_double;

    simplejs_number_execute_alu_type_table[alu_type](op, out, a, b);
}
