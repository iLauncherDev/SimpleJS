#include <tokenizer.h>

static char *useless_chars = "\r";
static char *separators_chars = " \n";
static char *valid_operators = "~;:,.${}()[]<>+-*/%|&!=";

static char *oneline_comment_operators = "//";

static char *multiline_comment_operators_start = "/*";
static char *multiline_comment_operators_end = "*/";

static char curly_brace_left = '{';
static char curly_brace_right = '}';

static char *template_expr_start = "${";
static char *template_expr_end = "}";

static char special_char_vector[] = {
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
    ['b'] = '\b',
};

char *keywords_list_string[] = {
    "const", "let", "var",
    "if", "else",
    "for", "while",
    "class", "extends",
    "async", "function", "return",
    "label", "goto"};

char *expr_keywords_list_string[] = {
    "typeof", "delete", "new",
    "true", "false",
    "null", "undefined",
    "this", "super",
    "globalThis"};

char *composite_list_string[] = {
    "=",
    "==",
    "<=",
    ">=",
    "!=",

    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "<<=",
    ">>=",
    "<<<=",
    ">>>=",

    "=>",

    "&",
    "&&",

    "|",
    "||",

    ">>",
    "<<",

    "<<<",
    ">>>",

    "<",
    ">",

    "+",
    "-",

    "++",
    "--",

    "*",
    "/",
    "%",

    ":",
    ";",

    ".",
    ",",

    "[",
    "]",

    "{",
    "}",

    "(",
    ")",

    "?",
    "??",

    "!",
    "~",
};

simplejs_composite_token_entry_t composite_token_list[sizeof(composite_list_string) / sizeof(*composite_list_string)];

simplejs_composite_token_t composite_token = {
    .maxLen = 4,
    .entries = sizeof(composite_token_list) / sizeof(*composite_token_list),
    .list = composite_token_list,
};

#define string_char '\''
#define double_string_char '\"'
#define template_string_char '`'

static char basic_string_chars[] = {
    string_char,
    double_string_char,
    template_string_char,
    '\0',
};

static char string_type_end[] = {
    [SIMPLEJS_TOKEN_STATE_STRING] = string_char,
    [SIMPLEJS_TOKEN_STATE_DOUBLE_STRING] = double_string_char,
    [SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING] = template_string_char,
};

static simplejs_token_state_t string_type_state[] = {
    [string_char] = SIMPLEJS_TOKEN_STATE_STRING,
    [double_string_char] = SIMPLEJS_TOKEN_STATE_DOUBLE_STRING,
    [template_string_char] = SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING,
};

void simplejs_initialize_composite_list()
{
    char **string_vec = composite_list_string;
    uint8_t entries = composite_token.entries;

    for (size_t i = 0; i < entries; i++)
    {
        simplejs_utf8_string_t *string = &composite_token.list[i].tokens;

        string->buffer = string_vec[i];
        string->valid_size = strlen(string->buffer);
        string->max_size = string->valid_size + 1;
    }
}

static FUNC_IS_ON_STRING(simplejs_is_keyword, keywords_list_string);
static FUNC_IS_ON_STRING(simplejs_is_expr_keyword, expr_keywords_list_string);

static bool simplejs_is_number(char *string)
{
    char c1 = string[0];
    if (c1 >= '0' && c1 <= '9')
        return true;

    return false;
}

char *simplejs_get_token_type_string(simplejs_token_type_t type)
{
    switch (type)
    {
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_IDENTIFIER);
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_KEYWORD);
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_EXPR_KEYWORD);
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_OPERATOR);
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_NUMBER);
        CASE_TO_STRING(SIMPLEJS_TOKEN_TYPE_STRING);
    }

    return "Unknown";
}

char *simplejs_get_token_state_string(simplejs_token_state_t type)
{
    switch (type)
    {
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_IDLE);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_TEMPLATE_EXPR);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_STRING);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_DOUBLE_STRING);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_ONE_LINE_COMMENT);
        CASE_TO_STRING(SIMPLEJS_TOKEN_STATE_MULTI_LINE_COMMENT);
    }

    return "Unknown";
}

