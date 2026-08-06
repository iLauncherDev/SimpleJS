#include <parser.h>

char *func_args_end_operators[] = {
    ",",
    ")",
};

char *regular_end_operators[] = {
    ";",
};

char *variable_end_operators[] = {
    ",",
    ";",
};

char *branch_end_operators[] = {
    ")",
};

const char *simplejs_get_ast_node_type_string(simplejs_ast_node_type_t type)
{
    switch (type)
    {
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_EXPRESSION);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_RETURN);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_BRANCH);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_IF);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ELSE);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ELSE_IF);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_FOR_LOOP);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_WHILE_LOOP);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_LABEL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_GOTO);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_UNARY_OPERATOR);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_BINARY_OPERATOR);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_COMMA_OPERATOR);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_NUMBER);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_STRING);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ARGUMENT_REFERENCE);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ASSIGN);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_OP_ASSIGN);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_DELETE);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_LOGICAL_OR);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_LOGICAL_AND);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_INC);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_DEC);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_LOGICAL_NOT);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_BITWISE_NOT);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_NEG);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_NOT_EQUAL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_GREATER);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_BELOW);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_GREATER_EQUAL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_BELOW_EQUAL);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_OR);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_AND);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_SHL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_SHR);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_SAL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_SAR);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_ADD);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_SUB);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_MUL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_DIV);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ALU_MOD);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_EXPRESSION_PROPERTY_ACCESS);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_CODEBLOCK);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_VARDECL_LIST);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_VARDECL);
        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_FUNCDECL);

        CASE_TO_STRING(SIMPLEJS_AST_NODE_TYPE_ROOT);
    }

    return "Unknown";
}

bool simplejs_get_scoped_var_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out, bool is_out_of_function)
{
    simplejs_utf8_string_t *string = context;

    simplejs_list_entry_t *end_var = &scope_context->var_list_entry;
    simplejs_list_entry_t *current_var = end_var->next;

    while (current_var != end_var)
    {
        simplejs_list_entry_t *next_var = current_var->next;
        simplejs_ast_var_context_t *var = simplejs_get_list_entry_structure(current_var);

        if (!strcmp((char *)var->name->buffer, (char *)string->buffer))
        {
            *(void **)out = var;
            return true;
        }

        current_var = next_var;
    }

    return false;
}

bool simplejs_get_scoped_label_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out, bool is_out_of_function)
{
    simplejs_utf8_string_t *string = context;

    simplejs_list_entry_t *end_label = &scope_context->label_list_entry;
    simplejs_list_entry_t *current_label = end_label->next;

    while (current_label != end_label)
    {
        simplejs_list_entry_t *next_label = current_label->next;
        simplejs_ast_label_context_t *label = simplejs_get_list_entry_structure(current_label);

        simplejs_printf("label = %p\n", label);
        simplejs_printf("label->name = %p\n", label->name);

        if (!strcmp((char *)label->name->buffer, (char *)string->buffer))
        {
            *(void **)out = label;
            return true;
        }

        current_label = next_label;
    }

    return false;
}

