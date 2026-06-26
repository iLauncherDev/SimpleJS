#include <parser.h>

simplejs_status_t simplejs_alloc_ast_node(simplejs_ast_node_type_t type, simplejs_ast_node_t **out);
void simplejs_insert_children_ast_to_parent(simplejs_ast_node_t *parent, simplejs_ast_node_t *children);
bool simplejs_check_token_expr_keyword(simplejs_token_t *token, char *value);
bool simplejs_check_token_keyword(simplejs_token_t *token, char *value);
bool simplejs_check_token_operator(simplejs_token_t *token, char *value);

bool simplejs_get_function_output(
    simplejs_parser_ctx_t *parser_ctx, simplejs_utf8_string_t *name, simplejs_utf8_string_t **out);

bool simplejs_get_scoped_output(
    simplejs_parser_ctx_t *parser_ctx, void *context, void *out,
    bool (*callback)(simplejs_ast_scope_context_t *scope_context, void *context, void *out));

void simplejs_present_parser_diagnostic(
    simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *ast_node, simplejs_token_t *target_token,
    char *type, char *message);

int simplejs_parse_expression_level = 0;

typedef struct local_scoped
{
    bool is_function, is_argument;

    uint32_t reference;
    simplejs_utf8_string_t *string;
} local_scoped_t;

typedef struct operators_type_node
{
    char *operator;
    simplejs_ast_node_type_t node_type;
    bool support_assign_first;
} operators_type_node_t;

operators_type_node_t unary_operators_type_node[] = {
    {
        .operator = "++",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_INC,
        .support_assign_first = true,
    },
    {
        .operator = "--",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_DEC,
        .support_assign_first = true,
    },

    {
        .operator = "~",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_NOT,
    },
    {
        .operator = "-",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_NEG,
    },
};

operators_type_node_t binary_operators_type_node[] = {
    {
        .operator = ",",
        .node_type = SIMPLEJS_AST_NODE_TYPE_COMMA_OPERATOR,
    },

    {
        .operator = ".",
        .node_type = SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS,
    },
    {
        .operator = "=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ASSIGN,
    },

    {
        .operator = "[",
        .node_type = SIMPLEJS_AST_NODE_TYPE_EXPRESSION_PROPERTY_ACCESS,
    },

    {
        .operator = "||",
        .node_type = SIMPLEJS_AST_NODE_TYPE_LOGICAL_OR,
    },
    {
        .operator = "&&",
        .node_type = SIMPLEJS_AST_NODE_TYPE_LOGICAL_AND,
    },

    {
        .operator = "==",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL,
    },
    {
        .operator = "!=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_NOT_EQUAL,
    },
    {
        .operator = ">",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_GREATER,
    },
    {
        .operator = "<",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_BELOW,
    },

    {
        .operator = ">=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_GREATER_EQUAL,
    },
    {
        .operator = "<=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_BELOW_EQUAL,
    },

    {
        .operator = "|",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_OR,
    },
    {
        .operator = "&",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_AND,
    },

    {
        .operator = "<<",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SHL,
    },
    {
        .operator = ">>",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SHR,
    },
    {
        .operator = "<<<",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SAL,
    },
    {
        .operator = ">>>",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SAR,
    },

    {
        .operator = "+",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_ADD,
    },
    {
        .operator = "-",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SUB,
    },
    {
        .operator = "*",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_MUL,
    },
    {
        .operator = "/",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_DIV,
    },
    {
        .operator = "%",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_MOD,
    },
    {
        .operator = "(",
        .node_type = SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL,
    },
};

void _simplejs_parser_printf(char *file, int line, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (int i = 1; i < simplejs_parse_expression_level; i++)
    {
        simplejs_printf_ex(file, line, "\t");
    }

    simplejs_vprintf_ex(file, line, fmt, args);

    va_end(args);
}

#define simplejs_parser_printf(fmt, ...) _simplejs_parser_printf(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