static bool match_special_char_vector(char chr1)
{
    bool result = false;

    if (sizeof(special_char_vector) > chr1)
    {
        if (special_char_vector[chr1] != '\0')
            result = true;
    }

    return result;
}

static simplejs_token_ctx_t *simplejs_alloc_token_ctx()
{
    simplejs_token_ctx_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        return NULL;
    }

    memclr(ret, sizeof(*ret));

    ret->state = SIMPLEJS_TOKEN_STATE_IDLE;

    simplejs_init_list_entry(&ret->token_list, ret);

    return ret;
}

static simplejs_utf8_string_t *simplejs_create_substring(simplejs_token_ctx_t *ctx, uint64_t start, uint64_t end)
{
    uint64_t len = end - start;
    simplejs_utf8_string_t *string;

    SIMPLEJS_ASSERT(start < end);
    SIMPLEJS_ASSERT(end > start);

    size_t buffer_size = sizeof(*string) + len + 1;
    string = simplejs_hook_malloc(buffer_size);
    if (!string)
    {
        goto result;
    }

    memclr(string, buffer_size);

    string->buffer = (void *)((uint8_t *)string + sizeof(*string));
    string->max_size = buffer_size - sizeof(*string);

    for (uint64_t i = 0; i < len; i++)
    {
        simplejs_fetch_char_from_cache(ctx->cache_fetch, start + i, &string->buffer[i]);
    }

    string->valid_size = simplejs_strnlen(string->buffer, string->max_size);

result:
    return string;
}

static simplejs_status_t simplejs_process_substring(simplejs_token_ctx_t *ctx, simplejs_token_t *token)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_utf8_string_t *string = token->string;

    int utf8BufferPos = 0;
    char utf8Buffer[7] = {};
    int utf8BufferLimit = sizeof(utf8Buffer) - 1;

    char charsBuffer[4] = {};
    uint8_t charsToReplace = 0;

    bool isVariableSizeCodePoint = false;
    bool isProcessingUtf8 = false;
    uint8_t backslashChars = 0;
    size_t savedOffset = 0;

    for (size_t i = 0; i < string->valid_size; i++)
    {
        char chr1 = simplejs_safe_string_fetch(string, i);

        if (backslashChars > 0)
        {
            if (isProcessingUtf8)
            {
                chr1 = tolower(chr1);

                switch (chr1)
                {
                case '{':
                    backslashChars = -1;
                    isVariableSizeCodePoint = true;
                    break;

                case '}':
                    if (!isVariableSizeCodePoint)
                    {
                        simplejs_printf("invalid curly brace '%c'\n", chr1);

                        status = SIMPLEJS_STATUS_ENCODING_ERROR;
                        goto result;
                    }

                    if (utf8BufferPos < 1)
                    {
                        simplejs_printf("cannot have empty utf8 code point!\n");

                        status = SIMPLEJS_STATUS_ENCODING_ERROR;
                        goto result;
                    }

                    utf8BufferPos--;
                    backslashChars = 1;
                    isVariableSizeCodePoint = false;
                    break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case 'a':
                case 'b':
                case 'c':
                case 'd':
                case 'e':
                case 'f':
                    utf8Buffer[utf8BufferPos++] = chr1;
                    break;

                default:
                    simplejs_printf("unknown '%.*s' character\n", (int)((i - savedOffset) + 1), &string->buffer[savedOffset]);

                    status = SIMPLEJS_STATUS_ENCODING_ERROR;
                    goto result;
                }

                if (utf8BufferPos >= utf8BufferLimit &&
                    backslashChars > 1)
                {
                    simplejs_printf("cannot have more than %d character on utf8 buffer\n", utf8BufferLimit);

                    backslashChars = 1;

                    status = SIMPLEJS_STATUS_ENCODING_ERROR;
                    goto result;
                }
                else if (backslashChars < 2)
                {
                    utf8Buffer[utf8BufferPos + 1] = '\0';

                    uint32_t code = strtoul(utf8Buffer, NULL, 16);
                    status = simplejs_encode_utf8_char(code, &charsToReplace, &string->buffer[savedOffset]);

                    simplejs_printf("codeStr = '%.*s'\n", utf8BufferPos + 1, utf8Buffer);
                    simplejs_printf("code = 0x%06x\n", code);

                    simplejs_printf("finish\n");

                    backslashChars = 1;
                }
            }
            else
            {
                switch (chr1)
                {
                case 'u':
                    utf8BufferPos = 0;
                    isVariableSizeCodePoint = false;
                    isProcessingUtf8 = true;
                    backslashChars += 4;
                    break;

                default:
                    if (match_special_char_vector(chr1))
                    {
                        string->buffer[savedOffset] = special_char_vector[chr1];
                    }
                    else
                    {
                        simplejs_printf("unknown '%.*s' character\n", (int)((i - savedOffset) + 1), &string->buffer[savedOffset]);

                        status = SIMPLEJS_STATUS_ENCODING_ERROR;
                        goto result;
                    }
                    break;
                }
            }

            backslashChars--;

            if (backslashChars < 1)
            {
                char *stringEnd = &string->buffer[string->valid_size + 1];
                char *stringAhead = &string->buffer[i + 1];
                char *stringToMove = &string->buffer[savedOffset + charsToReplace];

                memcpy(stringToMove, stringAhead, (size_t)stringEnd - (size_t)stringAhead);

                string->valid_size = simplejs_strnlen(string->buffer, string->max_size);

                i = savedOffset - 1;
            }
        }
        else if (chr1 == '\\')
        {
            charsToReplace = 1;
            isProcessingUtf8 = false;
            backslashChars = 1;
            savedOffset = i;
        }
    }

