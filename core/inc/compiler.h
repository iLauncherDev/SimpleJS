#pragma once
#include "bytecode.h"
#include "parser.h"
#include <simplejs/compiler.h>

typedef enum simplejs_compiler_instruction_type
{
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_NORMAL,
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO,
    SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_FUNCTION,
} simplejs_compiler_instruction_type_t;

typedef struct simplejs_compiler_debug
{
    simplejs_token_t *diagnostic_token;
    simplejs_linemap_offset_t diagnostic_offset;

    uint32_t _debug_offset;

    uint32_t flags;
    uint32_t children_list_count;
    simplejs_list_entry_t children_list_entry;

    simplejs_linemap_offset_t code_offset;
    simplejs_linemap_offset_t source_offset;

    simplejs_list_entry_t _temp_list_entry;
    simplejs_list_entry_t list_entry;
} simplejs_compiler_debug_t;

typedef struct simplejs_compiler_instruction
{
    simplejs_compiler_instruction_type_t type;

    simplejs_compiler_debug_t *compiler_debug;

    simplejs_bytecode_instruction_t instruction;

    struct
    {
        simplejs_ast_node_t *node;
        uint32_t data_offset;
        uintptr_t label_id;
    } symbol;

    simplejs_list_entry_t list_entry;
} simplejs_compiler_instruction_t;

typedef struct simplejs_compiler_ast_info
{
    uintptr_t return_label_id;
    uintptr_t break_label_id;
    uintptr_t loop_continue_label_id;
} simplejs_compiler_ast_info_t;

typedef struct simplejs_compiler_reg_info
{
    uint32_t debug_flags;

    bool is_sub_op, is_sub_assign, avoid_refetch;
    bool is_write, have_parent;
    uint8_t reg_a, reg_b, reg_parent;
} simplejs_compiler_reg_info_t;

struct simplejs_compiler_ctx
{
    simplejs_parser_ctx_t *parser_ctx;

    uint32_t data_offset;

    uint32_t instruction_list_count;
    uint32_t debug_list_count;
    uint32_t debug_list_total_count;

    simplejs_list_entry_t instruction_list;
    simplejs_list_entry_t debug_list;

    uint8_t *executable;
    uint32_t executable_size;
};

void simplejs_disasm_bytecode(simplejs_bytecode_instruction_t instruction, uintptr_t instruction_pointer);
