#pragma once
#include "parser.h"

#ifndef SIMPLEJS_CORE
typedef struct simplejs_compiler_ctx simplejs_compiler_ctx_t;
#endif

simplejs_status_t SIMPLEJS_API simplejs_ast_to_bytecode(simplejs_parser_ctx_t *parser_ctx, simplejs_compiler_ctx_t **out);
