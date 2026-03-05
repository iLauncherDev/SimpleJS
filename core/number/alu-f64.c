#define simplejs_number_get_floatXX_jumptable simplejs_number_get_float64_jumptable
#define simplejs_number_get_intXX_jumptable simplejs_number_get_int_jumptable

#define FLOAT_NAME 64
#define FLOAT_TYPE double
#define INT_TYPE uint64_t

#include "alu-f.c"