static simplejs_ast_node_type_t simplejs_get_node_type_for_unary_operator(simplejs_utf8_string_t *string, int string_cut, bool assign_first)
{
    int len = string->valid_size - string_cut;

    for (size_t i = 0; i < sizeof(unary_operators_type_node) / sizeof(*unary_operators_type_node); i++)
    {
        operators_type_node_t *entry = &unary_operators_type_node[i];

        if (strlen(entry->operator) == len &&
            !strncmp(entry->operator, string->buffer, len))
        {
            if (assign_first && (entry->support_assign_first != assign_first))
                continue;

            return entry->node_type;
        }
    }

    return SIMPLEJS_AST_NODE_TYPE_UNARY_OPERATOR;
}

static simplejs_ast_node_type_t simplejs_get_node_type_for_binary_operator(simplejs_utf8_string_t *string, int string_cut)
{
    int len = string->valid_size - string_cut;

    for (size_t i = 0; i < sizeof(binary_operators_type_node) / sizeof(*binary_operators_type_node); i++)
    {
        operators_type_node_t *entry = &binary_operators_type_node[i];

        if (strlen(entry->operator) == len &&
            !strncmp(entry->operator, string->buffer, len))
        {
            return entry->node_type;
        }
    }

    return SIMPLEJS_AST_NODE_TYPE_BINARY_OPERATOR;
}

static simplejs_status_t simplejs_clone_ast_node(simplejs_ast_node_t *node, simplejs_ast_node_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *cloned_parent;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(node->type, &cloned_parent), result, status);

    cloned_parent->context = node->context;
    cloned_parent->flags = node->flags;

    simplejs_list_entry_t *end_ast = &node->children_list_entry;
    simplejs_list_entry_t *current_ast = &node->children_list_entry;

    while (current_ast != end_ast)
    {
        simplejs_ast_node_t *child_to_clone = simplejs_get_list_entry_structure(current_ast);
        simplejs_ast_node_t *cloned_child;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_clone_ast_node(child_to_clone, &cloned_child), result, status);

        simplejs_insert_children_ast_to_parent(cloned_parent, cloned_child);

        current_ast = current_ast->next;
    }

    *out = cloned_parent;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (cloned_parent)
            simplejs_free_ast_list(cloned_parent);
    }

    return status;
}

static simplejs_status_t simplejs_make_unary_node(simplejs_token_t *operator, simplejs_ast_node_type_t node_type, bool assign_first, simplejs_ast_node_t **out, simplejs_ast_node_t *left)
{
    simplejs_ast_node_t *unary_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(node_type, &unary_ast), result, status);
    unary_ast->context = operator->string;
    unary_ast->flags = assign_first;

    SIMPLEJS_ASSERT(left != NULL);

    simplejs_insert_children_ast_to_parent(unary_ast, left);

    *out = unary_ast;

result:
    return status;
}

static simplejs_status_t simplejs_make_binary_node(simplejs_token_t *operator, simplejs_ast_node_type_t node_type, simplejs_ast_node_t **out, simplejs_ast_node_t *left, simplejs_ast_node_t *right)
{
    simplejs_ast_node_t *binary_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(node_type, &binary_ast), result, status);
    binary_ast->context = operator->string;

    SIMPLEJS_ASSERT(left != NULL);
    SIMPLEJS_ASSERT(right != NULL);

    simplejs_insert_children_ast_to_parent(binary_ast, left);
    simplejs_insert_children_ast_to_parent(binary_ast, right);

    *out = binary_ast;

result:
    return status;
}

static simplejs_token_t *simplejs_token_next(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token)
{
    simplejs_token_t *token = simplejs_get_list_entry_structure(*start_token);

    (*start_token) = (*start_token)->next;
    return token;
}

static simplejs_token_t *simplejs_token_prev(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token)
{
    simplejs_token_t *token = simplejs_get_list_entry_structure(*start_token);

    (*start_token) = (*start_token)->prev;
    return token;
}

static simplejs_token_t *simplejs_token_peek(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token)
{
    simplejs_token_t *token = simplejs_get_list_entry_structure(*start_token);
    return token;
}

