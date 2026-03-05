#include <parser.h>

simplejs_status_t simplejs_alloc_ast_node(simplejs_ast_node_type_t type, simplejs_ast_node_t **out);
void simplejs_insert_children_ast_to_parent(simplejs_ast_node_t *parent, simplejs_ast_node_t *children);
bool simplejs_check_token_keyword(simplejs_token_t *token, char *value);
bool simplejs_check_token_operator(simplejs_token_t *token, char *value);

bool simplejs_get_scoped_output(
    simplejs_parser_ctx_t *parser_ctx, void *context, void *out,
    bool (*callback)(simplejs_ast_scope_context_t *scope_context, void *context, void *out));

int simplejs_parse_expression_level = 0;

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

typedef struct local_scoped
{
    bool is_function, is_argument;

    uint32_t reference;
    simplejs_utf8_string_t *string;
} local_scoped_t;

static bool simplejs_get_scoped_reference_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out)
{
    simplejs_utf8_string_t *string = context;
    local_scoped_t *out_struct = out;

    {
        simplejs_list_entry_t *end_var = &scope_context->var_list_entry;
        simplejs_list_entry_t *current_var = end_var->next;

        while (current_var != end_var)
        {
            simplejs_ast_var_context_t *var = simplejs_get_list_entry_structure(current_var);

            if (!strcmp((char *)var->name->buffer, (char *)string->buffer))
            {
                out_struct->is_function = false;
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

    return false;
}

simplejs_status_t simplejs_alloc_identifier_node(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t **out, simplejs_token_t *token, bool is_property)
{
    simplejs_utf8_string_t *function_name;
    local_scoped_t local_scoped;
    bool is_local = simplejs_get_scoped_output(parser_ctx, token->string, &local_scoped, simplejs_get_scoped_reference_callback);

    simplejs_ast_node_t *identifier_ast = NULL;
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
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE, &identifier_ast), result, status);

            identifier_ast->context = (void *)((uintptr_t)local_scoped.reference);
        }
    }
    else
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE, &identifier_ast), result, status);

        identifier_ast->context = token->string;
    }

    *out = identifier_ast;

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

static simplejs_ast_node_type_t inc_dec_table[2][2] = {
    {
        SIMPLEJS_AST_NODE_TYPE_ALU_ASSIGN_INC,
        SIMPLEJS_AST_NODE_TYPE_ALU_ASSIGN_DEC,
    },
    {
        SIMPLEJS_AST_NODE_TYPE_ALU_INC_ASSIGN,
        SIMPLEJS_AST_NODE_TYPE_ALU_DEC_ASSIGN,
    },
};

static simplejs_ast_node_t *simplejs_alloc_inc_dec_node(simplejs_ast_node_t *reference, bool is_dec, bool assign_after_operation)
{
    simplejs_ast_node_type_t type = inc_dec_table[assign_after_operation][is_dec];
    simplejs_ast_node_t *operation_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_ASSERT(reference != NULL);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(type, &operation_ast), result, status);

    simplejs_insert_tail_list(&operation_ast->children_list_entry, &reference->list_entry);

result:
    return operation_ast;
}