result:
    return status;
}

typedef struct
{
    char *sufix;
    simplejs_number_type_t type;
    bool is_float;
} number_types_t;
number_types_t number_types[] = {
    {
        .sufix = "i32",
        .type = SIMPLEJS_NUMBER_TYPE_I32,
    },
    {
        .sufix = "i64",
        .type = SIMPLEJS_NUMBER_TYPE_I64,
    },
    {
        .sufix = "ui32",
        .type = SIMPLEJS_NUMBER_TYPE_UI32,
    },
    {
        .sufix = "ui64",
        .type = SIMPLEJS_NUMBER_TYPE_UI64,
    },
    {
        .sufix = "f32",
        .type = SIMPLEJS_NUMBER_TYPE_F32,
    },
    {
        .sufix = "f64",
        .type = SIMPLEJS_NUMBER_TYPE_F64,
    },
};

static int simplejs_get_number_base_by_string(char *string, int *reserve_chars)
{
    size_t len = strlen(string);
    if (len < 2)
        return 10;

    char chr1 = tolower(string[0]);
    char chr2 = tolower(string[1]);

    if (chr1 == '0')
    {
        *reserve_chars = 2;

        switch (chr2)
        {
        case '.':
            *reserve_chars = 1;
            return 10;

        case 'x':
            return 16;

        case 'o':
            return 8;

        case 'b':
            return 2;

        default:
            *reserve_chars = 1;
            return 8;
        }
    }

    return 10;
}

static bool simplejs_get_number_by_char(char character, int base, uint8_t *out)
{
    char noncase_char = tolower(character);

    switch (base)
    {
    case 16:
        if (noncase_char >= '0' && noncase_char <= '9')
        {
            *out = noncase_char - '0';
            return true;
        }

        if (noncase_char >= 'a' && noncase_char <= 'f')
        {
            *out = (noncase_char - 'a') + 10;
            return true;
        }
        break;

    case 10:
        if (noncase_char >= '0' && noncase_char <= '9')
        {
            *out = noncase_char - '0';
            return true;
        }
        break;
    case 8:
        if (noncase_char >= '0' && noncase_char <= '7')
        {
            *out = noncase_char - '0';
            return true;
        }
        break;
    case 2:
        if (noncase_char >= '0' && noncase_char <= '1')
        {
            *out = noncase_char - '0';
            return true;
        }
        break;
    }

    return false;
}

static simplejs_status_t simplejs_process_int_number(char *string, uint64_t *output)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    uint64_t ret = 0;
    uint64_t pow = 1;

    int reserve_chars = 0;
    int base = simplejs_get_number_base_by_string(string, &reserve_chars);

    for (size_t i = strlen(string); i > reserve_chars; i--)
    {
        char character = string[i - 1];

        uint8_t number;

        bool is_invalid = !simplejs_get_number_by_char(character, base, &number);
        if (is_invalid)
        {
            simplejs_printf("invalid number encoding '%c'\n", character);
            status = SIMPLEJS_STATUS_ENCODING_ERROR;
            goto result;
        }

        ret += (uint64_t)number * pow;
        pow *= base;
    }

    *output = ret;