static bool simplejs_get_scoped_reference_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out)
{
    simplejs_utf8_string_t *string = context;

    local_scoped_t *out_struct = out;
    memclr(out_struct, sizeof(*out_struct));

    {
        simplejs_list_entry_t *end_var = &scope_context->var_list_entry;
        simplejs_list_entry_t *current_var = end_var->next;

        while (current_var != end_var)
        {
            simplejs_ast_var_context_t *var = simplejs_get_list_entry_structure(current_var);

            if (!strcmp((char *)var->name->buffer, (char *)string->buffer))
            {
                out_struct->reference = var->index;
                return true;
            }

            current_var = current_var->next;
        }
    }

    {
        simplejs_list_entry_t *end_function = &scope_context->function_list_entry;
        simplejs_list_entry_t *current_function = end_function->next;

        while (current_function != end_function)
        {
            simplejs_ast_function_context_t *function = simplejs_get_list_entry_structure(current_function);
            if (!function->name)
                goto skip_scoped_function;

            if (!strcmp((char *)function->name->buffer, (char *)string->buffer))
            {
                out_struct->is_function = true;
                out_struct->string = function->name;
                return true;
            }

        skip_scoped_function:
            current_function = current_function->next;
        }
    }

    {
        simplejs_list_entry_t *end_arg = &scope_context->arg_list_entry;
        simplejs_list_entry_t *current_arg = end_arg->next;

        while (current_arg != end_arg)
        {
            simplejs_token_t *arg = simplejs_get_list_entry_structure(current_arg);

            if (!strcmp((char *)arg->string->buffer, (char *)string->buffer))
            {
                out_struct->is_argument = true;
                out_struct->reference = arg->_arg_index;
                return true;
            }

            current_arg = current_arg->next;
        }
    }

    return false;
}

simplejs_status_t simplejs_alloc_identifier_node(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t **out, simplejs_token_t *token, bool is_property)
{
    simplejs_utf8_string_t *function_name;

    local_scoped_t local_scoped;
    bool is_local = simplejs_get_scoped_output(parser_ctx, token->string, &local_scoped, simplejs_get_scoped_reference_callback);
    bool is_local_function_fallback = simplejs_get_function_output(parser_ctx, token->string, &function_name);

    simplejs_ast_node_t *identifier_ast = NULL;
    simplejs_ast_node_t *global_ast = NULL;
    simplejs_ast_node_t *property_ast = NULL;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (is_property)
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE, &identifier_ast), result, status);

        identifier_ast->context = token->string;
    }
    else if (is_local)
    {
        if (local_scoped.is_function)
        {
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE, &identifier_ast), result, status);

            identifier_ast->context = local_scoped.string;
        }
        else
        {
            simplejs_ast_node_type_t node_type = SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE;
            if (local_scoped.is_argument)
                node_type = SIMPLEJS_AST_NODE_TYPE_ARGUMENT_REFERENCE;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(node_type, &identifier_ast), result, status);

            identifier_ast->context = (void *)((uintptr_t)local_scoped.reference);
        }
    }
    else if (is_local_function_fallback)
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE, &identifier_ast), result, status);

        identifier_ast->context = function_name;
    }
    else
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE, &global_ast), result, status);
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE, &property_ast), result, status);
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_make_binary_node(token, SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS, &identifier_ast, global_ast, property_ast), result, status);

        property_ast->context = token->string;
    }

    *out = identifier_ast;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (identifier_ast)
            simplejs_hook_mfree(identifier_ast);

        if (global_ast)
            simplejs_hook_mfree(global_ast);

        if (property_ast)
            simplejs_hook_mfree(property_ast);
    }

    return status;
}

simplejs_status_t simplejs_alloc_string_node(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t **out, simplejs_token_t *token)
{
    simplejs_ast_node_t *string_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_STRING, &string_ast), result, status);

    string_ast->context = token->string;

    *out = string_ast;

result:
    return status;
}

