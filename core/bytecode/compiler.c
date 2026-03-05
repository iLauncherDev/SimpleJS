#include <compiler.h>

bool simplejs_get_scoped_var_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out);
bool simplejs_get_scoped_label_callback(simplejs_ast_scope_context_t *scope_context, void *context, void *out);
bool simplejs_get_scoped_output_alt(
    simplejs_parser_ctx_t *parser_ctx, simplejs_ast_node_t *node, void *context, void *out,
    bool (*callback)(simplejs_ast_scope_context_t *scope_context, void *context, void *out));

static uint32_t simplejs_align_to_cacheline(uint32_t offset)
{
    uint32_t size = (16 - 1);

    return (offset + size) & ~size;
}

simplejs_status_t simplejs_alloc_instruction(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_instruction_t instruction, simplejs_compiler_instruction_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    *ret = instruction;
    simplejs_init_list_entry(&ret->list_entry, ret);

    *out = ret;

result:
    return status;
}

void simplejs_insert_instruction(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_instruction_t *instruction)
{
    simplejs_insert_tail_list(&compiler_ctx->instruction_list, &instruction->list_entry);
    compiler_ctx->instruction_list_count++;
}

simplejs_status_t simplejs_add_instruction(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_instruction_t instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t *ret;

    status = simplejs_alloc_instruction(compiler_ctx, instruction, &ret);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    *ret = instruction;
    simplejs_init_list_entry(&ret->list_entry, ret);

    simplejs_insert_instruction(compiler_ctx, ret);

result:
    return status;
}

bool simplejs_reuse_symbol(simplejs_compiler_ctx_t *compiler_ctx, uint8_t opcode, simplejs_compiler_instruction_t *out)
{
    simplejs_list_entry_t *end_instruction = &compiler_ctx->instruction_list;
    simplejs_list_entry_t *current_instruction = end_instruction->next;

    simplejs_ast_node_t *left = out->symbol.node;

    SIMPLEJS_ASSERT(left != NULL);

    while (current_instruction != end_instruction)
    {
        simplejs_compiler_instruction_t *compiler_instruction = simplejs_get_list_entry_structure(current_instruction);
        simplejs_bytecode_instruction_t *instruction = &compiler_instruction->instruction;
        if (instruction->opcode != opcode)
            goto skip_instruction;

        uint32_t right_data_offset = compiler_instruction->symbol.data_offset;
        simplejs_ast_node_t *right = compiler_instruction->symbol.node;

        SIMPLEJS_ASSERT(right != NULL);

        switch (left->type)
        {
        case SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE:
        case SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE:
        {
            simplejs_utf8_string_t *left_string = left->context;
            simplejs_utf8_string_t *right_string = right->context;

            if (!strcmp(left_string->buffer, right_string->buffer))
            {
                out->symbol.node = right;
                out->symbol.data_offset = right_data_offset;
                return true;
            }

            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_NUMBER:
        {
            simplejs_number_t *left_number = left->context;
            simplejs_number_t *right_number = right->context;

            if (!memcmp(left_number, right_number, sizeof(*left_number)))
            {
                out->symbol.node = right;
                out->symbol.data_offset = right_data_offset;
                return true;
            }

            break;
        }

        default:
            SIMPLEJS_ASSERT("cannot reuse symbol" && false);
            break;
        }

    skip_instruction:
        current_instruction = current_instruction->next;
    }

    return false;
}

simplejs_bytecode_opcode_t binary_ast_opcode_map[SIMPLEJS_AST_NODE_TYPE_END] = {
    [SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL] = SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_GREATER] = SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_BELOW] = SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_ADD] = SIMPLEJS_BYTECODE_OPCODE_ADD_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_SUB] = SIMPLEJS_BYTECODE_OPCODE_SUB_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_MUL] = SIMPLEJS_BYTECODE_OPCODE_MUL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_DIV] = SIMPLEJS_BYTECODE_OPCODE_DIV_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_MOD] = SIMPLEJS_BYTECODE_OPCODE_MOD_VAR,
};

