#define simplejs_number_get_intXX_jumptable simplejs_number_get_int_jumptable

#define INT_NAME 32
#define UPPER_INT_NAME 32

#define INT_TYPE int32_t
#define UINT_TYPE uint32_t
#define SINT_TYPE int32_t

#define _ALU_NAME(name) i##name
#define _UPPER_ALU_NAME(name) I##name
#define ALU_NAME(name) _ALU_NAME(name)
#define UPPER_ALU_NAME(name) _UPPER_ALU_NAME(name)

#include "alu-i.c"