simplejs_status_t simplejs_alloc_number_node(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t **out, simplejs_token_t *token)
{
    simplejs_ast_node_t *number_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_NUMBER, &number_ast), result, status);

    number_ast->context = &token->number;

    *out = number_ast;

result:
    return status;
}

static void simplejs_get_binding_power(simplejs_token_t *operator, int *lbp, int *rbp)
{
    if (simplejs_check_token_operator(operator, ","))
    {
        *lbp = 0;
        *rbp = 1;
        return;
    }

    if (simplejs_check_token_operator(operator, "=") ||
        simplejs_check_token_operator(operator, "+=") ||
        simplejs_check_token_operator(operator, "-=") ||
        simplejs_check_token_operator(operator, "*=") ||
        simplejs_check_token_operator(operator, "/=") ||
        simplejs_check_token_operator(operator, "%=") ||
        simplejs_check_token_operator(operator, "<<=") ||
        simplejs_check_token_operator(operator, ">>=") ||
        simplejs_check_token_operator(operator, "<<<=") ||
        simplejs_check_token_operator(operator, ">>>="))
    {
        *lbp = 10;
        *rbp = 9;
        return;
    }

    if (simplejs_check_token_operator(operator, "&&"))
    {
        *lbp = 20;
        *rbp = 21;
        return;
    }

    if (simplejs_check_token_operator(operator, "||"))
    {
        *lbp = 30;
        *rbp = 31;
        return;
    }

    if (simplejs_check_token_operator(operator, "|"))
    {
        *lbp = 40;
        *rbp = 41;
        return;
    }

    if (simplejs_check_token_operator(operator, "^"))
    {
        *lbp = 50;
        *rbp = 51;
        return;
    }

    if (simplejs_check_token_operator(operator, "&"))
    {
        *lbp = 60;
        *rbp = 61;
        return;
    }

    if (simplejs_check_token_operator(operator, "==") ||
        simplejs_check_token_operator(operator, "!="))
    {
        *lbp = 70;
        *rbp = 71;
        return;
    }

    if (simplejs_check_token_operator(operator, "<=") ||
        simplejs_check_token_operator(operator, ">=") ||
        simplejs_check_token_operator(operator, "<") ||
        simplejs_check_token_operator(operator, ">"))
    {
        *lbp = 80;
        *rbp = 81;
        return;
    }

    if (simplejs_check_token_operator(operator, "+") ||
        simplejs_check_token_operator(operator, "-"))
    {
        *lbp = 90;
        *rbp = 91;
        return;
    }

    if (simplejs_check_token_operator(operator, "*") ||
        simplejs_check_token_operator(operator, "/") ||
        simplejs_check_token_operator(operator, "%"))
    {
        *lbp = 100;
        *rbp = 101;
        return;
    }

    if (simplejs_check_token_operator(operator, "++") ||
        simplejs_check_token_operator(operator, "--"))
    {
        *lbp = 110;
        *rbp = 111;
        return;
    }

    if (simplejs_check_token_operator(operator, "(") ||
        simplejs_check_token_operator(operator, "["))
    {
        *lbp = 120;
        *rbp = 121;
        return;
    }

    if (simplejs_check_token_operator(operator, "."))
    {
        *lbp = 130;
        *rbp = 131;
        return;
    }

    *lbp = -1;
    *rbp = 0;
}