bool simplejs_get_scoped_output(
    simplejs_parser_ctx_t *parser_ctx, void *context, void *out,
    simplejs_get_scoped_callback_f callback)
{
    SIMPLEJS_ASSERT(parser_ctx != NULL);
    SIMPLEJS_ASSERT(callback != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;

    simplejs_list_entry_t *end_scope = &current_function_context->scope_stack;
    simplejs_list_entry_t *current_scope = &current_scope_context->_function_scope_list_entry;

    while (current_scope != end_scope)
    {
        simplejs_list_entry_t *prev_scope = current_scope->prev;
        simplejs_ast_scope_context_t *scope_context = simplejs_get_list_entry_structure(current_scope);

        bool result = callback(scope_context, context, out, false);
        if (result)
            return true;

        current_scope = prev_scope;
    }

    return false;
}

bool simplejs_get_scoped_output_alt(
    simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *node, void *context, void *out,
    simplejs_get_scoped_callback_f callback,
    bool out_of_function_search)
{
    SIMPLEJS_ASSERT(parser_ctx != NULL);
    SIMPLEJS_ASSERT(callback != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    bool is_out_of_function = false;
    simplejs_ast_node_t *current_ast = node;

    while (current_ast != NULL)
    {
        bool will_quit = false;
        simplejs_ast_scope_context_t *scope_context = NULL;

        switch (current_ast->type)
        {
        case SIMPLEJS_AST_NODE_TYPE_CODEBLOCK:
            scope_context = current_ast->context;
            break;

        case SIMPLEJS_AST_NODE_TYPE_ROOT:
        case SIMPLEJS_AST_NODE_TYPE_FUNCDECL:
        {
            simplejs_ast_function_context_t *function_context = current_ast->context;

            scope_context = function_context->root_scope;

            will_quit = true;
            break;
        }

        default:
            goto skip_ast;
        }

        simplejs_printf("level change (type: '%s')\n", simplejs_get_ast_node_type_string(current_ast->type));

        bool result = callback(scope_context, context, out, is_out_of_function);
        if (result)
            return true;

        if (will_quit)
        {
            is_out_of_function = true;

            if (!out_of_function_search)
                break;
        }

    skip_ast:
        current_ast = current_ast->parent_node;
    }

    return false;
}

simplejs_status_t simplejs_alloc_ast_node(simplejs_ast_node_type_t type, simplejs_ast_node_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    ret->type = type;

    simplejs_init_list_entry(&ret->children_list_entry, ret);
    simplejs_init_list_entry(&ret->list_entry, ret);

    simplejs_init_list_entry(&ret->_stack_list_entry, ret);
    simplejs_init_list_entry(&ret->_ast_function_list_entry, ret);

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_expression_context(simplejs_ast_expression_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_expression_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_var_context(simplejs_ast_var_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_var_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->_scope_var_list_entry, ret);

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_label_context(simplejs_ast_label_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_label_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->_scope_label_list_entry, ret);

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_branch_context(simplejs_ast_branch_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_branch_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_scope_context(simplejs_ast_scope_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_scope_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->function_list_entry, ret);
    simplejs_init_list_entry(&ret->arg_list_entry, ret);
    simplejs_init_list_entry(&ret->var_list_entry, ret);
    simplejs_init_list_entry(&ret->label_list_entry, ret);

    simplejs_init_list_entry(&ret->_function_scope_list_entry, ret);

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_ast_function_context(simplejs_ast_function_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_function_context_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->scope_stack, ret);

    simplejs_init_list_entry(&ret->_stack_list_entry, ret);
    simplejs_init_list_entry(&ret->_scope_list_entry, ret);

    *out = ret;

result:
    return status;
}

static simplejs_status_t simplejs_alloc_full_function_context(simplejs_ast_function_context_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_scope_context_t *scope_context = NULL;
    simplejs_ast_function_context_t *function_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_scope_context(&scope_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_function_context(&function_context), result, status);

    function_context->root_scope = scope_context;
    function_context->current_scope_stack = scope_context;
    simplejs_insert_tail_list(&function_context->scope_stack, &scope_context->_function_scope_list_entry);

    *out = function_context;
result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (scope_context)
            simplejs_hook_mfree(scope_context);

        if (function_context)
            simplejs_hook_mfree(function_context);
    }

    return status;
}

static simplejs_status_t simplejs_alloc_root_ast(simplejs_ast_node_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *ret = NULL;
    simplejs_ast_function_context_t *function_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_full_function_context(&function_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_ROOT, &ret), result, status);

    ret->context = function_context;

    *out = ret;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (function_context)
        {
            simplejs_hook_mfree(simplejs_get_list_entry_structure(function_context->scope_stack.next));
            simplejs_hook_mfree(function_context);
        }

        if (ret)
            simplejs_hook_mfree(ret);
    }

    return status;
}

static void simplejs_push_ast_to_stack(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *node)
{
    simplejs_insert_tail_list(&parser_ctx->ast_stack, &node->_stack_list_entry);
    parser_ctx->current_ast_stack = simplejs_get_list_entry_structure(parser_ctx->ast_stack.prev);
    parser_ctx->current_ast_stack->_saved_parser_state = parser_ctx->state;

    SIMPLEJS_ASSERT(parser_ctx->current_ast_stack == node);
}

static void simplejs_pop_ast_from_stack(simplejs_parser_ctx_t *parser_ctx)
{
    parser_ctx->state = parser_ctx->current_ast_stack->_saved_parser_state;

    simplejs_remove_entry_list(parser_ctx->ast_stack.prev);
    parser_ctx->current_ast_stack = simplejs_get_list_entry_structure(parser_ctx->ast_stack.prev);

    simplejs_printf("popped to state %u\n", parser_ctx->state);
    simplejs_printf("popped to ast %s\n", simplejs_get_ast_node_type_string(parser_ctx->current_ast_stack->type));
}

void simplejs_insert_children_ast_to_parent(simplejs_ast_node_t *parent, simplejs_ast_node_t *children)
{
    SIMPLEJS_ASSERT(parent != NULL);
    SIMPLEJS_ASSERT(children != NULL);

    children->parent_node = parent;
    simplejs_insert_tail_list(&parent->children_list_entry, &children->list_entry);
    parent->children_list_count++;
}

static void simplejs_add_children_ast(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *children)
{
    simplejs_ast_node_t *parent = parser_ctx->current_ast_stack;

    simplejs_insert_children_ast_to_parent(parent, children);
}

static simplejs_status_t simplejs_add_function_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_ast_node_t *function_ast = NULL;
    simplejs_ast_function_context_t *function_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_full_function_context(&function_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_FUNCDECL, &function_ast), result, status);

    function_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_insert_tail_list(&current_scope_context->function_list_entry, &function_context->_scope_list_entry);

    parser_ctx->current_function_context_stack = function_context;
    simplejs_insert_tail_list(&parser_ctx->function_context_stack, &parser_ctx->current_function_context_stack->_stack_list_entry);
    simplejs_insert_tail_list(&parser_ctx->ast_function_list, &function_ast->_ast_function_list_entry);

    function_ast->context = function_context;

    simplejs_add_children_ast(parser_ctx, function_ast);
    simplejs_push_ast_to_stack(parser_ctx, function_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_FUNCDECL_NAME;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (function_context)
        {
            simplejs_hook_mfree(simplejs_get_list_entry_structure(function_context->scope_stack.next));
            simplejs_hook_mfree(function_context);
        }

        if (function_ast)
            simplejs_hook_mfree(function_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_branch_ast_ex(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_type_t type, simplejs_ast_node_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *branch_ast = NULL;
    simplejs_ast_branch_context_t *branch_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_branch_context(&branch_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(type, &branch_ast), result, status);

    branch_ast->diagnostic_token = parser_ctx->current_token;

    branch_ast->context = branch_context;

    simplejs_add_children_ast(parser_ctx, branch_ast);
    simplejs_push_ast_to_stack(parser_ctx, branch_ast);

    *out = branch_ast;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (branch_context)
            simplejs_hook_mfree(branch_context);

        if (branch_ast)
            simplejs_hook_mfree(branch_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_branch_ast(simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_type_t type, simplejs_parser_state_t state, bool will_merge)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_ast_node_t *branch_ast = NULL;
    simplejs_ast_node_t *branch_statement_ast = NULL;

    if (!will_merge)
    {
        status = simplejs_add_branch_ast_ex(parser_ctx, SIMPLEJS_AST_NODE_TYPE_BRANCH, &branch_ast);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }

        status = simplejs_add_branch_ast_ex(parser_ctx, type, &branch_statement_ast);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }

        parser_ctx->state = state;
    }
    else
    {
        simplejs_ast_node_t *private_branch_ast = simplejs_get_list_entry_structure(parser_ctx->current_ast_stack->children_list_entry.prev);
        SIMPLEJS_ASSERT(private_branch_ast->type == SIMPLEJS_AST_NODE_TYPE_BRANCH);

        simplejs_printf("merging with '%s'\n", simplejs_get_ast_node_type_string(private_branch_ast->type));

        simplejs_push_ast_to_stack(parser_ctx, private_branch_ast);

        status = simplejs_add_branch_ast_ex(parser_ctx, type, &branch_statement_ast);
        if (!SIMPLEJS_SUCCESS(status))
        {
            simplejs_pop_ast_from_stack(parser_ctx);
            goto result;
        }

        parser_ctx->state = state;
    }

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (branch_ast->context)
            simplejs_hook_mfree(branch_ast->context);
        if (branch_ast)
            simplejs_hook_mfree(branch_ast);

        if (branch_statement_ast->context)
            simplejs_hook_mfree(branch_statement_ast->context);
        if (branch_statement_ast)
            simplejs_hook_mfree(branch_statement_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_codeblock_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *scope_ast = NULL;
    simplejs_ast_scope_context_t *scope_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_scope_context(&scope_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_CODEBLOCK, &scope_ast), result, status);

    scope_ast->diagnostic_token = parser_ctx->current_token;

    scope_ast->context = scope_context;

    current_function_context->current_scope_stack = scope_context;
    simplejs_insert_tail_list(&current_function_context->scope_stack, &scope_context->_function_scope_list_entry);

    simplejs_add_children_ast(parser_ctx, scope_ast);
    simplejs_push_ast_to_stack(parser_ctx, scope_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_CODEBLOCK;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (scope_context)
            simplejs_hook_mfree(scope_context);

        if (scope_ast)
            simplejs_hook_mfree(scope_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_while_loop_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *while_loop_ast = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_WHILE_LOOP, &while_loop_ast), result, status);

    while_loop_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_add_children_ast(parser_ctx, while_loop_ast);
    simplejs_push_ast_to_stack(parser_ctx, while_loop_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_WHILE_LOOP_ARGS;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (while_loop_ast)
            simplejs_hook_mfree(while_loop_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_for_loop_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *for_loop_ast = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_FOR_LOOP, &for_loop_ast), result, status);

    for_loop_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_add_children_ast(parser_ctx, for_loop_ast);
    simplejs_push_ast_to_stack(parser_ctx, for_loop_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_ARGS;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (for_loop_ast)
            simplejs_hook_mfree(for_loop_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_label_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *label_ast = NULL;
    simplejs_ast_label_context_t *label_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_label_context(&label_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_LABEL, &label_ast), result, status);

    label_ast->diagnostic_token = parser_ctx->current_token;

    label_ast->context = label_context;

    simplejs_insert_tail_list(&current_scope_context->label_list_entry, &label_context->_scope_label_list_entry);

    simplejs_add_children_ast(parser_ctx, label_ast);
    simplejs_push_ast_to_stack(parser_ctx, label_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_LABEL_NAME;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (label_context)
            simplejs_hook_mfree(label_context);

        if (label_ast)
            simplejs_hook_mfree(label_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_goto_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *goto_ast = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_GOTO, &goto_ast), result, status);

    goto_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_add_children_ast(parser_ctx, goto_ast);
    simplejs_push_ast_to_stack(parser_ctx, goto_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_GOTO_NAME;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (goto_ast)
            simplejs_hook_mfree(goto_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_var_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *var_ast = NULL;
    simplejs_ast_var_context_t *var_context = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_var_context(&var_context), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_VARDECL, &var_ast), result, status);

    var_ast->diagnostic_token = parser_ctx->current_token;

    var_ast->context = var_context;

    var_context->index = current_function_context->local_var_count++;
    simplejs_insert_tail_list(&current_scope_context->var_list_entry, &var_context->_scope_var_list_entry);

    simplejs_add_children_ast(parser_ctx, var_ast);
    simplejs_push_ast_to_stack(parser_ctx, var_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_VARDECL_NAME;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (var_context)
            simplejs_hook_mfree(var_context);

        if (var_ast)
            simplejs_hook_mfree(var_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_var_idle_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;
    simplejs_ast_scope_context_t *current_scope_context = current_function_context->current_scope_stack;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *var_ast = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_VARDECL_LIST, &var_ast), result, status);

    var_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_add_children_ast(parser_ctx, var_ast);
    simplejs_push_ast_to_stack(parser_ctx, var_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_VARDECL_IDLE;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (var_ast)
            simplejs_hook_mfree(var_ast);
    }

    return status;
}

static simplejs_status_t simplejs_add_expression_ast(simplejs_parser_ctx_t *parser_ctx, char **end_operators, int end_operators_size)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *expression_ast = NULL;
    simplejs_ast_expression_context_t *expression_ctx = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_EXPRESSION, &expression_ast), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_expression_context(&expression_ctx), result, status);

    expression_ast->diagnostic_token = parser_ctx->current_token;

    expression_ctx->end_operators = end_operators;
    expression_ctx->end_operators_size = end_operators_size;

    expression_ast->context = expression_ctx;

    simplejs_add_children_ast(parser_ctx, expression_ast);
    simplejs_push_ast_to_stack(parser_ctx, expression_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_EXPRESSION;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (expression_ast)
            simplejs_hook_mfree(expression_ast);

        if (expression_ctx)
            simplejs_hook_mfree(expression_ctx);
    }

    return status;
}

