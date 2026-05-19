#pragma once
#include "parser.h"

typedef struct simplejs_compiler_ctx simplejs_compiler_ctx_t;

void SIMPLEJS_API simplejs_free_compiler_ctx(simplejs_compiler_ctx_t *compiler_ctx);
simplejs_status_t SIMPLEJS_API simplejs_ast_to_bytecode(simplejs_parser_ctx_t *parser_ctx, simplejs_compiler_ctx_t **out);
void SIMPLEJS_API simplejs_compiler_ctx_get_executable(simplejs_compiler_ctx_t *compiler_ctx, void **executable_out, uint32_t *size_out);
uintptr_t SIMPLEJS_API simplejs_compiler_get_executable_entry_point(void *executable, uint32_t size);
