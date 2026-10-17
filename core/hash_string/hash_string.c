#include <hash_string.h>
#include <lib/fnv1a_hash.h>

uint64_t SIMPLEJS_API simplejs_hash_string_generate_hash(const char *string)
{
    SIMPLEJS_ASSERT(string != NULL);

    return simplejs_generate_fnv1a_hash(string, (size_t)-1, true);
}

void SIMPLEJS_API simplejs_init_hash_string(simplejs_hash_string_t *hash_string, const char *string)
{
    SIMPLEJS_ASSERT(hash_string != NULL);
    SIMPLEJS_ASSERT(string != NULL);

    memcpy(hash_string->string, string, strlen(string) + 1);
    hash_string->hash = simplejs_hash_string_generate_hash(hash_string->string);
}

void SIMPLEJS_API simplejs_init_hash_string_ctx(simplejs_hash_string_ctx_t *hash_string_ctx, const char *string)
{
    SIMPLEJS_ASSERT(hash_string_ctx != NULL);
    SIMPLEJS_ASSERT(string != NULL);

    hash_string_ctx->string = string;
    hash_string_ctx->hash = simplejs_hash_string_generate_hash(hash_string_ctx->string);
}

void SIMPLEJS_API simplejs_hash_string_get_context(simplejs_hash_string_ctx_t *out, const simplejs_hash_string_t *in)
{
    SIMPLEJS_ASSERT(out != NULL);
    SIMPLEJS_ASSERT(in != NULL);

    out->hash = in->hash;
    out->string = in->string;
}