result:
    return status;
}

static simplejs_status_t simplejs_process_float_number(char *string, double *output)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    double ret = 0;
    double pow = 1;
    int dots = 0;

    int reserve_chars = 0;
    int base = simplejs_get_number_base_by_string(string, &reserve_chars);

    {
        for (size_t i = strlen(string); i > reserve_chars; i--)
        {
            char character = string[i - 1];

            if (character == '.')
                dots++;

            if (dots > 1)
            {
                simplejs_printf("cannot have more then one dot\n");
                status = SIMPLEJS_STATUS_ENCODING_ERROR;
                goto result;
            }
        }
    }

    bool is_processing_decimal = dots == 1;

    for (size_t i = strlen(string); i > reserve_chars; i--)
    {
        char character = string[i - 1];

        if (character == '.')
        {
            ret /= pow;
            pow = 1;

            is_processing_decimal = false;
            continue;
        }

        uint8_t number;

        bool is_invalid = !simplejs_get_number_by_char(character, base, &number);
        if (is_invalid)
        {
            simplejs_printf("invalid number encoding '%c'\n", character);
            status = SIMPLEJS_STATUS_ENCODING_ERROR;
            goto result;
        }

        ret += (double)number * pow;
        pow *= base;
    }

    *output = ret;

result:
    return status;
}

static simplejs_status_t simplejs_process_number(simplejs_token_ctx_t *ctx, simplejs_token_t *token)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_utf8_string_t *string = token->string;
    simplejs_number_t output = {0};

    size_t max_len = 0;
    simplejs_number_type_t best_type = SIMPLEJS_NUMBER_TYPE_DEFAULT;
    char *type_sufix = NULL;
    bool is_float = false;

    for (size_t i = 0; i < sizeof(number_types) / sizeof(*number_types); i++)
    {
        number_types_t *entry = &number_types[i];
        size_t len = strlen(entry->sufix);
        if (len <= max_len)
            continue;

        if (!simplejs_strcasecmp(string->buffer + (string->valid_size - len), entry->sufix))
        {
            type_sufix = entry->sufix;
            best_type = entry->type;
            max_len = len;
        }
    }

    string->buffer[string->valid_size - (type_sufix ? strlen(type_sufix) : 0)] = '\0';

    switch (best_type)
    {
    case SIMPLEJS_NUMBER_TYPE_F32:
    case SIMPLEJS_NUMBER_TYPE_F64:
        is_float = true;
        break;

    default:
        break;
    }

    output.type = best_type;

    if (is_float)
    {
        double value;

        status = simplejs_process_float_number(string->buffer, &value);
        if (!SIMPLEJS_SUCCESS(status))
            goto result;

        simplejs_printf("(double) %f\n", value);

        switch (best_type)
        {
        case SIMPLEJS_NUMBER_TYPE_F32:
            output.value.f32 = (float)value;
            break;

        case SIMPLEJS_NUMBER_TYPE_F64:
            output.value.f64 = (double)value;
            break;

        default:
            break;
        }
    }
    else
    {
        uint64_t value;

        status = simplejs_process_int_number(string->buffer, &value);
        if (!SIMPLEJS_SUCCESS(status))
            goto result;

        simplejs_printf("(uint64_t) %lu\n", value);

        switch (best_type)
        {
        case SIMPLEJS_NUMBER_TYPE_I32:
        case SIMPLEJS_NUMBER_TYPE_UI32:
            output.value.ui32 = (uint32_t)value;
            break;

        case SIMPLEJS_NUMBER_TYPE_I64:
        case SIMPLEJS_NUMBER_TYPE_UI64:
            output.value.ui64 = (uint64_t)value;
            break;

        default:
            break;
        }
    }

    token->number = output;

result:
    return status;
}

