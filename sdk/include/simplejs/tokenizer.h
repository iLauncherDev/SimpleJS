#pragma once
#include "default.h"
#include "number.h"

typedef struct simplejs_token_ctx simplejs_token_ctx_t;

void SIMPLEJS_API simplejs_tokenize_dump_tokens(simplejs_token_ctx_t *ctx);

void SIMPLEJS_API simplejs_free_token_ctx(simplejs_token_ctx_t *ctx);
simplejs_status_t SIMPLEJS_API simplejs_tokenize(char *file_path, simplejs_utf8_string_t *code, simplejs_token_ctx_t **out);
