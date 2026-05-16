#pragma once
#include "tokenizer.h"

typedef struct simplejs_parser_ctx simplejs_parser_ctx_t;

void SIMPLEJS_API simplejs_free_parser_ctx(simplejs_parser_ctx_t *parser_ctx);
simplejs_status_t SIMPLEJS_API simplejs_tokens_to_ast(simplejs_token_ctx_t *token_ctx, simplejs_parser_ctx_t **out);