static simplejs_status_t simplejs_add_return_ast(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_node_t *return_ast = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_ast_node(SIMPLEJS_AST_NODE_TYPE_RETURN, &return_ast), result, status);

    return_ast->diagnostic_token = parser_ctx->current_token;

    simplejs_add_children_ast(parser_ctx, return_ast);
    simplejs_push_ast_to_stack(parser_ctx, return_ast);

    parser_ctx->state = SIMPLEJS_PARSER_STATE_RETURN_EXPRESSION;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (return_ast)
            simplejs_hook_mfree(return_ast);
    }

    return status;
}

static void simplejs_leave_codeblock(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_ast_function_context_t *current_function_context = parser_ctx->current_function_context_stack;

    simplejs_remove_entry_list(&current_function_context->current_scope_stack->_function_scope_list_entry);
    current_function_context->current_scope_stack = simplejs_get_list_entry_structure(current_function_context->scope_stack.prev);
}

static void simplejs_leave_function(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_remove_entry_list(parser_ctx->function_context_stack.prev);
    parser_ctx->current_function_context_stack = simplejs_get_list_entry_structure(parser_ctx->function_context_stack.prev);
}

void simplejs_free_ast_list(simplejs_ast_node_t *node)
{
    simplejs_list_entry_t *end_ast = &node->children_list_entry;
    simplejs_list_entry_t *current_ast = end_ast->next;

    simplejs_printf("freeing node of '%s' type\n", simplejs_get_ast_node_type_string(node->type));

    while (current_ast != end_ast)
    {
        simplejs_list_entry_t *next_ast = current_ast->next;
        simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

        simplejs_free_ast_list(ast);

        current_ast = next_ast;
    }

    switch (node->type)
    {
    case SIMPLEJS_AST_NODE_TYPE_ROOT:
    case SIMPLEJS_AST_NODE_TYPE_FUNCDECL:
    {
        simplejs_ast_function_context_t *function_context = node->context;
        if (!function_context)
        {
            goto skip_function_context;
        }

        if (function_context->root_scope)
            simplejs_hook_mfree(function_context->root_scope);
    }
    skip_function_context:

    case SIMPLEJS_AST_NODE_TYPE_LABEL:
    case SIMPLEJS_AST_NODE_TYPE_BRANCH:

    case SIMPLEJS_AST_NODE_TYPE_IF:
    case SIMPLEJS_AST_NODE_TYPE_ELSE:
    case SIMPLEJS_AST_NODE_TYPE_ELSE_IF:

    case SIMPLEJS_AST_NODE_TYPE_CODEBLOCK:

    case SIMPLEJS_AST_NODE_TYPE_EXPRESSION:

    case SIMPLEJS_AST_NODE_TYPE_VARDECL:
        if (node->context)
            simplejs_hook_mfree(node->context);
        break;

    default:
        break;
    }

    simplejs_hook_mfree(node);
}