simplejs_status_t simplejs_compile_ast_operation(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compile_reg_info_t reg_info, simplejs_ast_node_t *side)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t instruct_tmp = {0};

    switch (side->type)
    {
    case SIMPLEJS_AST_NODE_TYPE_EXPRESSION:
    {
        if (!side->children_list_count)
            break;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, reg_info, left), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_VARDECL:
    {
        if (!side->children_list_count)
            break;

        simplejs_ast_var_context_t *var_context = side->context;

        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(side->children_list_entry.next);

        simplejs_compile_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR;
        instruct_tmp.instruction.reg_1 = tmp_reg_info.reg_a;
        instruct_tmp.instruction.imm = var_context->index;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_FUNCTION_CALL:
    {
        SIMPLEJS_ASSERT(side->children_list_count >= 1);

        uint32_t argument_index = 0;
        uint32_t argument_count = side->children_list_count - 1;

        simplejs_ast_node_t *reference = simplejs_get_list_entry_structure(side->children_list_entry.next);

        simplejs_list_entry_t *end_argument = &side->children_list_entry;
        simplejs_list_entry_t *current_argument = reference->list_entry.next;

        simplejs_compile_reg_info_t tmp_reg_info;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_ARG_OFFSET;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS;
        instruct_tmp.instruction.imm = argument_count;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        while (current_argument != end_argument)
        {
            simplejs_ast_node_t *argument = simplejs_get_list_entry_structure(current_argument);

            memclr(&tmp_reg_info, sizeof(tmp_reg_info));
            tmp_reg_info.is_write = false;
            tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;
            tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, argument), result, status);

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_ARG_VAR;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;
            instruct_tmp.instruction.imm = argument_index;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

            current_argument = current_argument->next;
            argument_index++;
        }

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, reference), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_CALL;
        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS;
        instruct_tmp.instruction.imm = argument_count;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_MOV_VAR;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;
        instruct_tmp.instruction.reg_2 = SIMPLEJS_BYTECODE_VARIABLE_FUNC_RETURN;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        simplejs_compile_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, left), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));

        if (reg_info.is_write)
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP;
        }
        else
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP;
        }

        instruct_tmp.instruction.reg_1 = reg_info.reg_a;
        instruct_tmp.instruction.reg_2 = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;
        instruct_tmp.instruction.imm = reg_info.reg_b;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_NUMBER:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.node = side;
        instruct_tmp.symbol.data_offset = compiler_ctx->data_offset;

        if (!simplejs_reuse_symbol(compiler_ctx, SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER, &instruct_tmp))
            compiler_ctx->data_offset += simplejs_align_to_cacheline(SIMPLEJS_NUMBER_SIZE);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE:
    {
        simplejs_utf8_string_t *string = side->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING;

        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.node = side;
        instruct_tmp.symbol.data_offset = compiler_ctx->data_offset;

        if (!simplejs_reuse_symbol(compiler_ctx, SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING, &instruct_tmp))
            compiler_ctx->data_offset += simplejs_align_to_cacheline(string->valid_size + 1);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE:
    {
        simplejs_utf8_string_t *string = side->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.label_id = (uintptr_t)string->buffer;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        if (reg_info.is_write)
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_b;
        }
        else
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
        }

        instruct_tmp.instruction.imm = (uint32_t)((uintptr_t)side->context);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR;
        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_GLOBAL;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        simplejs_utf8_string_t *string = side->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING;
        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;

        instruct_tmp.symbol.node = side;
        instruct_tmp.symbol.data_offset = compiler_ctx->data_offset;

        if (!simplejs_reuse_symbol(compiler_ctx, SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING, &instruct_tmp))
            compiler_ctx->data_offset += simplejs_align_to_cacheline(string->valid_size + 1);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        if (reg_info.is_write)
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP;

            instruct_tmp.instruction.imm = reg_info.reg_b;
        }
        else
        {
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP;

            instruct_tmp.instruction.imm = reg_info.reg_a;
        }

        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_GLOBAL;
        instruct_tmp.instruction.reg_2 = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_GREATER:
    case SIMPLEJS_AST_NODE_TYPE_ALU_BELOW:

    case SIMPLEJS_AST_NODE_TYPE_ALU_ADD:
    case SIMPLEJS_AST_NODE_TYPE_ALU_SUB:
    case SIMPLEJS_AST_NODE_TYPE_ALU_MUL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_DIV:
    case SIMPLEJS_AST_NODE_TYPE_ALU_MOD:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_bytecode_opcode_t selected_binary_opcode = binary_ast_opcode_map[side->type];
        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        simplejs_compile_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_OP_A;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_OP_A;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, left), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_OP_B;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_OP_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = selected_binary_opcode;

        instruct_tmp.instruction.imm = reg_info.reg_a;
        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_OP_A;
        instruct_tmp.instruction.reg_2 = SIMPLEJS_BYTECODE_VARIABLE_OP_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ASSIGN:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        simplejs_compile_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = false;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, right), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.is_write = true;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, tmp_reg_info, left), result, status);

        break;
    }

    default:
        SIMPLEJS_ASSERT("unknown side->type" && false);
        break;
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_single_ast(simplejs_compiler_ctx_t *compile_ctx, simplejs_ast_node_t *ast);