static simplejs_status_t simplejs_add_token(simplejs_token_ctx_t *ctx, simplejs_token_type_t type, void *forced_pointer)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_t *token = NULL;
    simplejs_utf8_string_t *string = NULL;
    if (!ctx->len && !forced_pointer)
    {
        simplejs_printf("cannot add token with empty content\n");
        goto result;
    }

    token = simplejs_hook_malloc(sizeof(*token));
    if (!token)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(token, sizeof(*token));

    simplejs_init_list_entry(&token->list_entry, token);

    simplejs_init_list_entry(&token->_arg_list_entry, token);

    token->offset.start = ctx->offset;
    token->offset.end = ctx->offset + ctx->len;

    token->type = type;
    if (forced_pointer)
    {
        token->string = forced_pointer;
    }
    else
    {
        string = simplejs_create_substring(ctx, ctx->offset, ctx->offset + ctx->len);
        if (!string)
        {
            status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
            goto result;
        }

        token->string = string;

        if (token->type == SIMPLEJS_TOKEN_TYPE_STRING)
        {
            status = simplejs_process_substring(ctx, token);
            if (!SIMPLEJS_SUCCESS(status))
            {
                goto result;
            }
        }
    }

    if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
    {
        simplejs_utf8_string_t *string = token->string;

        if (simplejs_is_keyword(string->buffer))
            token->type = SIMPLEJS_TOKEN_TYPE_KEYWORD;

        if (simplejs_is_expr_keyword(string->buffer))
            token->type = SIMPLEJS_TOKEN_TYPE_EXPR_KEYWORD;

        if (simplejs_is_number(string->buffer))
            token->type = SIMPLEJS_TOKEN_TYPE_NUMBER;
    }

    if (token->type == SIMPLEJS_TOKEN_TYPE_EXPR_KEYWORD)
    {
        bool is_true = !strcmp(string->buffer, "true");

        simplejs_utf8_string_t *string = token->string;

        if (is_true ||
            !strcmp(string->buffer, "false"))
        {
            token->type = SIMPLEJS_TOKEN_TYPE_NUMBER;
            token->number.type = SIMPLEJS_NUMBER_TYPE_BOOLEAN;
            token->number.value.boolean = is_true;
        }
    }
    else if (token->type == SIMPLEJS_TOKEN_TYPE_NUMBER)
    {
        status = simplejs_process_number(ctx, token);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }
    }

    simplejs_insert_tail_list(&ctx->token_list, &token->list_entry);

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (token)
            simplejs_hook_mfree(token);

        if (string)
            simplejs_hook_mfree(string);
    }

    return status;
}

void simplejs_reset_position(simplejs_token_ctx_t *ctx)
{
    ctx->offset = ctx->index + 1;
    ctx->len = 0;
}

void simplejs_reset_state(simplejs_token_ctx_t *ctx)
{
    simplejs_reset_position(ctx);
    ctx->state = SIMPLEJS_TOKEN_STATE_IDLE;
}

void SIMPLEJS_API simplejs_tokenize_dump_tokens(simplejs_token_ctx_t *ctx)
{
    simplejs_list_entry_t *end = &ctx->token_list;
    simplejs_list_entry_t *current = end->next;

    while (current != end)
    {
        simplejs_list_entry_t *next = current->next;
        simplejs_token_t *token = simplejs_get_list_entry_structure(current);

        simplejs_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
        if (token->type == SIMPLEJS_TOKEN_TYPE_NUMBER)
        {
            simplejs_printf("token->number = ");

            bool is_signed = token->number.type < SIMPLEJS_NUMBER_TYPE_UI32;

            switch (token->number.type)
            {
            case SIMPLEJS_NUMBER_TYPE_I32:
            case SIMPLEJS_NUMBER_TYPE_UI32:
                if (is_signed)
                    simplejs_printf("%d", token->number.value.i32);
                else
                    simplejs_printf("%u", token->number.value.ui32);
                break;

            case SIMPLEJS_NUMBER_TYPE_I64:
            case SIMPLEJS_NUMBER_TYPE_UI64:
                if (is_signed)
                    simplejs_printf("%ld", token->number.value.i64);
                else
                    simplejs_printf("%lu", token->number.value.ui64);
                break;

            case SIMPLEJS_NUMBER_TYPE_F32:
                simplejs_printf("%f", token->number.value.f32);
                break;

            case SIMPLEJS_NUMBER_TYPE_F64:
                simplejs_printf("%f", token->number.value.f64);
                break;

            default:
                simplejs_printf("(Unknown type) %u", token->number.type);
                break;
            }

            simplejs_printf("\n");
        }
        simplejs_printf("token->string = \"%s\"\n", token->string->buffer);

        current = next;
    }
}

