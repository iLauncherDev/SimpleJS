#pragma once
#include "default.h"
#include "number.h"
#include "linemap.h"

typedef struct simplejs_token_ctx simplejs_token_ctx_t;

void SIMPLEJS_API simplejs_tokenize_dump_tokens(simplejs_token_ctx_t *ctx);

void SIMPLEJS_API simplejs_free_token_ctx(simplejs_token_ctx_t *ctx);
simplejs_status_t SIMPLEJS_API simplejs_tokenize(simplejs_linemap_ctx_t *linemap_ctx, simplejs_token_ctx_t **out);