simplejs_status_t simplejs_compile_ast_branch(
    simplejs_compiler_ctx_t *compile_ctx, simplejs_ast_node_t *branch_node,
    uintptr_t end_id, uintptr_t next_id)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (branch_node->type == SIMPLEJS_AST_NODE_TYPE_BRANCH)
    {
        uintptr_t label_id = (uintptr_t)branch_node;

        simplejs_list_entry_t *end_ast = &branch_node->children_list_entry;
        simplejs_list_entry_t *current_ast = end_ast->next;

        while (current_ast != end_ast)
        {
            simplejs_list_entry_t *next_ast = current_ast->next;
            simplejs_ast_node_t *statement_ast = simplejs_get_list_entry_structure(current_ast);

            next_id = (uintptr_t)simplejs_get_list_entry_structure(next_ast);
            if (next_ast == end_ast)
                next_id = label_id;

            status = simplejs_compile_ast_branch(compile_ctx, statement_ast, label_id, next_id);
            if (!SIMPLEJS_SUCCESS(status))
            {
                goto result;
            }

            current_ast = current_ast->next;
        }

        simplejs_compiler_instruction_t instruct_tmp = {0};

        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = label_id;
        simplejs_add_instruction(compile_ctx, instruct_tmp);
    }
    else
    {
        simplejs_compiler_instruction_t instruct_tmp = {0};

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = (uintptr_t)branch_node;
        simplejs_add_instruction(compile_ctx, instruct_tmp);

        switch (branch_node->type)
        {
        case SIMPLEJS_AST_NODE_TYPE_IF:
        case SIMPLEJS_AST_NODE_TYPE_ELSE_IF:
        {
            SIMPLEJS_ASSERT(branch_node->children_list_count >= 1);

            simplejs_list_entry_t *end_ast = &branch_node->children_list_entry;
            simplejs_list_entry_t *expression_ast = end_ast->next;
            simplejs_list_entry_t *current_ast = expression_ast->next;

            simplejs_ast_node_t *expression_node = simplejs_get_list_entry_structure(expression_ast);

            simplejs_compile_reg_info_t reg_info = {0};

            reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
            reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compile_ctx, reg_info, expression_node), result, status);

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
            instruct_tmp.symbol.label_id = next_id;
            simplejs_add_instruction(compile_ctx, instruct_tmp);

            while (current_ast != end_ast)
            {
                simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

                simplejs_compile_single_ast(compile_ctx, ast);

                current_ast = current_ast->next;
            }

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
            instruct_tmp.symbol.label_id = end_id;
            simplejs_add_instruction(compile_ctx, instruct_tmp);

            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_ELSE:
        {
            simplejs_list_entry_t *end_ast = &branch_node->children_list_entry;
            simplejs_list_entry_t *current_ast = end_ast->next;

            while (current_ast != end_ast)
            {
                simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

                simplejs_compile_single_ast(compile_ctx, ast);

                current_ast = current_ast->next;
            }

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
            instruct_tmp.symbol.label_id = end_id;
            simplejs_add_instruction(compile_ctx, instruct_tmp);

            break;
        }

        default:
            SIMPLEJS_ASSERT("unexpected branch node!" && false);
            break;
        }
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_ast(simplejs_compiler_ctx_t *compile_ctx, simplejs_list_entry_t *list);

