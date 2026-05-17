#pragma once
#include "tokenizer.h"
#include "lib/utf8.h"
#include <simplejs/parser.h>

typedef enum
{
    SIMPLEJS_AST_NODE_TYPE_EXPRESSION,

    SIMPLEJS_AST_NODE_TYPE_RETURN,

    SIMPLEJS_AST_NODE_TYPE_BRANCH,

    SIMPLEJS_AST_NODE_TYPE_IF,
    SIMPLEJS_AST_NODE_TYPE_ELSE,
    SIMPLEJS_AST_NODE_TYPE_ELSE_IF,

    SIMPLEJS_AST_NODE_TYPE_FOR_LOOP,
    SIMPLEJS_AST_NODE_TYPE_WHILE_LOOP,

    SIMPLEJS_AST_NODE_TYPE_LABEL,
    SIMPLEJS_AST_NODE_TYPE_GOTO,

    SIMPLEJS_AST_NODE_TYPE_UNARY_OPERATOR,
    SIMPLEJS_AST_NODE_TYPE_BINARY_OPERATOR,

    SIMPLEJS_AST_NODE_TYPE_NUMBER,
    SIMPLEJS_AST_NODE_TYPE_STRING,

    SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE,
    SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE,
    SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE,
    SIMPLEJS_AST_NODE_TYPE_ARGUMENT_REFERENCE,
    SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE,

    SIMPLEJS_AST_NODE_TYPE_ASSIGN,

    SIMPLEJS_AST_NODE_TYPE_LOGICAL_OR,
    SIMPLEJS_AST_NODE_TYPE_LOGICAL_AND,

    SIMPLEJS_AST_NODE_TYPE_ALU_INC,
    SIMPLEJS_AST_NODE_TYPE_ALU_DEC,

    SIMPLEJS_AST_NODE_TYPE_ALU_NOT,
    SIMPLEJS_AST_NODE_TYPE_ALU_NEG,

    SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL,
    SIMPLEJS_AST_NODE_TYPE_ALU_NOT_EQUAL,
    SIMPLEJS_AST_NODE_TYPE_ALU_GREATER,
    SIMPLEJS_AST_NODE_TYPE_ALU_BELOW,

    SIMPLEJS_AST_NODE_TYPE_ALU_GREATER_EQUAL,
    SIMPLEJS_AST_NODE_TYPE_ALU_BELOW_EQUAL,

    SIMPLEJS_AST_NODE_TYPE_ALU_OR,
    SIMPLEJS_AST_NODE_TYPE_ALU_AND,

    SIMPLEJS_AST_NODE_TYPE_ALU_SHL,
    SIMPLEJS_AST_NODE_TYPE_ALU_SHR,
    SIMPLEJS_AST_NODE_TYPE_ALU_SAL,
    SIMPLEJS_AST_NODE_TYPE_ALU_SAR,

    SIMPLEJS_AST_NODE_TYPE_ALU_ADD,
    SIMPLEJS_AST_NODE_TYPE_ALU_SUB,
    SIMPLEJS_AST_NODE_TYPE_ALU_MUL,
    SIMPLEJS_AST_NODE_TYPE_ALU_DIV,
    SIMPLEJS_AST_NODE_TYPE_ALU_MOD,

    SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL,

    SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS,

    SIMPLEJS_AST_NODE_TYPE_CODEBLOCK,

    SIMPLEJS_AST_NODE_TYPE_VARDECL,
    SIMPLEJS_AST_NODE_TYPE_FUNCDECL,

    SIMPLEJS_AST_NODE_TYPE_ROOT,

    SIMPLEJS_AST_NODE_TYPE_END,
} simplejs_ast_node_type_t;