static simplejs_status_t simplejs_nud(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token, simplejs_ast_node_t **out, simplejs_token_t *token)
{
    simplejs_status_t status = SIMPLEJS_STATUS_NOT_IMPLEMENTED;

    if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
    {
        status = simplejs_alloc_identifier_node(parser_ctx, out, token, false);
        return status;
    }

    if (token->type == SIMPLEJS_TOKEN_TYPE_NUMBER)
    {
        status = simplejs_alloc_number_node(parser_ctx, out, token);
        return status;
    }

    if (simplejs_check_token_operator(token, "("))
    {
        simplejs_parser_printf("starting sub-expression\n");

        status = simplejs_parse_expression(parser_ctx, start_token, out, 0);
        simplejs_token_next(parser_ctx, start_token);

        simplejs_parser_printf("ending sub-expression\n");
        return status;
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

    simplejs_parser_printf("unexpected token in expression\n");
    return status;
}

static void simplejs_get_binding_power(simplejs_token_t *operator, int *lbp, int *rbp)
{
    if (simplejs_check_token_operator(operator, "=") ||
        simplejs_check_token_operator(operator, "+=") ||
        simplejs_check_token_operator(operator, "-="))
    {
        *lbp = 2;
        *rbp = 1;
        return;
    }

    if (simplejs_check_token_operator(operator, "+") ||
        simplejs_check_token_operator(operator, "-"))
    {
        *lbp = 10;
        *rbp = 11;
        return;
    }

    if (simplejs_check_token_operator(operator, "*") ||
        simplejs_check_token_operator(operator, "/") ||
        simplejs_check_token_operator(operator, "%"))
    {
        *lbp = 20;
        *rbp = 21;
        return;
    }

    if (simplejs_check_token_operator(operator, "."))
    {
        *lbp = 30;
        *rbp = 31;
        return;
    }

    if (simplejs_check_token_operator(operator, "("))
    {
        *lbp = 40;
        *rbp = 41;
        return;
    }

    if (simplejs_check_token_operator(operator, "<=") ||
        simplejs_check_token_operator(operator, ">=") ||
        simplejs_check_token_operator(operator, "<") ||
        simplejs_check_token_operator(operator, ">"))
    {
        *lbp = 50;
        *rbp = 51;
        return;
    }

    if (simplejs_check_token_operator(operator, "==") ||
        simplejs_check_token_operator(operator, "!="))
    {
        *lbp = 60;
        *rbp = 61;
        return;
    }

    *lbp = -1;
    *rbp = 0;
}

static char *expression_operators[] = {
    "=",
    "==",
    ">",
    "<",
    ".",
    "++",
    "--",
    "+",
    "-",
    "*",
    "/",
    "%",
    "(",
};

FUNC_IS_ON_STRING(__is_operator, expression_operators)

static bool is_operator(simplejs_parser_ctx_t *parser_ctx, simplejs_token_t *operator)
{
    simplejs_parser_printf("operator->type = %s\n", simplejs_get_token_type_string(operator->type));
    simplejs_parser_printf("operator->string = \"%s\"\n", operator->string->buffer);

    if (operator->type != SIMPLEJS_TOKEN_TYPE_OPERATOR)
        return false;

    simplejs_parser_printf("is_operator = %u\n", __is_operator(operator->string->buffer));

    return __is_operator(operator->string->buffer);
}

static struct operators_type_node
{
    char *operator;
    simplejs_ast_node_type_t node_type;
} binary_operators_type_node[] = {
    {
        .operator = ".",
        .node_type = SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS,
    },
    {
        .operator = "=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ASSIGN,
    },
    {
        .operator = "+=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_ADD_ASSIGN,
    },
    {
        .operator = "-=",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_SUB_ASSIGN,
    },

    {
        .operator = "==",
        .node_type = SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL,
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

static simplejs_ast_node_type_t simplejs_get_node_type_for_binary_operator(simplejs_utf8_string_t *string)
{
    for (size_t i = 0; i < sizeof(binary_operators_type_node) / sizeof(*binary_operators_type_node); i++)
    {
        if (!strcmp(binary_operators_type_node[i].operator, string->buffer))
        {
            return binary_operators_type_node[i].node_type;
        }
    }

    return SIMPLEJS_AST_NODE_TYPE_BINARY_OPERATOR;
}

static simplejs_status_t make_unary_node(simplejs_token_t *operator, simplejs_ast_node_type_t node_type, simplejs_ast_node_t **out, simplejs_ast_node_t *left)
{
    simplejs_ast_node_t *unary_ast = NULL;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(node_type, &unary_ast), result, status);
    unary_ast->context = operator->string;

    SIMPLEJS_ASSERT(left != NULL);

    simplejs_insert_children_ast_to_parent(unary_ast, left);

    *out = unary_ast;

result:
    return status;
}

static simplejs_status_t make_binary_node(simplejs_token_t *operator, simplejs_ast_node_type_t node_type, simplejs_ast_node_t **out, simplejs_ast_node_t *left, simplejs_ast_node_t *right)
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

simplejs_status_t simplejs_parse_expression(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **start_token, simplejs_ast_node_t **out, int min_bp)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_t *token = simplejs_token_next(parser_ctx, start_token);

    simplejs_ast_node_t *left;

    simplejs_parse_expression_level++;

    simplejs_parser_printf("(start) simplejs_parse_expression_level = %u\n", simplejs_parse_expression_level);

    simplejs_parser_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
    simplejs_parser_printf("token->string = \"%s\"\n", token->string->buffer);

    status = simplejs_nud(parser_ctx, start_token, &left, token);
    if (!SIMPLEJS_SUCCESS(status))
        goto result;

    while (true)
    {
        simplejs_token_t *operator = simplejs_token_peek(parser_ctx, start_token);
        if (!is_operator(parser_ctx, operator))
        {
            simplejs_parser_printf("not valid operator\n");
            break;
        }

        int lbp, rbp;
        simplejs_get_binding_power(operator, &lbp, &rbp);

        if (lbp < min_bp)
        {
            simplejs_parser_printf("weak binding\n");
            break;
        }

        simplejs_token_next(parser_ctx, start_token);

        simplejs_ast_node_t *right = NULL;
        simplejs_ast_node_type_t node_type = simplejs_get_node_type_for_binary_operator(operator->string);

        simplejs_parser_printf("node_type = %u\n", node_type);

        switch (node_type)
        {
        case SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS:
        {
            simplejs_token_t *prop = simplejs_token_next(parser_ctx, start_token);

            if (prop->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_parser_printf("invalid format\n");
                goto result;
            }

            status = simplejs_alloc_identifier_node(parser_ctx, &right, prop, true);

            if (!SIMPLEJS_SUCCESS(status))
                goto result;

            status = make_binary_node(operator, node_type, &left, left, right);
            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL:
        {
            simplejs_parser_printf("processing function!\n");

            status = make_unary_node(operator, node_type, &left, left);
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

                status = simplejs_parse_expression(parser_ctx, start_token, &arg, 0);
                if (!SIMPLEJS_SUCCESS(status))
                    goto result;

                simplejs_insert_children_ast_to_parent(left, arg);

                operator = simplejs_token_next(parser_ctx, start_token);
                if (simplejs_check_token_operator(operator, ")"))
                    break;

                if (!simplejs_check_token_operator(operator, ","))
                {
                    simplejs_parser_printf("invalid call argument separator! ('%s')\n", operator->string->buffer);
                    status = SIMPLEJS_STATUS_INVALID_TOKEN;
                    goto result;
                }
            }

            simplejs_token_next(parser_ctx, start_token);
            break;
        }

        default:
        {
            status = simplejs_parse_expression(parser_ctx, start_token, &right, rbp);
            if (!SIMPLEJS_SUCCESS(status))
            {
                simplejs_parser_printf("expression error\n", simplejs_get_token_type_string(token->type));
                goto result;
            }

            status = make_binary_node(operator, node_type, &left, left, right);
            break;
        }
        }

        if (!SIMPLEJS_SUCCESS(status))
            goto result;
    }

    *out = left;

result:
    simplejs_parser_printf("(end) simplejs_parse_expression_level = %u\n", simplejs_parse_expression_level);

    simplejs_parse_expression_level--;
    return status;
}
