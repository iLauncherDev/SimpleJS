#pragma once
#include "default.h"
#include <simplejs/number.h>

typedef enum
{
    SIMPLEJS_NUMBER_ALU_INC,
    SIMPLEJS_NUMBER_ALU_DEC,

    SIMPLEJS_NUMBER_ALU_LOGICAL_NOT,
    SIMPLEJS_NUMBER_ALU_BITWISE_NOT,
    SIMPLEJS_NUMBER_ALU_NEG,

    SIMPLEJS_NUMBER_ALU_EQUAL,
    SIMPLEJS_NUMBER_ALU_NOT_EQUAL,
    SIMPLEJS_NUMBER_ALU_GREATER,
    SIMPLEJS_NUMBER_ALU_BELOW,
    SIMPLEJS_NUMBER_ALU_GREATER_EQUAL,
    SIMPLEJS_NUMBER_ALU_BELOW_EQUAL,

    SIMPLEJS_NUMBER_ALU_OR,
    SIMPLEJS_NUMBER_ALU_AND,

    SIMPLEJS_NUMBER_ALU_SHL,
    SIMPLEJS_NUMBER_ALU_SHR,

    SIMPLEJS_NUMBER_ALU_SAL,
    SIMPLEJS_NUMBER_ALU_SAR,

    SIMPLEJS_NUMBER_ALU_ADD,
    SIMPLEJS_NUMBER_ALU_SUB,
    SIMPLEJS_NUMBER_ALU_MUL,
    SIMPLEJS_NUMBER_ALU_DIV,
    SIMPLEJS_NUMBER_ALU_MOD,

    SIMPLEJS_NUMBER_ALU_END,
} simplejs_number_alu_t;

#define _GENERATE_ALU_EXEC_DECL(op, name) \
    static void simplejs_execute_alu_##name##_##op(simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
#define GENERATE_ALU_EXEC_DECL(op, name) _GENERATE_ALU_EXEC_DECL(op, name)

#define _GENERATE_ALU_EXEC_IDENTIFIER(op, name) \
    simplejs_execute_alu_##name##_##op
#define GENERATE_ALU_EXEC_IDENTIFIER(op, name) _GENERATE_ALU_EXEC_IDENTIFIER(op, name)

#define _GENERATE_ALU_DECL(name) \
    void simplejs_number_execute_alu_##name(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
#define GENERATE_ALU_DECL(name) _GENERATE_ALU_DECL(name)

#define _GENERATE_ALU_OUT_VALUE(out, name) \
    out->value.name
#define GENERATE_ALU_OUT_VALUE(out, name) _GENERATE_ALU_OUT_VALUE(out, name)

#define _GENERATE_ALU_FLUSH_RESULT(result, out, name, upper_name) \
    out->type = SIMPLEJS_NUMBER_TYPE_##upper_name;                \
    GENERATE_ALU_OUT_VALUE(out, name) = result
#define GENERATE_ALU_FLUSH_RESULT(result, out, name, upper_name) _GENERATE_ALU_FLUSH_RESULT(result, out, name, upper_name)

extern uint64_t (*simplejs_number_get_int_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);
extern float (*simplejs_number_get_float32_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);
extern double (*simplejs_number_get_float64_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);

GENERATE_ALU_DECL(iptr);
GENERATE_ALU_DECL(i32);
GENERATE_ALU_DECL(i64);

GENERATE_ALU_DECL(uiptr);
GENERATE_ALU_DECL(ui32);
GENERATE_ALU_DECL(ui64);

GENERATE_ALU_DECL(f32);
GENERATE_ALU_DECL(f64);

void simplejs_number_execute_alu(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b);

void simplejs_number_encode(uint8_t *buffer, simplejs_number_t *number);
void simplejs_number_decode(simplejs_number_t *number, uint8_t *buffer);
