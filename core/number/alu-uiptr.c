#define simplejs_number_get_intXX_jumptable simplejs_number_get_int_jumptable

#define INT_NAME ptr
#define UPPER_INT_NAME PTR

#define INT_TYPE uintptr_t
#define UINT_TYPE uintptr_t
#define SINT_TYPE intptr_t

#define _ALU_NAME(name) ui##name
#define _UPPER_ALU_NAME(name) UI##name
#define ALU_NAME(name) _ALU_NAME(name)
#define UPPER_ALU_NAME(name) _UPPER_ALU_NAME(name)

#include "alu-i.c"
