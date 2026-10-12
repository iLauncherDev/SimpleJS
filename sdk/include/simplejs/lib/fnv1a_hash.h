#pragma once
#include "../default.h"

#ifdef __cplusplus
extern "C" {
#endif

uint64_t SIMPLEJS_API simplejs_generate_fnv1a_hash(const void *data, size_t data_size, bool has_null_terminator);

#ifdef __cplusplus
}
#endif