void SIMPLEJS_API simplejs_free_parser_ctx(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_free_ast_list(parser_ctx->root_ast);

    simplejs_hook_mfree(parser_ctx);
}

bool simplejs_check_token_expr_keyword(simplejs_token_t *token, char *value)
{
    simplejs_utf8_string_t *string = token->string;

    if (token->type != SIMPLEJS_TOKEN_TYPE_EXPR_KEYWORD)
        return false;

    if (strncmp((char *)string->buffer, value, string->max_size))
        return false;

    return true;
}

bool simplejs_check_token_keyword(simplejs_token_t *token, char *value)
{
    simplejs_utf8_string_t *string = token->string;

    if (token->type != SIMPLEJS_TOKEN_TYPE_KEYWORD)
        return false;

    if (strncmp((char *)string->buffer, value, string->max_size))
        return false;

    return true;
}

bool simplejs_check_token_operator(simplejs_token_t *token, char *value)
{
    simplejs_utf8_string_t *string = token->string;

    if (token->type != SIMPLEJS_TOKEN_TYPE_OPERATOR)
        return false;

    if (strncmp((char *)string->buffer, value, string->max_size))
        return false;

    return true;
}

static bool used_spaces = false;

static void simplejs_printf_space(int spaces, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    if (!used_spaces)
    {
        used_spaces = true;
        for (int i = 0; i < spaces; i++)
            simplejs_printf(" ");
    }

    if (strchr(fmt, '\n'))
    {
        used_spaces = false;
    }

    simplejs_vprintf(fmt, args);
    va_end(args);
}

