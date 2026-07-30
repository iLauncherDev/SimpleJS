#pragma once
#include "default.h"

typedef enum
{
    SIMPLEJS_NUMBER_TYPE_UNDEFINED,
    SIMPLEJS_NUMBER_TYPE_NULL,

    SIMPLEJS_NUMBER_TYPE_BOOLEAN,

    SIMPLEJS_NUMBER_TYPE_IPTR,
    SIMPLEJS_NUMBER_TYPE_I32,
    SIMPLEJS_NUMBER_TYPE_I64,

    SIMPLEJS_NUMBER_TYPE_UIPTR,
    SIMPLEJS_NUMBER_TYPE_UI32,
    SIMPLEJS_NUMBER_TYPE_UI64,

    SIMPLEJS_NUMBER_TYPE_F32,
    SIMPLEJS_NUMBER_TYPE_F64,

    SIMPLEJS_NUMBER_TYPE_END,
} simplejs_number_type_t;

#ifndef SIMPLEJS_NUMBER_TYPE_DEFAULT
#define SIMPLEJS_NUMBER_TYPE_DEFAULT SIMPLEJS_NUMBER_TYPE_F64
#endif

#define SIMPLEJS_NUMBER_SIZE 16

typedef struct simplejs_number
{
    uint16_t reserved;
    uint16_t type;
    union
    {
        uint64_t generic;

        bool boolean;

        intptr_t iptr;
        int32_t i32;
        int64_t i64;

        uintptr_t uiptr;
        uint32_t ui32;
        uint64_t ui64;

        float f32;
        double f64;
    } value;
} simplejs_number_t;

static_assert(sizeof(simplejs_number_t) == SIMPLEJS_NUMBER_SIZE);

float SIMPLEJS_API simplejs_number_get_float32(simplejs_number_t *number);
double SIMPLEJS_API simplejs_number_get_float64(simplejs_number_t *number);
uint32_t SIMPLEJS_API simplejs_number_get_int32(simplejs_number_t *number);
uint64_t SIMPLEJS_API simplejs_number_get_int64(simplejs_number_t *number);
