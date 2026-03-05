#pragma once
#include "default.h"

typedef enum
{
    SIMPLEJS_NUMBER_TYPE_UNDEFINED,
    SIMPLEJS_NUMBER_TYPE_NULL,

    SIMPLEJS_NUMBER_TYPE_I32,
    SIMPLEJS_NUMBER_TYPE_I64,

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

#pragma pack(push, 1)

typedef struct simplejs_number
{
    uint32_t reserved;
    uint32_t type;
    union
    {
        uint64_t generic;

        int32_t i32;
        int64_t i64;

        uint32_t ui32;
        uint64_t ui64;

        float f32;
        double f64;
    } value;
} simplejs_number_t;

#pragma pack(pop)

static_assert(sizeof(simplejs_number_t) == SIMPLEJS_NUMBER_SIZE);
