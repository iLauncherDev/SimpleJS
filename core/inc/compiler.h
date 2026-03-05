#pragma once
#include "bytecode.h"
#include "parser.h"

typedef enum simplejs_compiler_instruction_type
{
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_NORMAL,
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO,
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_FUNCTION,
} simplejs_compiler_instruction_type_t;

typedef struct simplejs_compiler_instruction
{
    simplejs_compiler_instruction_type_t type;

    simplejs_bytecode_instruction_t instruction;
    struct 
    {
        simplejs_ast_node_t *node;
        uint32_t data_offset;
        uintptr_t label_id;
    } symbol;

    simplejs_list_entry_t list_entry;
} simplejs_compiler_instruction_t;

typedef struct simplejs_compiler_ctx
{
    simplejs_parser_ctx_t *parser_ctx;

    uint32_t data_offset;
    uint32_t instruction_list_count;
    simplejs_list_entry_t instruction_list;

    uint8_t *executable;
    uint32_t executable_size;
} simplejs_compiler_ctx_t;

typedef struct simplejs_compile_reg_info
{
    bool is_write;
    uint8_t reg_a, reg_b;
} simplejs_compile_reg_info_t;

#include <simplejs/compiler.h>