void SIMPLEJS_API simplejs_free_token_ctx(simplejs_token_ctx_t *ctx)
{
    SIMPLEJS_ASSERT(ctx != NULL);

    simplejs_list_entry_t *end = &ctx->token_list;
    simplejs_list_entry_t *current = end->next;

    while (current != end)
    {
        simplejs_list_entry_t *next = current->next;
        simplejs_token_t *token = simplejs_get_list_entry_structure(current);

        simplejs_hook_mfree(token->string);
        simplejs_hook_mfree(token);

        current = next;
    }

    simplejs_hook_mfree(ctx);
}

#define simplejs_tokenize_flush_identifier(label, status) SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_token(ctx, SIMPLEJS_TOKEN_TYPE_IDENTIFIER, NULL), label, status)

simplejs_status_t SIMPLEJS_API simplejs_tokenize(simplejs_linemap_ctx_t *linemap_ctx, simplejs_token_ctx_t **out)
{
    SIMPLEJS_ASSERT(linemap_ctx != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_ctx_t *ctx = NULL;

    ctx = simplejs_alloc_token_ctx();
    if (!ctx)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    ctx->linemap_ctx = linemap_ctx;
    ctx->state = SIMPLEJS_TOKEN_STATE_IDLE;

    ctx->cache_fetch = &ctx->linemap_ctx->cache_fetch;

    for (ctx->index = 0; simplejs_fetch_char_from_cache(ctx->cache_fetch, ctx->index, NULL); ctx->index++)
    {
        char str4[] = {
            '\0',
            '\0',
            '\0',
            '\0',
            '\0',
        };
        for (int i = 0; i < sizeof(str4) - 1; i++)
            simplejs_fetch_char_from_cache(ctx->cache_fetch, ctx->index + (uint64_t)i, &str4[i]);

        char chr1 = str4[0];

        if (string_includes(useless_chars, chr1))
        {
            simplejs_reset_position(ctx);
            continue;
        }

        bool isTemplateString = ctx->state == SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING;
        bool isTemplateExpr = ctx->state == SIMPLEJS_TOKEN_STATE_TEMPLATE_EXPR;

        switch (ctx->state)
        {
        case SIMPLEJS_TOKEN_STATE_IDLE:
        case SIMPLEJS_TOKEN_STATE_TEMPLATE_EXPR:
            if (string_includes(basic_string_chars, chr1))
            {
                simplejs_tokenize_flush_identifier(result, status);
                simplejs_reset_position(ctx);

                ctx->state = string_type_state[chr1];
                break;
            }

            if (!strncmp(str4, oneline_comment_operators, 2))
            {
                simplejs_tokenize_flush_identifier(result, status);

                ctx->index++;
                simplejs_reset_position(ctx);

                ctx->state = SIMPLEJS_TOKEN_STATE_ONE_LINE_COMMENT;
                break;
            }

            if (!strncmp(str4, multiline_comment_operators_start, 2))
            {
                simplejs_tokenize_flush_identifier(result, status);

                ctx->index++;
                simplejs_reset_position(ctx);

                ctx->state = SIMPLEJS_TOKEN_STATE_MULTI_LINE_COMMENT;
                break;
            }

            if (!ctx->len &&
                chr1 >= '0' && chr1 <= '9')
            {
                ctx->index--;

                simplejs_tokenize_flush_identifier(result, status);
                simplejs_reset_position(ctx);

                ctx->state = SIMPLEJS_TOKEN_STATE_NUMBER;
                break;
            }

            if (isTemplateExpr)
            {
                if (chr1 == curly_brace_left)
                    ctx->template_expr_level++;

                if (chr1 == curly_brace_right)
                    ctx->template_expr_level--;

                if (ctx->template_expr_level < 1)
                {
                    simplejs_tokenize_flush_identifier(result, status);

                    simplejs_reset_position(ctx);

                    ctx->state = SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING;
                    break;
                }
            }

            if (string_includes(valid_operators, chr1))
            {
                bool exitCompositeLoop = false;

                simplejs_tokenize_flush_identifier(result, status);
                simplejs_reset_position(ctx);

                for (uint8_t c = composite_token.maxLen; c > 0; c--)
                {
                    for (uint8_t i = 0; i < composite_token.entries; i++)
                    {
                        simplejs_composite_token_entry_t *composite_entry = &composite_token.list[i];
                        simplejs_utf8_string_t *composite_entry_string = &composite_entry->tokens;

                        if (composite_entry_string->valid_size != c)
                            continue;

                        if (!strncmp(str4, composite_entry_string->buffer, c))
                        {
                            ctx->offset = ctx->index;
                            ctx->len = c;

                            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_token(ctx, SIMPLEJS_TOKEN_TYPE_OPERATOR, NULL), result, status);

                            ctx->index += c - 1;
                            simplejs_reset_position(ctx);

                            exitCompositeLoop = true;
                            break;
                        }
                    }

                    if (exitCompositeLoop)
                        break;
                }

                if (exitCompositeLoop)
                    break;
            }

            if (string_includes(separators_chars, chr1))
            {
                simplejs_tokenize_flush_identifier(result, status);
                simplejs_reset_position(ctx);
                break;
            }

            ctx->len++;
            break;

        case SIMPLEJS_TOKEN_STATE_NUMBER:
        {
            char chr1_low = tolower(chr1);

            if (!((chr1_low == '.') ||
                  (chr1_low == 'u') ||
                  (chr1_low == 'i') ||
                  (chr1_low == 'x') ||
                  (chr1_low == 'o') ||
                  (chr1_low == 'b') ||
                  (chr1_low >= '0' && chr1_low <= '9') ||
                  (chr1_low >= 'a' && chr1_low <= 'f')))
            {
                ctx->index--;

                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_token(ctx, SIMPLEJS_TOKEN_TYPE_NUMBER, NULL), result, status);
                simplejs_reset_state(ctx);
                break;
            }

            ctx->len++;
            break;
        }

        case SIMPLEJS_TOKEN_STATE_STRING:
        case SIMPLEJS_TOKEN_STATE_DOUBLE_STRING:
        case SIMPLEJS_TOKEN_STATE_TEMPLATE_STRING:
        {
            if (chr1 == string_type_end[ctx->state])
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_token(ctx, SIMPLEJS_TOKEN_TYPE_STRING, NULL), result, status);
                simplejs_reset_state(ctx);
                break;
            }

            if (isTemplateString && !strncmp(str4, template_expr_start, 2))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_token(ctx, SIMPLEJS_TOKEN_TYPE_STRING, NULL), result, status);

                ctx->index++;
                simplejs_reset_position(ctx);

                ctx->state = SIMPLEJS_TOKEN_STATE_TEMPLATE_EXPR;
                ctx->template_expr_level = 1;
                break;
            }

            ctx->len++;
            break;
        }

        case SIMPLEJS_TOKEN_STATE_ONE_LINE_COMMENT:
        {
            if (chr1 == '\n')
            {
                simplejs_reset_state(ctx);
                break;
            }

            ctx->len++;
            break;
        }

        case SIMPLEJS_TOKEN_STATE_MULTI_LINE_COMMENT:
        {
            if (!strncmp(str4, multiline_comment_operators_end, 2))
            {
                ctx->index++;

                simplejs_reset_state(ctx);
                break;
            }

            ctx->len++;
            break;
        }

        default:
            SIMPLEJS_ASSERT(ctx->state == 0);
            break;
        }
    }

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (linemap_ctx)
        {
            if (ctx)
                ctx->linemap_ctx = NULL;

            simplejs_free_linemap_ctx(linemap_ctx);
        }

        if (ctx)
            simplejs_free_token_ctx(ctx);
    }
    else
    {
        *out = ctx;
    }

    simplejs_printf("Finished simplejs_tokenizer function with %s\n", simplejs_get_status_string(status));
    return status;
}