static simplejs_status_t simplejs_nud(
    simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token, simplejs_ast_node_t **out, simplejs_token_t *token,
    char **end_operators, int end_operators_size)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
    {
        status = simplejs_alloc_identifier_node(parser_ctx, out, token, false);
        goto result;
    }

    if (token->type == SIMPLEJS_TOKEN_TYPE_NUMBER)
    {
        status = simplejs_alloc_number_node(parser_ctx, out, token);
        goto result;
    }

    if (token->type == SIMPLEJS_TOKEN_TYPE_STRING)
    {
        status = simplejs_alloc_string_node(parser_ctx, out, token);
        goto result;
    }

    if (simplejs_check_token_expr_keyword(token, "globalThis"))
    {
        status = simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE, out);
        goto result;
    }

    if (simplejs_check_token_operator(token, "("))
    {
        simplejs_parser_printf("starting sub-expression\n");

        char *end_operators[] = {
            ")",
        };

        status = simplejs_parse_expression(parser_ctx, start_token, out, 0, end_operators, sizeof(end_operators));
        simplejs_token_next(parser_ctx, start_token);

        simplejs_parser_printf("ending sub-expression\n");
        goto result;
    }

    if (simplejs_check_token_operator(token, "++") ||
        simplejs_check_token_operator(token, "--"))
    {
        simplejs_ast_node_type_t node_type =
            simplejs_check_token_operator(token, "++") ? SIMPLEJS_AST_NODE_TYPE_ALU_INC : SIMPLEJS_AST_NODE_TYPE_ALU_DEC;

        simplejs_ast_node_t *right;
        simplejs_token_t *right_token = simplejs_token_next(parser_ctx, start_token);

        status = simplejs_nud(parser_ctx, start_token, &right, right_token, end_operators, end_operators_size);
        if (!SIMPLEJS_SUCCESS(status))
            goto result;

        status = simplejs_make_unary_node(token, node_type, false, out, right);
        goto result;
    }

    if (simplejs_check_token_operator(token, "-") ||
        simplejs_check_token_operator(token, "~"))
    {
        simplejs_ast_node_type_t node_type = simplejs_get_node_type_for_unary_operator(token->string, 0, false);

        simplejs_ast_node_t *right;
        simplejs_token_t *right_token = simplejs_token_next(parser_ctx, start_token);

        status = simplejs_nud(parser_ctx, start_token, &right, right_token, end_operators, end_operators_size);
        if (!SIMPLEJS_SUCCESS(status))
            goto result;

        status = simplejs_make_unary_node(token, node_type, false, out, right);
        goto result;
    }

    // if (simplejs_check_token_operator(token, "~") ||
    //     simplejs_check_token_operator(token, "-") ||
    //     simplejs_check_token_operator(token, "!"))
    //{
    //     int rbp = 100;
    //
    //    simplejs_ast_node_t *operand = parse_expression(parser_ctx, start_token, rbp);
    //
    //    return make_unary_node(token, operand);
    //}

    simplejs_parser_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
    simplejs_parser_printf("token->string = \"%s\"\n", token->string->buffer);

    simplejs_present_parser_diagnostic(
        parser_ctx, parser_ctx->current_ast_stack, token,
        SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "unexpected token in expression");

result:
    return status;
}

static char *expression_operators[] = {
    "=",
    "++",
    "--",
    "~",
    "<=",
    ">=",
    "==",
    "!=",
    ">",
    "<",
    "|",
    "&",
    "||",
    "&&",
    "<<",
    ">>",
    "<<<",
    ">>>",
    "+",
    "-",
    "*",
    "/",
    "%",
    ".",
    ",",
    "(",
    "[",
};

static char *expression_assign_operators[] = {
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",

    "<<=",
    ">>=",
    "<<<=",
    ">>>=",
};

#define PARSER_FUNC_IS_ON_STRING(name, vector)                        \
    bool name(char *string, int string_cut)                           \
    {                                                                 \
        int len = strlen(string) - string_cut;                        \
        for (size_t i = 0; i < sizeof(vector) / sizeof(*vector); i++) \
        {                                                             \
            if (strlen(vector[i]) == len &&                           \
                !strncmp(vector[i], string, len))                     \
                return true;                                          \
        }                                                             \
        return false;                                                 \
    }

PARSER_FUNC_IS_ON_STRING(__is_operator, expression_operators)
PARSER_FUNC_IS_ON_STRING(__is_assign_operator, expression_assign_operators)

static bool is_assign_operator(simplejs_token_t *operator, int string_cut)
{
    return __is_assign_operator(operator->string->buffer, string_cut);
}