static void simplejs_dump_ast_node(simplejs_list_entry_t *ast_list_entry, int spaces)
{
    simplejs_list_entry_t *end_ast = ast_list_entry;
    simplejs_list_entry_t *current_ast = end_ast->next;

    while (current_ast != end_ast)
    {
        simplejs_list_entry_t *next_ast = current_ast->next;
        simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

        simplejs_printf_space(spaces, "AST Node (Type: %s)\n", simplejs_get_ast_node_type_string(ast->type));
        if (!ast->diagnostic_token)
            simplejs_printf_space(spaces, "No diagnostic token\n");

        switch (ast->type)
        {
        case SIMPLEJS_AST_NODE_TYPE_BINARY_OPERATOR:
            simplejs_printf_space(spaces, "Binary Operator \"%s\"\n", ((simplejs_utf8_string_t *)ast->context)->buffer);
            break;

        case SIMPLEJS_AST_NODE_TYPE_NUMBER:
        {
            simplejs_printf_space(spaces, "Number: ");

            simplejs_number_t *number = ast->context;
            bool is_signed = number->type < SIMPLEJS_NUMBER_TYPE_UI32;

            switch (number->type)
            {
            case SIMPLEJS_NUMBER_TYPE_I32:
            case SIMPLEJS_NUMBER_TYPE_UI32:
                if (is_signed)
                    simplejs_printf_space(spaces, "(i8) %d", number->value.i32);
                else
                    simplejs_printf_space(spaces, "(ui32) %u", number->value.ui32);
                break;

            case SIMPLEJS_NUMBER_TYPE_I64:
            case SIMPLEJS_NUMBER_TYPE_UI64:
                if (is_signed)
                    simplejs_printf_space(spaces, "(i8) %ld", number->value.i64);
                else
                    simplejs_printf_space(spaces, "(ui64) %lu", number->value.ui64);
                break;

            case SIMPLEJS_NUMBER_TYPE_F32:
                simplejs_printf_space(spaces, "(f32) %f", number->value.f32);
                break;

            case SIMPLEJS_NUMBER_TYPE_F64:
                simplejs_printf_space(spaces, "(f64) %f", number->value.f64);
                break;
            }

            simplejs_printf_space(spaces, "\n");
            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE:
            simplejs_printf_space(spaces, "Local Reference %u\n", (uint32_t)((uintptr_t)ast->context));
            break;

        case SIMPLEJS_AST_NODE_TYPE_ARGUMENT_REFERENCE:
            simplejs_printf_space(spaces, "Argument Reference %u\n", (uint32_t)((uintptr_t)ast->context));
            break;

        case SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE:
            simplejs_printf_space(spaces, "Global Reference\n");
            break;

        case SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE:
            simplejs_printf_space(spaces, "Property Reference \"%s\"\n", ((simplejs_utf8_string_t *)ast->context)->buffer);
            break;

        default:
            break;
        }

        simplejs_printf_space(spaces, "AST Node Children Count %lu\n", ast->children_list_count);
        simplejs_printf_space(spaces, "AST Node Children Start\n");

        simplejs_dump_ast_node(&ast->children_list_entry, spaces + 4);

        simplejs_printf_space(spaces, "AST Node Children End\n");

        simplejs_printf_space(spaces, "\n");

        current_ast = next_ast;
    }
}

void simplejs_dump_ast_tree(simplejs_parser_ctx_t *parser_ctx)
{
    simplejs_dump_ast_node(&parser_ctx->root_ast->children_list_entry, 0);
}

static simplejs_status_t check_var_expression(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **current_token)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_t *token = simplejs_get_list_entry_structure(*current_token);

    if (simplejs_check_token_keyword(token, "var"))
    {
        simplejs_printf("vars to be done!\n");
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_var_idle_ast(parser_ctx), result, status);
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, regular_end_operators, sizeof(regular_end_operators)), result, status);
    (*current_token) = (*current_token)->prev;

result:
    return status;
}

static simplejs_status_t check_code_expression(simplejs_parser_ctx_t *parser_ctx, simplejs_list_entry_t **current_token)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_token_t *token = simplejs_get_list_entry_structure(*current_token);

    if (simplejs_check_token_operator(token, "{"))
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_codeblock_ast(parser_ctx), result, status);
        goto result;
    }

    if (simplejs_check_token_keyword(token, "return"))
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_return_ast(parser_ctx), result, status);
        goto result;
    }

    if (simplejs_check_token_keyword(token, "goto"))
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_goto_ast(parser_ctx), result, status);
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, regular_end_operators, sizeof(regular_end_operators)), result, status);
    (*current_token) = (*current_token)->prev;