typedef enum
{
    SIMPLEJS_PARSER_STATE_IDLE,
    SIMPLEJS_PARSER_STATE_CODEBLOCK,

    SIMPLEJS_PARSER_STATE_BRANCH_IF,
    SIMPLEJS_PARSER_STATE_BRANCH_IF_EXPR,
    SIMPLEJS_PARSER_STATE_BRANCH_ELSE,
    SIMPLEJS_PARSER_STATE_BRANCH_CODE,
    SIMPLEJS_PARSER_STATE_BRANCH_END,

    SIMPLEJS_PARSER_STATE_FUNCDECL_NAME,
    SIMPLEJS_PARSER_STATE_FUNCDECL_ARGS,
    SIMPLEJS_PARSER_STATE_FUNCDECL_BLOCK,
    SIMPLEJS_PARSER_STATE_FUNCDECL_END,

    SIMPLEJS_PARSER_STATE_VARDECL_NAME,
    SIMPLEJS_PARSER_STATE_VARDECL_END,

    SIMPLEJS_PARSER_STATE_LABEL_NAME,
    SIMPLEJS_PARSER_STATE_GOTO_NAME,

    SIMPLEJS_PARSER_STATE_WHILE_LOOP_ARGS,
    SIMPLEJS_PARSER_STATE_WHILE_LOOP_CONDITION,
    SIMPLEJS_PARSER_STATE_WHILE_LOOP_CODE,
    SIMPLEJS_PARSER_STATE_WHILE_LOOP_END,

    SIMPLEJS_PARSER_STATE_FOR_LOOP_ARGS,
    SIMPLEJS_PARSER_STATE_FOR_LOOP_INIT,
    SIMPLEJS_PARSER_STATE_FOR_LOOP_CONDITION,
    SIMPLEJS_PARSER_STATE_FOR_LOOP_STEP,
    SIMPLEJS_PARSER_STATE_FOR_LOOP_CODE,
    SIMPLEJS_PARSER_STATE_FOR_LOOP_END,

    SIMPLEJS_PARSER_STATE_EXPRESSION,

    SIMPLEJS_PARSER_STATE_RETURN_EXPRESSION,
    SIMPLEJS_PARSER_STATE_RETURN_END,
} simplejs_parser_state_t;

typedef struct simplejs_ast_node
{
    simplejs_parser_state_t _saved_parser_state;

    simplejs_ast_node_type_t type;

    void *parent_node;
    void *context;
    uintptr_t flags;

    uint32_t children_list_count;
    simplejs_list_entry_t children_list_entry;

    simplejs_list_entry_t list_entry;

    simplejs_list_entry_t _stack_list_entry;
    simplejs_list_entry_t _ast_function_list_entry;
} simplejs_ast_node_t;

typedef struct simplejs_ast_branch_context
{
    bool ended_branch;

    bool expecting_if;
} simplejs_ast_branch_context_t;

typedef struct simplejs_ast_scope_context
{
    uint32_t var_count;

    simplejs_list_entry_t function_list_entry;
    simplejs_list_entry_t arg_list_entry;
    simplejs_list_entry_t var_list_entry;
    simplejs_list_entry_t label_list_entry;

    simplejs_list_entry_t _function_scope_list_entry;
} simplejs_ast_scope_context_t;

typedef struct simplejs_ast_function_context
{
    bool _process_arg;

    simplejs_utf8_string_t *name;

    uint32_t local_var_count, local_arg_count;

    simplejs_ast_scope_context_t *root_scope;

    simplejs_list_entry_t scope_stack;
    simplejs_ast_scope_context_t *current_scope_stack;

    simplejs_list_entry_t argument_list;

    simplejs_list_entry_t _stack_list_entry;
    simplejs_list_entry_t _scope_list_entry;
} simplejs_ast_function_context_t;

typedef struct simplejs_ast_var_context
{
    uint32_t index;
    simplejs_utf8_string_t *name;

    simplejs_list_entry_t _scope_var_list_entry;
} simplejs_ast_var_context_t;

typedef struct simplejs_ast_label_context
{
    simplejs_utf8_string_t *name;

    simplejs_list_entry_t _scope_label_list_entry;
} simplejs_ast_label_context_t;

struct simplejs_parser_ctx
{
    simplejs_parser_state_t state;

    simplejs_ast_node_t *root_ast;
    simplejs_list_entry_t ast_function_list;

    simplejs_list_entry_t ast_stack;
    simplejs_ast_node_t *current_ast_stack;

    simplejs_list_entry_t function_context_stack;
    simplejs_ast_function_context_t *current_function_context_stack;
};

const char *simplejs_get_ast_node_type_string(simplejs_ast_node_type_t type);
simplejs_status_t simplejs_parse_expression(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token, simplejs_ast_node_t **out, int min_bp);
void simplejs_free_ast_list(simplejs_ast_node_t *node);