static bool is_operator(simplejs_parser_ctx_t *parser_ctx, simplejs_token_t *operator)
{
    simplejs_parser_printf("operator->type = %s\n", simplejs_get_token_type_string(operator->type));
    simplejs_parser_printf("operator->string = \"%s\"\n", operator->string->buffer);

    if (operator->type != SIMPLEJS_TOKEN_TYPE_OPERATOR)
        return false;

    simplejs_parser_printf("is_operator = %u\n", __is_operator(operator->string->buffer, 0));

    return __is_operator(operator->string->buffer, 0) | is_assign_operator(operator, 0);
}

simplejs_status_t simplejs_process_operator(
    simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token,
    simplejs_ast_node_t **left, simplejs_token_t *operator, int rbp,
    char **end_operators, int end_operators_size,
    int string_cut)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_ast_node_t *right = NULL;

    simplejs_ast_node_type_t unary_node_type = simplejs_get_node_type_for_unary_operator(operator->string, string_cut, true);
    simplejs_ast_node_type_t binary_node_type = simplejs_get_node_type_for_binary_operator(operator->string, string_cut);

    bool is_unary = unary_node_type != SIMPLEJS_AST_NODE_TYPE_UNARY_OPERATOR;
    simplejs_ast_node_type_t node_type = is_unary ? unary_node_type : binary_node_type;

    simplejs_parser_printf("node_type = %u\n", node_type);

    if (!is_unary)
    {
        switch (node_type)
        {
        case SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS:
        {
            simplejs_token_t *prop = simplejs_token_next(parser_ctx, start_token);

            if (prop->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_present_parser_diagnostic(
                    parser_ctx, parser_ctx->current_ast_stack, prop,
                    SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid format");

                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }

            status = simplejs_alloc_identifier_node(parser_ctx, &right, prop, true);

            if (!SIMPLEJS_SUCCESS(status))
                goto result;

            status = simplejs_make_binary_node(operator, node_type, left, *left, right);
            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_EXPRESSION_PROPERTY_ACCESS:
        {
            node_type = SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS;

            char *end_operators[] = {
                "]",
            };

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_parse_expression(parser_ctx, start_token, &right, 0, end_operators, sizeof(end_operators)), result, status);

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_make_binary_node(operator, node_type, left, *left, right), result, status);

            simplejs_token_next(parser_ctx, start_token);
            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL:
        {
            simplejs_parser_printf("processing function!\n");

            char *end_operators[] = {
                ",",
                ")",
            };

            status = simplejs_make_unary_node(operator, node_type, false, left, *left);
            if (!SIMPLEJS_SUCCESS(status))
                goto result;

            while (true)
            {
                simplejs_token_t *operator;

                operator = simplejs_token_next(parser_ctx, start_token);
                if (simplejs_check_token_operator(operator, ")"))
                    break;
                simplejs_token_prev(parser_ctx, start_token);

                simplejs_ast_node_t *arg;

                status = simplejs_parse_expression(parser_ctx, start_token, &arg, 0, end_operators, sizeof(end_operators));
                if (!SIMPLEJS_SUCCESS(status))
                    goto result;

                simplejs_insert_children_ast_to_parent(*left, arg);

                operator = simplejs_token_next(parser_ctx, start_token);
                if (simplejs_check_token_operator(operator, ")"))
                    break;

                if (!simplejs_check_token_operator(operator, ","))
                {
                    char message[256] = {0};
                    snprintf(message, sizeof(message) - 1, "invalid call argument separator! ('%s')", operator->string->buffer);

                    simplejs_present_parser_diagnostic(
                        parser_ctx, parser_ctx->current_ast_stack, operator,
                        SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, message);

                    status = SIMPLEJS_STATUS_INVALID_TOKEN;
                    goto result;
                }
            }

            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_COMMA_OPERATOR:
        {
            simplejs_parser_printf("starting separated-expression\n");

            status = simplejs_parse_expression(parser_ctx, start_token, &right, 0, end_operators, end_operators_size);
            if (!SIMPLEJS_SUCCESS(status))
            {
                simplejs_parser_printf("expression error\n");
                goto result;
            }

            simplejs_parser_printf("ending separated-expression\n");

            status = simplejs_make_binary_node(operator, node_type, left, *left, right);
            break;
        }

        default:
        {
            bool is_alu_assign = is_assign_operator(operator, string_cut);

            if (is_alu_assign)
            {
                simplejs_ast_node_t *cloned_left;

                status = simplejs_clone_ast_node(*left, &cloned_left);
                if (!SIMPLEJS_SUCCESS(status))
                {
                    simplejs_parser_printf("simplejs_clone_ast_node error\n");
                    goto result;
                }

                status = simplejs_process_operator(
                    parser_ctx, start_token,
                    &cloned_left, operator, rbp,
                    end_operators, end_operators_size,
                    string_cut + 1);
                if (!SIMPLEJS_SUCCESS(status))
                {
                    simplejs_free_ast_list(cloned_left);

                    simplejs_parser_printf("simplejs_process_operator error\n");
                    goto result;
                }

                status = simplejs_make_binary_node(operator, SIMPLEJS_AST_NODE_TYPE_OP_ASSIGN, left, *left, cloned_left);
            }
            else
            {
                status = simplejs_parse_expression(parser_ctx, start_token, &right, rbp, end_operators, end_operators_size);
                if (!SIMPLEJS_SUCCESS(status))
                {
                    simplejs_parser_printf("expression error\n");
                    goto result;
                }

                status = simplejs_make_binary_node(operator, node_type, left, *left, right);
            }

            break;
        }
        }
    }
    else
    {
        simplejs_parser_printf("processing unary\n");

        status = simplejs_make_unary_node(operator, node_type, true, left, *left);
    }

result:
    return status;
}

simplejs_status_t simplejs_parse_expression(
    simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token, simplejs_ast_node_t **out, int min_bp,
    char **end_operators, int end_operators_size)
{
    int end_operators_count = end_operators_size / sizeof(*end_operators);

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_t *token = simplejs_token_next(parser_ctx, start_token);

    simplejs_ast_node_t *left = NULL;

    simplejs_parse_expression_level++;

    simplejs_parser_printf("(start) simplejs_parse_expression_level = %u\n", simplejs_parse_expression_level);

    simplejs_parser_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
    simplejs_parser_printf("token->string = \"%s\"\n", token->string->buffer);

    status = simplejs_nud(parser_ctx, start_token, &left, token, end_operators, end_operators_size);
    if (!SIMPLEJS_SUCCESS(status))
        goto result;

    while (true)
    {
        simplejs_token_t *operator = simplejs_token_peek(parser_ctx, start_token);
        for (int i = 0; i < end_operators_count; i++)
        {
            char *end_operator = end_operators[i];

            if (simplejs_check_token_operator(operator, end_operator))
            {
                simplejs_parser_printf("end of expression\n");
                goto expression_end;
            }
        }

        if (!is_operator(parser_ctx, operator))
        {
            simplejs_present_parser_diagnostic(
                parser_ctx, parser_ctx->current_ast_stack, operator,
                SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "expected a valid operator");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        int lbp, rbp;
        simplejs_get_binding_power(operator, &lbp, &rbp);

        if (lbp < min_bp)
        {
            simplejs_parser_printf("weak binding\n");
            break;
        }

        simplejs_token_next(parser_ctx, start_token);

        status = simplejs_process_operator(parser_ctx, start_token, &left, operator, rbp, end_operators, end_operators_size, 0);
        if (!SIMPLEJS_SUCCESS(status))
            goto result;
    }

expression_end:
    *out = left;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        simplejs_parser_printf("freeing left\n");

        if (left)
            simplejs_free_ast_list(left);
    }

    simplejs_parser_printf("(end) simplejs_parse_expression_level = %u\n", simplejs_parse_expression_level);

    simplejs_parse_expression_level--;
    return status;
}