result:
    return status;
}

void simplejs_present_parser_diagnostic(
    simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *ast_node, simplejs_token_t *target_token,
    char *type, char *message)
{
    simplejs_token_t *ast_token = ast_node ? ast_node->diagnostic_token : NULL;
    if (!ast_token)
        ast_token = target_token;

    simplejs_diagnostic_message_t diagnostic_message;
    simplejs_init_diagnostic_message(&diagnostic_message);

    diagnostic_message.linemap_ctx = parser_ctx->token_ctx->linemap_ctx;

    diagnostic_message.line_offset.start = ast_token->offset.start;
    diagnostic_message.line_offset.end = target_token->offset.end;

    diagnostic_message.token_offset.start = target_token->offset.start;
    diagnostic_message.token_offset.end = target_token->offset.end;

    diagnostic_message.type = type;
    diagnostic_message.message = message;

    simplejs_present_diagnostic_message(&diagnostic_message);
}

simplejs_status_t SIMPLEJS_API simplejs_tokens_to_ast(simplejs_token_ctx_t *token_ctx, simplejs_parser_ctx_t **out)
{
    simplejs_diagnostic_message_t diagnostic_message;
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_parser_ctx_t *parser_ctx = simplejs_hook_malloc(sizeof(*parser_ctx));
    if (!parser_ctx)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(parser_ctx, sizeof(*parser_ctx));

    SIMPLEJS_ASSERT(token_ctx != NULL);
    SIMPLEJS_ASSERT(out != NULL);

    parser_ctx->token_ctx = token_ctx;

    parser_ctx->state = SIMPLEJS_PARSER_STATE_IDLE;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_root_ast(&parser_ctx->root_ast), result, status);

    simplejs_init_list_entry(&parser_ctx->ast_function_list, parser_ctx);
    simplejs_insert_tail_list(&parser_ctx->ast_function_list, &parser_ctx->root_ast->_ast_function_list_entry);

    parser_ctx->current_ast_stack = parser_ctx->root_ast;
    simplejs_init_list_entry(&parser_ctx->ast_stack, parser_ctx);
    simplejs_insert_tail_list(&parser_ctx->ast_stack, &parser_ctx->current_ast_stack->_stack_list_entry);

    parser_ctx->current_function_context_stack = parser_ctx->root_ast->context;
    simplejs_init_list_entry(&parser_ctx->function_context_stack, parser_ctx);
    simplejs_insert_tail_list(&parser_ctx->function_context_stack, &((simplejs_ast_function_context_t *)parser_ctx->current_function_context_stack)->_stack_list_entry);

    simplejs_list_entry_t *end_token = &token_ctx->token_list;
    simplejs_list_entry_t *current_token = end_token->next;

    while (current_token != end_token)
    {
        simplejs_token_t *token = simplejs_get_list_entry_structure(current_token);
        parser_ctx->current_token = token;

        simplejs_token_t *next_token = simplejs_get_list_entry_structure(current_token->next);
        if (&next_token->list_entry == end_token)
            next_token = NULL;

        bool is_codeblock = false;

        switch (parser_ctx->state)
        {
        case SIMPLEJS_PARSER_STATE_CODEBLOCK:
            is_codeblock = true;
        case SIMPLEJS_PARSER_STATE_IDLE:
            if (simplejs_check_token_operator(token, ";"))
            {
                break;
            }

            if (simplejs_check_token_keyword(token, "function"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_function_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "return"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_return_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "var"))
            {
                simplejs_printf("vars to be done!\n");
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_var_idle_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "label"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_label_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "goto"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_goto_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "if"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_branch_ast(parser_ctx, SIMPLEJS_AST_NODE_TYPE_IF, SIMPLEJS_PARSER_STATE_BRANCH_IF, false), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "else"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_branch_ast(parser_ctx, SIMPLEJS_AST_NODE_TYPE_ELSE, SIMPLEJS_PARSER_STATE_BRANCH_ELSE, true), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "while"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_while_loop_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_keyword(token, "for"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_for_loop_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_operator(token, "{"))
            {
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_codeblock_ast(parser_ctx), result, status);
                break;
            }

            if (is_codeblock && simplejs_check_token_operator(token, "}"))
            {
                simplejs_leave_codeblock(parser_ctx);
                simplejs_pop_ast_from_stack(parser_ctx);
                break;
            }

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, regular_end_operators, sizeof(regular_end_operators)), result, status);
            current_token = current_token->prev;
            break;

        case SIMPLEJS_PARSER_STATE_BRANCH_IF:
        {
            if (simplejs_check_token_operator(token, "("))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_BRANCH_IF_EXPR;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, branch_end_operators, sizeof(branch_end_operators)), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_BRANCH_IF_EXPR:
        {
            if (simplejs_check_token_operator(token, ")"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_BRANCH_CODE;
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_BRANCH_ELSE:
        {
            if (simplejs_check_token_keyword(token, "if"))
            {
                parser_ctx->current_ast_stack->type = SIMPLEJS_AST_NODE_TYPE_ELSE_IF;
                parser_ctx->state = SIMPLEJS_PARSER_STATE_BRANCH_IF;
                break;
            }

            parser_ctx->state = SIMPLEJS_PARSER_STATE_BRANCH_CODE;
            goto branch_code;
        }

        case SIMPLEJS_PARSER_STATE_BRANCH_CODE:
        {
        branch_code:
            parser_ctx->state = SIMPLEJS_PARSER_STATE_BRANCH_END;
            SIMPLEJS_REQUIRE_SUCCESS(check_code_expression(parser_ctx, &current_token), result, status);

            break;
        }

        case SIMPLEJS_PARSER_STATE_BRANCH_END:
        {
            current_token = current_token->prev;

            simplejs_ast_node_t *branch_node = parser_ctx->current_ast_stack;
            simplejs_ast_node_t *parent_branch_node = parser_ctx->current_ast_stack->parent_node;
            simplejs_ast_branch_context_t *branch_context = parent_branch_node->context;

            if (branch_node->type == SIMPLEJS_AST_NODE_TYPE_ELSE)
            {
                simplejs_printf("branch ended because of else statement!\n");
                branch_context->ended_branch = true;
            }

            simplejs_pop_ast_from_stack(parser_ctx);
            simplejs_pop_ast_from_stack(parser_ctx);

            break;
        }

        case SIMPLEJS_PARSER_STATE_FUNCDECL_NAME:
        {
            simplejs_ast_function_context_t *function_context = parser_ctx->current_ast_stack->context;

            if (!function_context->name &&
                token->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_printf("its expected to have function name!\n");
                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }
            else if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                function_context->name = token->string;
                break;
            }

            if (simplejs_check_token_operator(token, "("))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FUNCDECL_ARGS;
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FUNCDECL_ARGS:
        {
            simplejs_ast_function_context_t *function_context = parser_ctx->current_ast_stack->context;
            simplejs_ast_scope_context_t *scope_context = simplejs_get_list_entry_structure(function_context->scope_stack.next);

            if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER &&
                !function_context->_process_arg)
            {
                token->_arg_index = function_context->local_arg_count++;

                simplejs_init_list_entry(&token->_arg_list_entry, token);
                simplejs_insert_tail_list(&scope_context->arg_list_entry, &token->_arg_list_entry);

                function_context->_process_arg = true;
                break;
            }

            if (simplejs_check_token_operator(token, ",") &&
                function_context->_process_arg)
            {
                function_context->_process_arg = false;
                break;
            }

            if (simplejs_check_token_operator(token, ")"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FUNCDECL_BLOCK;
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FUNCDECL_BLOCK:
        {
            if (simplejs_check_token_operator(token, "{"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FUNCDECL_END;
                simplejs_add_codeblock_ast(parser_ctx);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FUNCDECL_END:
        {
            current_token = current_token->prev;

            simplejs_leave_function(parser_ctx);
            simplejs_pop_ast_from_stack(parser_ctx);
            break;
        }

        case SIMPLEJS_PARSER_STATE_VARDECL_IDLE:
        {
            if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                current_token = current_token->prev;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_var_ast(parser_ctx), result, status);
                break;
            }

            if (simplejs_check_token_operator(token, ";"))
            {
                current_token = current_token->prev;
                simplejs_pop_ast_from_stack(parser_ctx);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_VARDECL_NAME:
        {
            simplejs_ast_var_context_t *var_context = parser_ctx->current_ast_stack->context;

            if (!var_context->name &&
                token->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_printf("its expected to have var name!\n");
                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }
            else if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                var_context->name = token->string;
                break;
            }

            if (simplejs_check_token_operator(token, ";") ||
                simplejs_check_token_operator(token, ","))
            {
                simplejs_printf("var end\n");
                goto vardecl_end;
            }
            else if (simplejs_check_token_operator(token, "="))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_VARDECL_END;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, variable_end_operators, sizeof(variable_end_operators)), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        vardecl_end:
        case SIMPLEJS_PARSER_STATE_VARDECL_END:
        {
            simplejs_ast_var_context_t *var_context = parser_ctx->current_ast_stack->context;

            if (simplejs_check_token_operator(token, ";"))
            {
                current_token = current_token->prev;
            }
            else if (!simplejs_check_token_operator(token, ","))
            {
                simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }

            simplejs_printf("var end!\n");

            simplejs_pop_ast_from_stack(parser_ctx);
            break;
        }

        case SIMPLEJS_PARSER_STATE_LABEL_NAME:
        {
            simplejs_ast_label_context_t *label_context = parser_ctx->current_ast_stack->context;

            if (!label_context->name &&
                token->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_present_parser_diagnostic(
                    parser_ctx, parser_ctx->current_ast_stack, token,
                    SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "expected a valid label name");

                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }
            else if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                label_context->name = token->string;
                break;
            }

            if (simplejs_check_token_operator(token, ";"))
            {
                SIMPLEJS_ASSERT(label_context->name != NULL);

                simplejs_printf("label end\n");
                simplejs_pop_ast_from_stack(parser_ctx);
                break;
            }

            simplejs_present_parser_diagnostic(
                parser_ctx, parser_ctx->current_ast_stack, token,
                SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_GOTO_NAME:
        {
            if (!parser_ctx->current_ast_stack->context &&
                token->type != SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                simplejs_printf("its expected to have goto name!\n");
                status = SIMPLEJS_STATUS_INVALID_TOKEN;
                goto result;
            }
            else if (token->type == SIMPLEJS_TOKEN_TYPE_IDENTIFIER)
            {
                parser_ctx->current_ast_stack->context = token->string;
                break;
            }

            if (simplejs_check_token_operator(token, ";"))
            {
                SIMPLEJS_ASSERT(parser_ctx->current_ast_stack->context != NULL);

                simplejs_printf("goto end\n");
                simplejs_pop_ast_from_stack(parser_ctx);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_WHILE_LOOP_ARGS:
        {
            if (simplejs_check_token_operator(token, "("))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_WHILE_LOOP_CONDITION;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, branch_end_operators, sizeof(branch_end_operators)), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_WHILE_LOOP_CONDITION:
        {
            if (simplejs_check_token_operator(token, ")"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_WHILE_LOOP_CODE;
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_WHILE_LOOP_CODE:
        {
            parser_ctx->state = SIMPLEJS_PARSER_STATE_WHILE_LOOP_END;
            SIMPLEJS_REQUIRE_SUCCESS(check_code_expression(parser_ctx, &current_token), result, status);

            break;
        }

        case SIMPLEJS_PARSER_STATE_WHILE_LOOP_END:
        {
            current_token = current_token->prev;

            simplejs_pop_ast_from_stack(parser_ctx);
            break;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_ARGS:
        {
            if (simplejs_check_token_operator(token, "("))
            {
                current_token = current_token->next;

                parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_INIT;
                SIMPLEJS_REQUIRE_SUCCESS(check_var_expression(parser_ctx, &current_token), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_INIT:
        {
            if (simplejs_check_token_operator(token, ";"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_CONDITION;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, regular_end_operators, sizeof(regular_end_operators)), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_CONDITION:
        {
            if (simplejs_check_token_operator(token, ";"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_STEP;
                SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, branch_end_operators, sizeof(branch_end_operators)), result, status);
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_STEP:
        {
            if (simplejs_check_token_operator(token, ")"))
            {
                parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_CODE;
                break;
            }

            simplejs_present_parser_diagnostic(parser_ctx, parser_ctx->current_ast_stack, token, SIMPLEJS_DIAGNOSTIC_MESSAGE_TYPE_ERROR, "invalid token!");

            status = SIMPLEJS_STATUS_INVALID_TOKEN;
            goto result;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_CODE:
        {
            parser_ctx->state = SIMPLEJS_PARSER_STATE_FOR_LOOP_END;
            SIMPLEJS_REQUIRE_SUCCESS(check_code_expression(parser_ctx, &current_token), result, status);

            break;
        }

        case SIMPLEJS_PARSER_STATE_FOR_LOOP_END:
        {
            current_token = current_token->prev;

            simplejs_pop_ast_from_stack(parser_ctx);
            break;
        }

        case SIMPLEJS_PARSER_STATE_EXPRESSION:
        {
            simplejs_printf("start expression token!\n");
            simplejs_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
            simplejs_printf("token->string = \"%s\"\n", token->string->buffer);

            simplejs_ast_node_t *expression;
            simplejs_ast_expression_context_t *expression_ctx = parser_ctx->current_ast_stack->context;

            status = simplejs_parse_expression(parser_ctx, &current_token, &expression, 0, expression_ctx->end_operators, expression_ctx->end_operators_size);
            if (!SIMPLEJS_SUCCESS(status))
            {
                goto result;
            }

            simplejs_add_children_ast(parser_ctx, expression);

            token = simplejs_get_list_entry_structure(current_token);

            simplejs_printf("ended expression tokens!\n");
            simplejs_printf("token->type = %s\n", simplejs_get_token_type_string(token->type));
            simplejs_printf("token->string = \"%s\"\n", token->string ? (char *)token->string->buffer : "NULL");

            current_token = current_token->prev;

            simplejs_pop_ast_from_stack(parser_ctx);

            simplejs_printf("%u\n", parser_ctx->state);
            break;
        }

        case SIMPLEJS_PARSER_STATE_RETURN_EXPRESSION:
        {
            current_token = current_token->prev;

            parser_ctx->state = SIMPLEJS_PARSER_STATE_RETURN_END;
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_expression_ast(parser_ctx, regular_end_operators, sizeof(regular_end_operators)), result, status);
            break;
        }

        case SIMPLEJS_PARSER_STATE_RETURN_END:
        {
            current_token = current_token->prev;

            simplejs_pop_ast_from_stack(parser_ctx);
            break;
        }

        default:
            SIMPLEJS_ASSERT("cannot stop here" && false);
            break;
        }

    skip_token:
        current_token = current_token->next;
    }

    *out = parser_ctx;

result:
    if (!SIMPLEJS_SUCCESS(status) && parser_ctx)
    {
        simplejs_free_parser_ctx(parser_ctx);
    }
    else
    {
        simplejs_dump_ast_tree(parser_ctx);
    }

    return status;
}
