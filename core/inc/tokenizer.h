#pragma once
#include "default.h"
#include "number.h"
#include "lib/utf8.h"
#include <simplejs/tokenizer.h>

typedef enum simplejs_token_state
{
    SIMPLEJS_TOKEN_STATE_IDLE,
    SIMPLEJS_TOKEN_STATE_TEMPLATE_EXPR,
    SIMPLEJS_TOKEN_STATE_NUMBER,
    SIMPLEJS_TOKEN_STATE_STRING,
    SIMPLEJS_TOKEN_STATE_DOUBLE_STRING,
    SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING,
    SIMPLEJS_TOKEN_STATE_ONE_LINE_COMMENT,
    SIMPLEJS_TOKEN_STATE_MULTI_LINE_COMMENT,
} simplejs_token_state_t;

typedef enum simplejs_token_type
{
    SIMPLEJS_TOKEN_TYPE_IDENTIFIER,
    SIMPLEJS_TOKEN_TYPE_KEYWORD,
    SIMPLEJS_TOKEN_TYPE_EXPR_KEYWORD,
    SIMPLEJS_TOKEN_TYPE_OPERATOR,
    SIMPLEJS_TOKEN_TYPE_NUMBER,
    SIMPLEJS_TOKEN_TYPE_STRING,

    SIMPLEJS_TOKEN_TYPE_END,
} simplejs_token_type_t;

typedef struct simplejs_composite_token_entry
{
    simplejs_utf8_string_t tokens;
} simplejs_composite_token_entry_t;

typedef struct simplejs_composite_token
{
    uint8_t maxLen;
    uint8_t entries;
    simplejs_composite_token_entry_t *list;
} simplejs_composite_token_t;

typedef struct simplejs_token
{
    simplejs_token_type_t type;
    struct
    {
        simplejs_utf8_string_t *string;
        simplejs_number_t number;
    };

    struct
    {
        uint64_t start, end;
    } offset;

    simplejs_list_entry_t list_entry;

    uint32_t _arg_index;
    simplejs_list_entry_t _arg_list_entry;
} simplejs_token_t;

struct simplejs_token_ctx
{
    simplejs_utf8_string_t *code;
    simplejs_token_state_t state;
    struct
    {
        bool startWithZeroPrefix;
    } numberState;

    size_t offset, len, index;
    size_t template_expr_level;

    simplejs_list_entry_t token_list;
};

char *simplejs_get_token_type_string(simplejs_token_type_t type);
char *simplejs_get_token_state_string(simplejs_token_state_t type);
