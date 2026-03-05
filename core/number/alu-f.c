#include <number.h>

GENERATE_ALU_FLOAT_EXEC_DECL(inc, FLOAT_NAME)
{
    GENERATE_ALU_FLOAT_OUT_VALUE(a, FLOAT_NAME)
    ++;
}

GENERATE_ALU_FLOAT_EXEC_DECL(dec, FLOAT_NAME)
{
    GENERATE_ALU_FLOAT_OUT_VALUE(a, FLOAT_NAME)
    --;
}

GENERATE_ALU_FLOAT_EXEC_DECL(equal, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = (FLOAT_TYPE)(a_float == b_float);

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(greater, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = (FLOAT_TYPE)(a_float > b_float);

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(below, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = (FLOAT_TYPE)(a_float < b_float);

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(add, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = a_float + b_float;

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(sub, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = a_float - b_float;

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(mul, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = a_float * b_float;

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(div, FLOAT_NAME)
{
    FLOAT_TYPE a_float = simplejs_number_get_floatXX_jumptable[a->type](a);
    FLOAT_TYPE b_float = simplejs_number_get_floatXX_jumptable[b->type](b);
    FLOAT_TYPE result = a_float / (b_float + (b_float == 0.0 || b_float == -0.0));

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

GENERATE_ALU_FLOAT_EXEC_DECL(mod, FLOAT_NAME)
{
    INT_TYPE a_float = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_float = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    FLOAT_TYPE result = (FLOAT_TYPE)(a_float % (b_float + (b_float == 0)));

    GENERATE_ALU_FLOAT_FLUSH_RESULT(result, out, FLOAT_NAME);
}

static void (*simplejs_execute_alu_fXX_op_table[SIMPLEJS_NUMBER_ALU_END])(simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b) = {
    [SIMPLEJS_NUMBER_ALU_INC] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(inc, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_DEC] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(dec, FLOAT_NAME),

    [SIMPLEJS_NUMBER_ALU_EQUAL] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(equal, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_GREATER] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(greater, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_BELOW] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(below, FLOAT_NAME),

    [SIMPLEJS_NUMBER_ALU_ADD] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(add, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_SUB] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(sub, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_MUL] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(mul, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_DIV] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(div, FLOAT_NAME),
    [SIMPLEJS_NUMBER_ALU_MOD] = GENERATE_ALU_FLOAT_EXEC_IDENTIFIER(mod, FLOAT_NAME),
};

GENERATE_ALU_FLOAT_DECL(FLOAT_NAME)
{
    simplejs_execute_alu_fXX_op_table[op](out, a, b);
}