simplejs_status_t simplejs_compile_single_ast(simplejs_compiler_ctx_t *compile_ctx, simplejs_ast_node_t *ast)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_compile_reg_info_t reg_info = {0};

    reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
    reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

    switch (ast->type)
    {
    case SIMPLEJS_AST_NODE_TYPE_VARDECL:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compile_ctx, reg_info, ast), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_EXPRESSION:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compile_ctx, reg_info, ast), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_LABEL:
    {
        simplejs_compiler_instruction_t instruct_tmp = {0};
        simplejs_ast_label_context_t *label_context = ast->context;

        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = (uintptr_t)label_context->name->buffer;
        simplejs_add_instruction(compile_ctx, instruct_tmp);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_GOTO:
    {
        simplejs_compiler_instruction_t instruct_tmp = {0};

        simplejs_ast_label_context_t *label_context = NULL;
        if (!simplejs_get_scoped_output_alt(compile_ctx->parser_ctx, ast, ast->context, &label_context, simplejs_get_scoped_label_callback))
        {
            simplejs_printf("goto label not found on compile time!\n");

            status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
            goto result;
        }

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
        instruct_tmp.symbol.label_id = (uintptr_t)label_context->name->buffer;
        simplejs_add_instruction(compile_ctx, instruct_tmp);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_BRANCH:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_branch(compile_ctx, ast, 0, 0), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_CODEBLOCK:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast(compile_ctx, &ast->children_list_entry), result, status);
        break;
    }
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_ast(simplejs_compiler_ctx_t *compile_ctx, simplejs_list_entry_t *list)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_list_entry_t *end_ast = list;
    simplejs_list_entry_t *current_ast = end_ast->next;

    while (current_ast != end_ast)
    {
        simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

        status = simplejs_compile_single_ast(compile_ctx, ast);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }

        current_ast = current_ast->next;
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_ast_function(simplejs_compiler_ctx_t *compile_ctx, simplejs_ast_node_t *function_node, bool is_main)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_function_context_t *function_context = function_node->context;

    simplejs_compiler_instruction_t instruct_tmp = {0};

    if (function_context->name)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_FUNCTION;
        instruct_tmp.symbol.label_id = (uintptr_t)function_context->name->buffer;
        simplejs_add_instruction(compile_ctx, instruct_tmp);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET;
    simplejs_add_instruction(compile_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET;
    simplejs_add_instruction(compile_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE;
    instruct_tmp.instruction.imm = function_context->local_var_count;
    simplejs_add_instruction(compile_ctx, instruct_tmp);

    for (size_t i = 0; i < function_context->local_var_count; i++)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_VAR;
        instruct_tmp.instruction.imm = i;
        simplejs_add_instruction(compile_ctx, instruct_tmp);
    }

    status = simplejs_compile_ast(compile_ctx, &function_node->children_list_entry);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    for (size_t i = 0; i < function_context->local_var_count; i++)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_FREE_LOC_VAR;
        instruct_tmp.instruction.imm = i;
        simplejs_add_instruction(compile_ctx, instruct_tmp);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE;
    instruct_tmp.instruction.imm = -function_context->local_var_count;
    simplejs_add_instruction(compile_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    if (is_main)
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_EXIT;
    else
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RETURN;

    simplejs_add_instruction(compile_ctx, instruct_tmp);

result:
    return status;
}

void simplejs_disasm_bytecode(simplejs_bytecode_instruction_t instruction, uintptr_t instruction_pointer)
{
    switch (instruction.opcode)
    {
    case SIMPLEJS_BYTECODE_OPCODE_MOV_VAR:
        simplejs_printf("mov_var v%u, v%u", instruction.reg_1, instruction.reg_2);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_ADD_STACK:
        simplejs_printf("add_stack %d", instruction.imm_signed);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE:
        simplejs_printf("add_stack_var_size %d", instruction.imm_signed);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX:
        simplejs_printf("save_ctx");
        break;
    case SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX:
        simplejs_printf("restore_ctx");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET:
        simplejs_printf("init_loc_offset");
        break;
    case SIMPLEJS_BYTECODE_OPCODE_INIT_ARG_OFFSET:
        simplejs_printf("init_arg_offset");
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET:
        simplejs_printf("save_arg_offset");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS:
        simplejs_printf("alloc_args %u", instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS:
        simplejs_printf("free_args %u", instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_VAR:
        simplejs_printf("init_loc_var %u", instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_FREE_LOC_VAR:
        simplejs_printf("free_loc_var %u", instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP:
        simplejs_printf("get_var_prop v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP:
        simplejs_printf("set_var_prop v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR:
        simplejs_printf("get_global_var v%u", instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR:
        simplejs_printf("get_loc_var v%u, %u", instruction.reg_1, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR:
        simplejs_printf("set_loc_var %u, v%u", instruction.imm, instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_ARG_VAR:
        simplejs_printf("get_arg_var v%u, %u", instruction.reg_1, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_ARG_VAR:
        simplejs_printf("set_arg_var %u, v%u", instruction.imm, instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER:
    {
        simplejs_number_t temp_number;
        simplejs_number_decode(&temp_number, (void *)(instruction_pointer + instruction.imm_signed));

        double value = simplejs_number_get_float64_jumptable[temp_number.type](&temp_number);

        simplejs_printf("set_var_number v%u, (ip relative %f) %d", instruction.reg_1, value, instruction.imm_signed);
        break;
    }
    case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION:
        simplejs_printf("set_var_native_function v%u, (ip relative) %d", instruction.reg_1, instruction.imm_signed);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING:
        simplejs_printf("set_var_fast_string v%u, (ip relative '%s') %d",
                        instruction.reg_1, (char *)(instruction_pointer + instruction.imm_signed), instruction.imm_signed);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_CALL:
        simplejs_printf("call v%u", instruction.reg_1);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_RETURN:
        simplejs_printf("return");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_JMP:
        simplejs_printf("jmp ");

        if (instruction.imm_signed > 0)
            simplejs_printf("0x%d", instruction.imm_signed);
        else
            simplejs_printf("-0x%x", -instruction.imm_signed);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO:
        simplejs_printf("jmp_if_zero v%u, ", instruction.reg_1);

        if (instruction.imm_signed > 0)
            simplejs_printf("0x%d", instruction.imm_signed);
        else
            simplejs_printf("-0x%x", -instruction.imm_signed);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_EXIT:
        simplejs_printf("exit");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_INC_VAR:
        simplejs_printf("inc_var v%u", instruction.reg_1);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_DEC_VAR:
        simplejs_printf("dec_var v%u", instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR:
        simplejs_printf("equal_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR:
        simplejs_printf("greater_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR:
        simplejs_printf("below_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_ADD_VAR:
        simplejs_printf("add_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SUB_VAR:
        simplejs_printf("sub_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_MUL_VAR:
        simplejs_printf("mul_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_DIV_VAR:
        simplejs_printf("div_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_MOD_VAR:
        simplejs_printf("mod_var v%u, v%u, v%u", instruction.reg_1, instruction.reg_2, instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_END:
        simplejs_printf("// end of code!");
        break;
    }

    simplejs_printf("\n");
}

void simplejs_ast_dump_bytecode(simplejs_compiler_ctx_t *compiler_ctx)
{
    simplejs_bytecode_header_t *header = (void *)compiler_ctx->executable;
    uint16_t header_size = ((uint16_t)header->size_low << 0) | ((uint16_t)header->size_high << 8);

    uint8_t *instruction_pointer = (uint8_t *)header + header_size;

    while (true)
    {
        simplejs_printf("/* ip: 0x%08x */ ", (uint32_t)((uintptr_t)instruction_pointer - (uintptr_t)header));

        uint8_t instruction_size;
        simplejs_bytecode_instruction_t instruction;
        simplejs_bytecode_decode(&instruction, instruction_pointer, &instruction_size);

        instruction_pointer += instruction_size;

        simplejs_disasm_bytecode(instruction, (uintptr_t)instruction_pointer);

        if (instruction.opcode == SIMPLEJS_BYTECODE_OPCODE_END)
            break;
    }
}

static uint8_t get_signed_int_size(uint32_t value)
{
    value &= ~(1 << (sizeof(value) * 8 - 1));

    if (value >= 0x10000)
        return 4;

    if (value >= 0x100)
        return 2;

    return 1;
}

static bool simplejs_get_label_offset(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_instruction_type_t type, uintptr_t label_id, uint32_t *out)
{
    simplejs_list_entry_t *end_instruction = &compiler_ctx->instruction_list;
    simplejs_list_entry_t *current_instruction = end_instruction->next;

    while (current_instruction != end_instruction)
    {
        simplejs_compiler_instruction_t *compiler_instruction = simplejs_get_list_entry_structure(current_instruction);
        if (compiler_instruction->type != type)
            goto skip_instruction;

        if (compiler_instruction->symbol.label_id == label_id)
        {
            printf("found label!\n");

            *out = compiler_instruction->symbol.data_offset;
            return true;
        }

    skip_instruction:
        current_instruction = current_instruction->next;
    }

    simplejs_printf("label not found!\n");
    return false;
}

simplejs_status_t simplejs_compile_instructions(simplejs_compiler_ctx_t *compiler_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    uint32_t total_size = 0;
    uint32_t header_size = simplejs_align_to_cacheline(SIMPLEJS_BYTECODE_HEADER_SIZE);
    uint32_t header_offset = 0;
    uint32_t code_offset = 0;
    uint32_t data_offset = 0;

    uint32_t current_code_offset = 0;

    total_size += header_size;
    code_offset = total_size;

    total_size += simplejs_align_to_cacheline(compiler_ctx->instruction_list_count * SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE);
    data_offset = total_size;

    total_size += simplejs_align_to_cacheline(compiler_ctx->data_offset);

    uint8_t *buffer = simplejs_hook_malloc(total_size);
    if (!buffer)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(buffer, total_size);

    simplejs_bytecode_header_t *header = (void *)buffer;

    header->size_low = (header_size >> 0) & 0xFF;
    header->size_high = (header_size >> 8) & 0xFF;
    header->version_low = (SIMPLEJS_BYTECODE_VERSION >> 0) & 0xFF;
    header->version_high = (SIMPLEJS_BYTECODE_VERSION >> 8) & 0xFF;

    uintptr_t repeat_count = 1;

    while (repeat_count--)
    {
    repeat_compile_loop:
        current_code_offset = code_offset;

        simplejs_list_entry_t *end_instruction = &compiler_ctx->instruction_list;
        simplejs_list_entry_t *current_instruction = end_instruction->next;

        printf("new iteration\n");

        while (current_instruction != end_instruction)
        {
            simplejs_compiler_instruction_t *compiler_instruction = simplejs_get_list_entry_structure(current_instruction);
            if (compiler_instruction->type != SIMPLEJS_COMPILER_INSTRUCTION_TYPE_NORMAL)
            {
                printf("(0x%04x) label %p\n", current_code_offset, (void *)compiler_instruction->symbol.label_id);

                if (compiler_instruction->symbol.data_offset != current_code_offset)
                    repeat_count++;

                compiler_instruction->symbol.data_offset = current_code_offset;
                goto skip_instruction;
            }

            uint8_t instruction_size;
            simplejs_bytecode_instruction_t *instruction = &compiler_instruction->instruction;

            switch (instruction->opcode)
            {
            case SIMPLEJS_BYTECODE_OPCODE_JMP:
            case SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO:
            {
                uint32_t absolute_offset = 0;
                bool label_result = simplejs_get_label_offset(compiler_ctx, SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO, compiler_instruction->symbol.label_id, &absolute_offset);
                SIMPLEJS_ASSERT(label_result == true);

                printf("(0x%04x) absolute jump offset 0x%x\n", current_code_offset, absolute_offset);

                simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);

                int32_t relative_offset = absolute_offset - (current_code_offset + instruction_size);
                bool extended_opcode = get_signed_int_size(relative_offset) > 2;

                if (instruction->extended_opcode != extended_opcode)
                {
                    instruction->extended_opcode = extended_opcode;
                    goto repeat_compile_loop;
                }

                instruction->imm_signed = relative_offset;
                break;
            }

            case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER:
            {
                uint32_t absolute_offset = data_offset + compiler_instruction->symbol.data_offset;

                simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);

                int32_t relative_offset = absolute_offset - (current_code_offset + instruction_size);
                bool extended_opcode = get_signed_int_size(relative_offset) > 2;

                if (instruction->extended_opcode != extended_opcode)
                {
                    instruction->extended_opcode = extended_opcode;
                    goto repeat_compile_loop;
                }

                simplejs_number_t *number = compiler_instruction->symbol.node->context;

                instruction->imm_signed = relative_offset;

                simplejs_number_encode(&buffer[absolute_offset], number);
                break;
            }

            case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION:
            {
                uint32_t absolute_offset = 0;
                bool label_result = simplejs_get_label_offset(compiler_ctx, SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_FUNCTION, compiler_instruction->symbol.label_id, &absolute_offset);
                SIMPLEJS_ASSERT(label_result == true);

                simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);

                int32_t relative_offset = absolute_offset - (current_code_offset + instruction_size);
                bool extended_opcode = get_signed_int_size(relative_offset) > 2;

                if (instruction->extended_opcode != extended_opcode)
                {
                    instruction->extended_opcode = extended_opcode;
                    goto repeat_compile_loop;
                }

                instruction->imm_signed = relative_offset;
                break;
            }

            case SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING:
            {
                uint32_t absolute_offset = data_offset + compiler_instruction->symbol.data_offset;

                simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);

                int32_t relative_offset = absolute_offset - (current_code_offset + instruction_size);
                bool extended_opcode = get_signed_int_size(relative_offset) > 2;

                if (instruction->extended_opcode != extended_opcode)
                {
                    instruction->extended_opcode = extended_opcode;
                    goto repeat_compile_loop;
                }

                simplejs_utf8_string_t *string = compiler_instruction->symbol.node->context;

                instruction->imm_signed = relative_offset;

                memcpy(&buffer[absolute_offset], string->buffer, string->valid_size);
                break;
            }
            }

            simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);
            current_code_offset += instruction_size;

        skip_instruction:
            current_instruction = current_instruction->next;
        }
    }

    compiler_ctx->executable = buffer;
    compiler_ctx->executable_size = total_size;

result:
    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_ast_to_bytecode(simplejs_parser_ctx_t *parser_ctx, simplejs_compiler_ctx_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_ctx_t *compiler_ctx = simplejs_hook_malloc(sizeof(*compiler_ctx));
    if (!compiler_ctx)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(compiler_ctx, sizeof(*compiler_ctx));

    compiler_ctx->parser_ctx = parser_ctx;
    simplejs_init_list_entry(&compiler_ctx->instruction_list, compiler_ctx);

    status = simplejs_compile_ast_function(compiler_ctx, parser_ctx->root_ast, true);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    simplejs_list_entry_t *end_ast = &parser_ctx->ast_function_list;
    simplejs_list_entry_t *current_ast = end_ast->next;

    while (current_ast != end_ast)
    {
        simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);
        if (ast == parser_ctx->root_ast)
            goto skip_ast_function;

        status = simplejs_compile_ast_function(compiler_ctx, ast, false);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }

    skip_ast_function:
        current_ast = current_ast->next;
    }

    simplejs_compiler_instruction_t instruct_tmp = {0};

    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_END;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

    status = simplejs_compile_instructions(compiler_ctx);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    *out = compiler_ctx;

result:
    if (SIMPLEJS_SUCCESS(status))
    {
        simplejs_ast_dump_bytecode(compiler_ctx);
    }

    return status;
}
