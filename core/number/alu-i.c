#include <number.h>

GENERATE_ALU_EXEC_DECL(inc, ALU_NAME(INT_NAME))
{
    GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME))
    ++;
}

GENERATE_ALU_EXEC_DECL(dec, ALU_NAME(INT_NAME))
{
    GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME))
    --;
}

GENERATE_ALU_EXEC_DECL(logical_not, ALU_NAME(INT_NAME))
{
    a->type = SIMPLEJS_NUMBER_TYPE_BOOLEAN;
    a->value.boolean = !GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME));
}

GENERATE_ALU_EXEC_DECL(bitwise_not, ALU_NAME(INT_NAME))
{
    GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME)) = ~GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME));
}

GENERATE_ALU_EXEC_DECL(neg, ALU_NAME(INT_NAME))
{
    GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME)) = -GENERATE_ALU_OUT_VALUE(a, ALU_NAME(INT_NAME));
}

GENERATE_ALU_EXEC_DECL(equal, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int == b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(not_equal, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int != b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(greater, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int > b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(below, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int < b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(greater_equal, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int >= b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(below_equal, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int <= b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(or, ALU_NAME(INT_NAME))
{
    UINT_TYPE a_int = (UINT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    UINT_TYPE b_int = (UINT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int | b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(and, ALU_NAME(INT_NAME))
{
    UINT_TYPE a_int = (UINT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    UINT_TYPE b_int = (UINT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int & b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(shl, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int << b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(shr, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int >> b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(sal, ALU_NAME(INT_NAME))
{
    SINT_TYPE a_int = (SINT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    SINT_TYPE b_int = (SINT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int << b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(sar, ALU_NAME(INT_NAME))
{
    SINT_TYPE a_int = (SINT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    SINT_TYPE b_int = (SINT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = (INT_TYPE)(a_int >> b_int);

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(add, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int + b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(sub, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int - b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(mul, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int * b_int;

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(div, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int / (b_int + (b_int == 0));

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

GENERATE_ALU_EXEC_DECL(mod, ALU_NAME(INT_NAME))
{
    INT_TYPE a_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[a->type](a);
    INT_TYPE b_int = (INT_TYPE)simplejs_number_get_intXX_jumptable[b->type](b);
    INT_TYPE result = a_int % (b_int + (b_int == 0));

    GENERATE_ALU_FLUSH_RESULT(result, out, ALU_NAME(INT_NAME), UPPER_ALU_NAME(UPPER_INT_NAME));
}

static void (*simplejs_execute_alu_fXX_op_table[SIMPLEJS_NUMBER_ALU_END])(simplejs_number_t *out, simplejs_number_t *a, simplejs_number_t *b) = {
    [SIMPLEJS_NUMBER_ALU_INC] = GENERATE_ALU_EXEC_IDENTIFIER(inc, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_DEC] = GENERATE_ALU_EXEC_IDENTIFIER(dec, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_LOGICAL_NOT] = GENERATE_ALU_EXEC_IDENTIFIER(logical_not, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_BITWISE_NOT] = GENERATE_ALU_EXEC_IDENTIFIER(bitwise_not, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_NEG] = GENERATE_ALU_EXEC_IDENTIFIER(neg, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_EQUAL] = GENERATE_ALU_EXEC_IDENTIFIER(equal, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_NOT_EQUAL] = GENERATE_ALU_EXEC_IDENTIFIER(not_equal, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_GREATER] = GENERATE_ALU_EXEC_IDENTIFIER(greater, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_BELOW] = GENERATE_ALU_EXEC_IDENTIFIER(below, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_GREATER_EQUAL] = GENERATE_ALU_EXEC_IDENTIFIER(greater_equal, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_BELOW_EQUAL] = GENERATE_ALU_EXEC_IDENTIFIER(below_equal, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_OR] = GENERATE_ALU_EXEC_IDENTIFIER(or, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_AND] = GENERATE_ALU_EXEC_IDENTIFIER(and, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_SHL] = GENERATE_ALU_EXEC_IDENTIFIER(shl, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_SHR] = GENERATE_ALU_EXEC_IDENTIFIER(shr, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_SAL] = GENERATE_ALU_EXEC_IDENTIFIER(sal, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_SAR] = GENERATE_ALU_EXEC_IDENTIFIER(sar, ALU_NAME(INT_NAME)),

    [SIMPLEJS_NUMBER_ALU_ADD] = GENERATE_ALU_EXEC_IDENTIFIER(add, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_SUB] = GENERATE_ALU_EXEC_IDENTIFIER(sub, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_MUL] = GENERATE_ALU_EXEC_IDENTIFIER(mul, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_DIV] = GENERATE_ALU_EXEC_IDENTIFIER(div, ALU_NAME(INT_NAME)),
    [SIMPLEJS_NUMBER_ALU_MOD] = GENERATE_ALU_EXEC_IDENTIFIER(mod, ALU_NAME(INT_NAME)),
};

GENERATE_ALU_DECL(ALU_NAME(INT_NAME))
{
    simplejs_execute_alu_fXX_op_table[op](out, a, b);
}
