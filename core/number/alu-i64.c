#define simplejs_number_get_intXX_jumptable simplejs_number_get_int_jumptable

#define INT_NAME 64
#define UPPER_INT_NAME 64

#define INT_TYPE int64_t
#define UINT_TYPE uint64_t
#define SINT_TYPE int64_t

#define _ALU_NAME(name) i##name
#define _UPPER_ALU_NAME(name) I##name
#define ALU_NAME(name) _ALU_NAME(name)
#define UPPER_ALU_NAME(name) _UPPER_ALU_NAME(name)

#include "alu-i.c"
