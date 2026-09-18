#pragma once
#include "default.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct simplejs_hash_string
{
    uint64_t hash;
    char string[];
} simplejs_hash_string_t;

typedef struct simplejs_hash_string_ctx
{
    uint64_t hash;
    const char *string;
} simplejs_hash_string_ctx_t;

uint64_t SIMPLEJS_API simplejs_hash_string_generate_hash(const char *string);
void SIMPLEJS_API simplejs_init_hash_string_ctx(simplejs_hash_string_ctx_t *hash_string_ctx, const char *string);
void SIMPLEJS_API simplejs_hash_string_get_context(simplejs_hash_string_ctx_t *out, const simplejs_hash_string_t *in);

#ifdef __cplusplus
}
#endif
