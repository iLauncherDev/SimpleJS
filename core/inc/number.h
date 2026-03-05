#pragma once
#include "default.h"
#include <simplejs/number.h>

typedef enum
{
    SIMPLEJS_NUMBER_ALU_INC,
    SIMPLEJS_NUMBER_ALU_DEC,

    SIMPLEJS_NUMBER_ALU_EQUAL,
    SIMPLEJS_NUMBER_ALU_GREATER,
    SIMPLEJS_NUMBER_ALU_BELOW,

    SIMPLEJS_NUMBER_ALU_ADD,
    SIMPLEJS_NUMBER_ALU_SUB,
    SIMPLEJS_NUMBER_ALU_MUL,
    SIMPLEJS_NUMBER_ALU_DIV,
    SIMPLEJS_NUMBER_ALU_MOD,

    SIMPLEJS_NUMBER_ALU_END,
} simplejs_number_alu_t;

#define _GENERATE_ALU_FLOAT_EXEC_DECL(op, name) \
    static void simplejs_execute_alu_f##name##_##op(simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
#define GENERATE_ALU_FLOAT_EXEC_DECL(op, name) _GENERATE_ALU_FLOAT_EXEC_DECL(op, name)

#define _GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(op, name) \
    simplejs_execute_alu_f##name##_##op
#define GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(op, name) _GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(op, name)

#define _GENERATE_ALU_FLOAT_DECL(name) \
    void simplejs_number_execute_alu_f##name(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b)
#define GENERATE_ALU_FLOAT_DECL(name) _GENERATE_ALU_FLOAT_DECL(name)

#define _GENERATE_ALU_FLOAT_OUT_VALUE(out, name) \
    out->value.f##name
#define GENERATE_ALU_FLOAT_OUT_VALUE(out, name) _GENERATE_ALU_FLOAT_OUT_VALUE(out, name)

#define _GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, name) \
    out->type = SIMPLEJS_NUMBER_TYPE_F##name; \
    GENERATE_ALU_FLOAT_OUT_VALUE(out, name) = result
#define GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, name) _GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, name)

extern uint64_t (*simplejs_number_get_int_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);
extern float (*simplejs_number_get_float32_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);
extern double (*simplejs_number_get_float64_jumptable[SIMPLEJS_NUMBER_TYPE_END])(simplejs_number_t *number);

GENERATE_ALU_FLOAT_DECL(32);
GENERATE_ALU_FLOAT_DECL(64);

void simplejs_number_execute_alu(simplejs_number_alu_t op, simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b);

uint32_t simplejs_number_get_int32(simplejs_number_t *number);
uint64_t simplejs_number_get_int64(simplejs_number_t *number);

void simplejs_number_encode(uint8_t *buffer, simplejs_number_t *number);
void simplejs_number_decode(simplejs_number_t *number, uint8_t *buffer);
