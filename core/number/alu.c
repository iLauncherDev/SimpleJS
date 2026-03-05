#include <number.h>

static bool ui64_check[SIMPLEJS_NUMBER_TYPE_END] = {
    [SIMPLEJS_NUMBER_TYPE_I64] = true,
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

static void (*simplejs_number_execute_alu_type_table[4])(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b) = {
    NULL,
    NULL,
    simplejs_number_execute_alu_f32,
    simplejs_number_execute_alu_f64
};

void simplejs_number_execute_alu(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
{
    uint8_t is_ui64 = ui64_check[a->type] | ui64_check[b->type];
    uint8_t is_float = float_check[a->type] | float_check[b->type];
    uint8_t is_double = double_check[a->type] | double_check[b->type];

    uint8_t alu_type = is_ui64 + is_float + is_double;

    simplejs_number_execute_alu_type_table[alu_type](op, out, a, b);
}
