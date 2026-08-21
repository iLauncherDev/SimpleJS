#include <compiler.h>

static uint32_t simplejs_align_to_cacheline(uint32_t offset)
{
    uint32_t size = (16 - 1);

    return (offset + size) & ~size;
}

simplejs_status_t simplejs_alloc_instruction(simplejs_compiler_instruction_t instruction, simplejs_compiler_instruction_t **out)
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

simplejs_status_t simplejs_alloc_debug(simplejs_compiler_debug_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_debug_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    simplejs_init_list_entry(&ret->children_list_entry, ret);
    simplejs_init_list_entry(&ret->_temp_list_entry, ret);
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

uint32_t simplejs_count_total_debug(simplejs_compiler_debug_t *parent_debug)
{
    uint32_t count = 1;

    simplejs_list_entry_t *end_debug = &parent_debug->children_list_entry;
    simplejs_list_entry_t *current_debug = end_debug->next;

    while (current_debug != end_debug)
    {
        simplejs_compiler_debug_t *debug = simplejs_get_list_entry_structure(current_debug);

        count += simplejs_count_total_debug(debug);

        current_debug = current_debug->next;
    }

    return count;
}

void simplejs_free_compile_debug_list(simplejs_list_entry_t *parent_list_entry)
{
    simplejs_list_entry_t *end_debug = parent_list_entry;
    simplejs_list_entry_t *current_debug = end_debug->next;

    while (current_debug != end_debug)
    {
        simplejs_list_entry_t *next_debug = current_debug->next;
        simplejs_compiler_debug_t *compiler_debug = simplejs_get_list_entry_structure(current_debug);

        simplejs_free_compile_debug_list(&compiler_debug->children_list_entry);
        simplejs_hook_mfree(compiler_debug);

        current_debug = next_debug;
    }
}

void simplejs_insert_debug(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_debug_t *parent, simplejs_compiler_debug_t *debug)
{
    SIMPLEJS_ASSERT(debug != NULL);

    if (parent)
    {
        simplejs_insert_tail_list(&parent->children_list_entry, &debug->list_entry);

        parent->children_list_count++;
    }
    else
    {
        simplejs_insert_tail_list(&compiler_ctx->debug_list, &debug->list_entry);

        compiler_ctx->debug_list_count++;
        compiler_ctx->debug_list_total_count += simplejs_count_total_debug(debug);
    }
}

simplejs_status_t simplejs_add_instruction(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_instruction_t instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t *ret;

    status = simplejs_alloc_instruction(instruction, &ret);
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
        case SIMPLEJS_AST_NODE_TYPE_STRING:
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

simplejs_bytecode_opcode_t op_ast_opcode_map[SIMPLEJS_AST_NODE_TYPE_END] = {
    [SIMPLEJS_AST_NODE_TYPE_ALU_INC] = SIMPLEJS_BYTECODE_OPCODE_INC_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_DEC] = SIMPLEJS_BYTECODE_OPCODE_DEC_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_LOGICAL_NOT] = SIMPLEJS_BYTECODE_OPCODE_LOGICAL_NOT_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_BITWISE_NOT] = SIMPLEJS_BYTECODE_OPCODE_BITWISE_NOT_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_NEG] = SIMPLEJS_BYTECODE_OPCODE_NEG_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL] = SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_NOT_EQUAL] = SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_GREATER] = SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_BELOW] = SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_GREATER_EQUAL] = SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_BELOW_EQUAL] = SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_OR] = SIMPLEJS_BYTECODE_OPCODE_OR_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_AND] = SIMPLEJS_BYTECODE_OPCODE_AND_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_SHL] = SIMPLEJS_BYTECODE_OPCODE_SHL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_SHR] = SIMPLEJS_BYTECODE_OPCODE_SHR_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_SAL] = SIMPLEJS_BYTECODE_OPCODE_SAL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_SAR] = SIMPLEJS_BYTECODE_OPCODE_SAR_VAR,

    [SIMPLEJS_AST_NODE_TYPE_ALU_ADD] = SIMPLEJS_BYTECODE_OPCODE_ADD_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_SUB] = SIMPLEJS_BYTECODE_OPCODE_SUB_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_MUL] = SIMPLEJS_BYTECODE_OPCODE_MUL_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_DIV] = SIMPLEJS_BYTECODE_OPCODE_DIV_VAR,
    [SIMPLEJS_AST_NODE_TYPE_ALU_MOD] = SIMPLEJS_BYTECODE_OPCODE_MOD_VAR,
};

#define simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, label, status) \
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_debug(&compiler_debug), result, status);             \
    compiler_debug->diagnostic_token = side->diagnostic_token;                                   \
    compiler_debug->diagnostic_offset = side->diagnostic_offset;                                 \
    simplejs_insert_tail_list(&compiler_debug_list, &compiler_debug->_temp_list_entry)

#define simplejs_alloc_and_insert_debug(compiler_debug, side, parent_debug, label, status) \
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_alloc_debug(&compiler_debug), result, status);       \
    compiler_debug->diagnostic_token = side->diagnostic_token;                             \
    compiler_debug->diagnostic_offset = side->diagnostic_offset;                           \
    simplejs_insert_debug(compiler_ctx, parent_debug, compiler_debug)

simplejs_status_t simplejs_compile_ast_operation(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_debug_t *parent_debug, simplejs_compiler_reg_info_t reg_info, simplejs_ast_node_t *side)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_list_entry_t compiler_debug_list;
    simplejs_init_list_entry(&compiler_debug_list, &compiler_debug_list);

    simplejs_compiler_instruction_t instruct_tmp = {0};
    if (!reg_info.have_parent)
        reg_info.reg_parent = reg_info.reg_a;

    switch (side->type)
    {
    case SIMPLEJS_AST_NODE_TYPE_EXPRESSION:
    {
        if (!side->children_list_count)
            break;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, reg_info, left), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_VARDECL:
    {
        if (!side->children_list_count)
            break;

        simplejs_ast_var_context_t *var_context = side->context;

        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_compiler_reg_info_t tmp_reg_info;

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;
        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

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

        simplejs_compiler_reg_info_t tmp_reg_info;

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, reference, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, reference, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;
        instruct_tmp.instruction.imm = argument_count;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        while (current_argument != end_argument)
        {
            simplejs_ast_node_t *argument = simplejs_get_list_entry_structure(current_argument);

            simplejs_compiler_debug_t *argument_compiler_debug;
            simplejs_alloc_and_insert_debug(argument_compiler_debug, argument, compiler_debug, result, status);

            memclr(&tmp_reg_info, sizeof(tmp_reg_info));
            tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
            tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;
            tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, argument_compiler_debug, tmp_reg_info, argument), result, status);

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.compiler_debug = argument_compiler_debug;

            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_CALL_ARG_VAR;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_ARGUMENT;
            instruct_tmp.instruction.imm = argument_index;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

            current_argument = current_argument->next;
            argument_index++;
        }

        if (reference->type != SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS)
        {
            memclr(&instruct_tmp, sizeof(instruct_tmp));
            simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, reference, compiler_debug, result, status);

            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_VAR;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_THIS;
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        }

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.reg_parent = SIMPLEJS_BYTECODE_VARIABLE_THIS;
        tmp_reg_info.have_parent = true;

        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;
        tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, reference), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_CALL;
        instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_FUNCTION;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, reference, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, reference, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_PROPERTY_ACCESS:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        simplejs_compiler_reg_info_t tmp_reg_info;

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_BINARY_OP_FLAG;
        compiler_debug->flags |= SIMPLEJS_BYTECODE_DEBUG_INFO_HINT_RIGHT_FLAG;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_LEFT_FLAG;

        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = reg_info.reg_parent;
        tmp_reg_info.reg_b = reg_info.reg_parent;

        if (!reg_info.avoid_refetch)
            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.reg_1 = reg_info.reg_parent;
        instruct_tmp.instruction.reg_2 = SIMPLEJS_BYTECODE_VARIABLE_PROPERTY;
        instruct_tmp.instruction.imm = reg_info.reg_b;

        switch (reg_info.operation_type)
        {
        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP;
            break;

        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP;
            break;

        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_DELETE:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_DELETE_VAR_PROP;

            instruct_tmp.instruction.imm = 0;
            break;

        default:
            break;
        }

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_NUMBER:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.node = side;
        instruct_tmp.symbol.data_offset = compiler_ctx->data_offset;

        if (!simplejs_reuse_symbol(compiler_ctx, SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER, &instruct_tmp))
            compiler_ctx->data_offset += SIMPLEJS_NUMBER_SIZE;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_STRING:
    case SIMPLEJS_AST_NODE_TYPE_PROPERTY_REFERENCE:
    {
        simplejs_utf8_string_t *string = side->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.node = side;
        instruct_tmp.symbol.data_offset = compiler_ctx->data_offset;

        if (!simplejs_reuse_symbol(compiler_ctx, SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING, &instruct_tmp))
            compiler_ctx->data_offset += string->valid_size + 1;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_FUNCTION_REFERENCE:
    {
        simplejs_utf8_string_t *string = side->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        instruct_tmp.symbol.label_id = (uintptr_t)string->buffer;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ARGUMENT_REFERENCE:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        switch (reg_info.operation_type)
        {
        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_FUNC_ARG_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
            break;

        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_FUNC_ARG_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_b;
            break;
        default:
            break;
        }

        instruct_tmp.instruction.imm = (uint32_t)((uintptr_t)side->context);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_LOCAL_REFERENCE:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        switch (reg_info.operation_type)
        {
        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
            break;

        case SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE:
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR;

            instruct_tmp.instruction.reg_1 = reg_info.reg_b;
            break;
        default:
            break;
        }

        instruct_tmp.instruction.imm = (uint32_t)((uintptr_t)side->context);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_GLOBAL_REFERENCE:
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_temp(instruct_tmp.compiler_debug, side, compiler_debug_list, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_LOGICAL_OR:
    case SIMPLEJS_AST_NODE_TYPE_LOGICAL_AND:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_BINARY_OP_FLAG;

        uintptr_t short_label_id = (uintptr_t)side;

        simplejs_compiler_reg_info_t tmp_reg_info;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        simplejs_bytecode_opcode_t selected_jump_opcode;

        switch (side->type)
        {
        case SIMPLEJS_AST_NODE_TYPE_LOGICAL_OR:
            selected_jump_opcode = SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO;
            break;

        case SIMPLEJS_AST_NODE_TYPE_LOGICAL_AND:
            selected_jump_opcode = SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO;
            break;

        default:
            SIMPLEJS_ASSERT("unhandled logical operator" && false);
            break;
        }

        uint8_t reg_op_a = SIMPLEJS_BYTECODE_VARIABLE_OP_A;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_LEFT_FLAG;

        tmp_reg_info.is_sub_op = !reg_info.is_sub_op;
        tmp_reg_info.reg_a = reg_op_a;
        tmp_reg_info.reg_b = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.opcode = selected_jump_opcode;
        instruct_tmp.instruction.reg_1 = reg_op_a;
        instruct_tmp.symbol.label_id = short_label_id;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.is_sub_op = !reg_info.is_sub_op;
        tmp_reg_info.reg_a = reg_op_a;
        tmp_reg_info.reg_b = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = short_label_id;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;
        instruct_tmp.instruction.reg_2 = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ALU_INC:
    case SIMPLEJS_AST_NODE_TYPE_ALU_DEC:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 1);

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_UNARY_OP_FLAG;

        simplejs_bytecode_opcode_t selected_unary_opcode = op_ast_opcode_map[side->type];
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(side->children_list_entry.next);

        uint8_t reg_op_a = SIMPLEJS_BYTECODE_VARIABLE_OP_A;

        simplejs_compiler_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.is_sub_op = !reg_info.is_sub_op;
        tmp_reg_info.reg_a = reg_op_a;
        tmp_reg_info.reg_b = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        if (side->flags)
        {
            memclr(&instruct_tmp, sizeof(instruct_tmp));
            simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_MOV_VAR;
            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
            instruct_tmp.instruction.reg_2 = reg_op_a;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        }

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.opcode = selected_unary_opcode;
        instruct_tmp.instruction.reg_1 = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        if (!side->flags)
        {
            memclr(&instruct_tmp, sizeof(instruct_tmp));
            simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_MOV_VAR;
            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
            instruct_tmp.instruction.reg_2 = reg_op_a;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        }

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ALU_LOGICAL_NOT:
    case SIMPLEJS_AST_NODE_TYPE_ALU_BITWISE_NOT:
    case SIMPLEJS_AST_NODE_TYPE_ALU_NEG:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 1);

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_UNARY_OP_FLAG;

        simplejs_bytecode_opcode_t selected_unary_opcode = op_ast_opcode_map[side->type];
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(side->children_list_entry.next);

        simplejs_compiler_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.opcode = selected_unary_opcode;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ALU_EQUAL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_NOT_EQUAL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_GREATER:
    case SIMPLEJS_AST_NODE_TYPE_ALU_BELOW:
    case SIMPLEJS_AST_NODE_TYPE_ALU_GREATER_EQUAL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_BELOW_EQUAL:

    case SIMPLEJS_AST_NODE_TYPE_ALU_OR:
    case SIMPLEJS_AST_NODE_TYPE_ALU_AND:

    case SIMPLEJS_AST_NODE_TYPE_ALU_SHL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_SHR:
    case SIMPLEJS_AST_NODE_TYPE_ALU_SAL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_SAR:

    case SIMPLEJS_AST_NODE_TYPE_ALU_ADD:
    case SIMPLEJS_AST_NODE_TYPE_ALU_SUB:
    case SIMPLEJS_AST_NODE_TYPE_ALU_MUL:
    case SIMPLEJS_AST_NODE_TYPE_ALU_DIV:
    case SIMPLEJS_AST_NODE_TYPE_ALU_MOD:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_BINARY_OP_FLAG;

        simplejs_bytecode_opcode_t selected_binary_opcode = op_ast_opcode_map[side->type];
        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        uint8_t reg_op_a = SIMPLEJS_BYTECODE_VARIABLE_OP_A;
        uint8_t reg_op_b = SIMPLEJS_BYTECODE_VARIABLE_OP_B;

        simplejs_compiler_reg_info_t tmp_reg_info;

        bool is_assign = reg_info.operation_type == SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_LEFT_FLAG;

        tmp_reg_info.have_parent = is_assign;
        tmp_reg_info.reg_parent = SIMPLEJS_BYTECODE_VARIABLE_PARENT;

        tmp_reg_info.is_sub_op = !reg_info.is_sub_op;
        tmp_reg_info.reg_a = reg_op_a;
        tmp_reg_info.reg_b = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        if (is_assign)
        {
            memclr(&instruct_tmp, sizeof(instruct_tmp));
            simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_PARENT;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        }

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR;
        instruct_tmp.instruction.reg_1 = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.is_sub_op = !reg_info.is_sub_op;
        tmp_reg_info.reg_a = reg_op_b;
        tmp_reg_info.reg_b = reg_op_b;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR;
        instruct_tmp.instruction.reg_1 = reg_op_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.compiler_debug = compiler_debug;

        instruct_tmp.instruction.opcode = selected_binary_opcode;

        instruct_tmp.instruction.imm = reg_info.reg_a;
        instruct_tmp.instruction.reg_1 = reg_op_a;
        instruct_tmp.instruction.reg_2 = reg_op_b;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        if (is_assign)
        {
            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR;
            instruct_tmp.instruction.reg_1 = SIMPLEJS_BYTECODE_VARIABLE_PARENT;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

            memclr(&tmp_reg_info, sizeof(tmp_reg_info));
            tmp_reg_info.avoid_refetch = true;
            tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE;
            tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_PARENT;
            tmp_reg_info.reg_b = reg_info.reg_a;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);
        }

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_ASSIGN:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_compiler_debug_t *compiler_debug;
        simplejs_alloc_and_insert_temp(compiler_debug, side, compiler_debug_list, result, status);
        compiler_debug->flags = SIMPLEJS_BYTECODE_DEBUG_INFO_BINARY_OP_FLAG;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        uint8_t reg_b = reg_info.is_sub_assign ? SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_C : SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

        simplejs_compiler_reg_info_t tmp_reg_info;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_RIGHT_FLAG;

        tmp_reg_info.is_sub_assign = !reg_info.is_sub_assign;
        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_READ;
        tmp_reg_info.reg_a = reg_b;
        tmp_reg_info.reg_b = reg_b;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        simplejs_alloc_and_insert_debug(instruct_tmp.compiler_debug, side, compiler_debug, result, status);

        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR;
        instruct_tmp.instruction.reg_1 = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.debug_flags = SIMPLEJS_BYTECODE_DEBUG_INFO_LEFT_FLAG;

        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_b;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_OP_ASSIGN:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 1);

        simplejs_compiler_debug_t *compiler_debug = parent_debug;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);

        simplejs_compiler_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_WRITE;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_DELETE:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 1);

        simplejs_compiler_debug_t *compiler_debug = parent_debug;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);

        simplejs_compiler_reg_info_t tmp_reg_info;

        memclr(&tmp_reg_info, sizeof(tmp_reg_info));
        tmp_reg_info.operation_type = SIMPLEJS_COMPILER_REG_OPERATION_TYPE_DELETE;
        tmp_reg_info.reg_a = reg_info.reg_a;
        tmp_reg_info.reg_b = reg_info.reg_a;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, tmp_reg_info, left), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_COMMA_OPERATOR:
    {
        SIMPLEJS_ASSERT(side->children_list_count == 2);

        simplejs_compiler_debug_t *compiler_debug = parent_debug;

        simplejs_ast_node_t *left = simplejs_get_list_entry_structure(side->children_list_entry.next);
        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(left->list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, reg_info, left), result, status);
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, compiler_debug, reg_info, right), result, status);

        break;
    }

    default:
        simplejs_printf("unknown '%s' type\n", simplejs_get_ast_node_type_string(side->type));
        SIMPLEJS_ASSERT("unknown side->type" && false);
        break;
    }

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        simplejs_list_entry_t *end_debug = &compiler_debug_list;
        simplejs_list_entry_t *current_debug = end_debug->next;

        while (current_debug != end_debug)
        {
            simplejs_list_entry_t *next_debug = current_debug->next;
            simplejs_compiler_debug_t *debug = simplejs_get_list_entry_structure(current_debug);

            simplejs_free_compile_debug_list(&debug->children_list_entry);
            simplejs_hook_mfree(debug);

            current_debug = next_debug;
        }
    }
    else
    {
        simplejs_list_entry_t *end_debug = &compiler_debug_list;
        simplejs_list_entry_t *current_debug = end_debug->next;

        while (current_debug != end_debug)
        {
            simplejs_compiler_debug_t *debug = simplejs_get_list_entry_structure(current_debug);
            simplejs_list_entry_t *next_debug = current_debug->next;

            debug->flags |= reg_info.debug_flags;

            simplejs_insert_debug(compiler_ctx, parent_debug, debug);

            current_debug = next_debug;
        }
    }

    return status;
}

simplejs_status_t simplejs_compile_single_ast(simplejs_compiler_ctx_t *compiler_ctx, simplejs_ast_node_t *ast, simplejs_compiler_ast_info_t ast_info);

simplejs_status_t simplejs_compile_ast_branch(
    simplejs_compiler_ctx_t *compiler_ctx, simplejs_ast_node_t *branch_node,
    simplejs_compiler_ast_info_t ast_info, uintptr_t end_id, uintptr_t next_id)
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

            status = simplejs_compile_ast_branch(compiler_ctx, statement_ast, ast_info, label_id, next_id);
            if (!SIMPLEJS_SUCCESS(status))
            {
                goto result;
            }

            current_ast = current_ast->next;
        }

        simplejs_compiler_instruction_t instruct_tmp = {0};

        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = label_id;
        simplejs_add_instruction(compiler_ctx, instruct_tmp);
    }
    else
    {
        simplejs_compiler_instruction_t instruct_tmp = {0};

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = (uintptr_t)branch_node;
        simplejs_add_instruction(compiler_ctx, instruct_tmp);

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

            simplejs_compiler_reg_info_t reg_info = {0};

            reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;
            reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, reg_info, expression_node), result, status);

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO;
            instruct_tmp.instruction.reg_1 = reg_info.reg_a;
            instruct_tmp.symbol.label_id = next_id;
            simplejs_add_instruction(compiler_ctx, instruct_tmp);

            while (current_ast != end_ast)
            {
                simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

                simplejs_compile_single_ast(compiler_ctx, ast, ast_info);

                current_ast = current_ast->next;
            }

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
            instruct_tmp.symbol.label_id = end_id;
            simplejs_add_instruction(compiler_ctx, instruct_tmp);

            break;
        }

        case SIMPLEJS_AST_NODE_TYPE_ELSE:
        {
            simplejs_list_entry_t *end_ast = &branch_node->children_list_entry;
            simplejs_list_entry_t *current_ast = end_ast->next;

            while (current_ast != end_ast)
            {
                simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

                simplejs_compile_single_ast(compiler_ctx, ast, ast_info);

                current_ast = current_ast->next;
            }

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
            instruct_tmp.symbol.label_id = end_id;
            simplejs_add_instruction(compiler_ctx, instruct_tmp);

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

simplejs_status_t simplejs_compile_ast(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_ast_info_t ast_info, simplejs_list_entry_t *list);

simplejs_status_t simplejs_build_loop_code(
    simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_ast_info_t ast_info,
    simplejs_ast_node_t *init_ast, simplejs_ast_node_t *condition_ast, simplejs_ast_node_t *step_ast, simplejs_ast_node_t *code_ast)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t instruct_tmp;

    simplejs_compiler_reg_info_t reg_info;

    memclr(&reg_info, sizeof(reg_info));
    reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;
    reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;

    uintptr_t repeat_label_id = (uintptr_t)condition_ast;
    uintptr_t break_label_id = (uintptr_t)code_ast + 0x20;
    uintptr_t loop_continue_label_id = (uintptr_t)code_ast + 0x40;

    simplejs_compiler_ast_info_t loop_ast_info = ast_info;
    loop_ast_info.break_label_id = break_label_id;
    loop_ast_info.loop_continue_label_id = loop_continue_label_id;

    SIMPLEJS_ASSERT(condition_ast != NULL);
    SIMPLEJS_ASSERT(code_ast != NULL);

    if (init_ast)
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_single_ast(compiler_ctx, init_ast, ast_info), result, status);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
    instruct_tmp.symbol.label_id = repeat_label_id;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, reg_info, condition_ast), result, status);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO;
    instruct_tmp.instruction.reg_1 = reg_info.reg_a;
    instruct_tmp.symbol.label_id = break_label_id;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_single_ast(compiler_ctx, code_ast, loop_ast_info), result, status);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
    instruct_tmp.symbol.label_id = loop_continue_label_id;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

    if (step_ast)
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, reg_info, step_ast), result, status);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
    instruct_tmp.symbol.label_id = repeat_label_id;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
    instruct_tmp.symbol.label_id = break_label_id;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

result:
    return status;
}

simplejs_status_t simplejs_compile_single_ast(simplejs_compiler_ctx_t *compiler_ctx, simplejs_ast_node_t *ast, simplejs_compiler_ast_info_t ast_info)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_compiler_instruction_t instruct_tmp;

    simplejs_compiler_reg_info_t tmp_reg_info;

    memclr(&tmp_reg_info, sizeof(tmp_reg_info));
    tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;
    tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_A;

    switch (ast->type)
    {
    case SIMPLEJS_AST_NODE_TYPE_FUNCDECL:
    {
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_VARDECL:
    {
        if (!ast->children_list_count)
            break;

        simplejs_ast_var_context_t *var_context = ast->context;

        simplejs_ast_node_t *right = simplejs_get_list_entry_structure(ast->children_list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, tmp_reg_info, right), result, status);

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR;
        instruct_tmp.instruction.reg_1 = tmp_reg_info.reg_a;
        instruct_tmp.instruction.imm = var_context->index;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_VARDECL_LIST:
    {
        SIMPLEJS_ASSERT(ast->children_list_count >= 1);

        simplejs_list_entry_t *end_var = &ast->children_list_entry;
        simplejs_list_entry_t *current_var = end_var->next;

        while (current_var != end_var)
        {
            simplejs_ast_node_t *var = simplejs_get_list_entry_structure(current_var);

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_single_ast(compiler_ctx, var, ast_info), result, status);

            current_var = current_var->next;
        }

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_EXPRESSION:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, tmp_reg_info, ast), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_RETURN:
    {
        SIMPLEJS_ASSERT(ast->children_list_count <= 1);

        if (ast->children_list_count == 1)
        {
            simplejs_ast_node_t *left = simplejs_get_list_entry_structure(ast->children_list_entry.next);
            simplejs_compiler_reg_info_t tmp_reg_info;

            memclr(&tmp_reg_info, sizeof(tmp_reg_info));
            tmp_reg_info.reg_a = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;
            tmp_reg_info.reg_b = SIMPLEJS_BYTECODE_VARIABLE_ASSIGN_B;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_operation(compiler_ctx, NULL, tmp_reg_info, left), result, status);

            memclr(&instruct_tmp, sizeof(instruct_tmp));
            instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SET_RETURN_VAR;
            instruct_tmp.instruction.reg_1 = tmp_reg_info.reg_a;

            SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);
        }

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
        instruct_tmp.symbol.label_id = ast_info.return_label_id;

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_LABEL:
    {
        simplejs_compiler_instruction_t instruct_tmp;
        simplejs_ast_label_context_t *label_context = ast->context;

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
        instruct_tmp.symbol.label_id = (uintptr_t)label_context->name->buffer;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_GOTO:
    {
        simplejs_compiler_instruction_t instruct_tmp;

        simplejs_ast_label_context_t *label_context = NULL;
        if (!simplejs_get_scoped_output_alt(compiler_ctx->parser_ctx, ast, ast->context, &label_context, simplejs_get_scoped_label_callback, false))
        {
            simplejs_printf("goto label not found on compile time!\n");

            status = SIMPLEJS_STATUS_OBJECT_NAME_DOES_NOT_EXIST;
            goto result;
        }

        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_JMP;
        instruct_tmp.symbol.label_id = (uintptr_t)label_context->name->buffer;
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_add_instruction(compiler_ctx, instruct_tmp), result, status);

        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_FOR_LOOP:
    {
        SIMPLEJS_ASSERT(ast->children_list_count == 4);

        simplejs_compiler_instruction_t instruct_tmp;

        simplejs_ast_node_t *init_ast = simplejs_get_list_entry_structure(ast->children_list_entry.next);
        simplejs_ast_node_t *condition_ast = simplejs_get_list_entry_structure(init_ast->list_entry.next);
        simplejs_ast_node_t *step_ast = simplejs_get_list_entry_structure(condition_ast->list_entry.next);

        simplejs_ast_node_t *code_ast = simplejs_get_list_entry_structure(step_ast->list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_build_loop_code(compiler_ctx, ast_info, init_ast, condition_ast, step_ast, code_ast), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_WHILE_LOOP:
    {
        SIMPLEJS_ASSERT(ast->children_list_count == 2);

        simplejs_compiler_instruction_t instruct_tmp;

        simplejs_ast_node_t *condition_ast = simplejs_get_list_entry_structure(ast->children_list_entry.next);
        simplejs_ast_node_t *code_ast = simplejs_get_list_entry_structure(condition_ast->list_entry.next);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_build_loop_code(compiler_ctx, ast_info, NULL, condition_ast, NULL, code_ast), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_BRANCH:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast_branch(compiler_ctx, ast, ast_info, 0, 0), result, status);
        break;
    }

    case SIMPLEJS_AST_NODE_TYPE_CODEBLOCK:
    {
        SIMPLEJS_REQUIRE_SUCCESS(simplejs_compile_ast(compiler_ctx, ast_info, &ast->children_list_entry), result, status);
        break;
    }

    default:
    {
        simplejs_printf("unhandled '%s'\n", simplejs_get_ast_node_type_string(ast->type));

        SIMPLEJS_ASSERT("unhandled simplejs_compile_single_ast!" && false);
        break;
    }
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_ast(simplejs_compiler_ctx_t *compiler_ctx, simplejs_compiler_ast_info_t ast_info, simplejs_list_entry_t *list)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_list_entry_t *end_ast = list;
    simplejs_list_entry_t *current_ast = end_ast->next;

    while (current_ast != end_ast)
    {
        simplejs_ast_node_t *ast = simplejs_get_list_entry_structure(current_ast);

        status = simplejs_compile_single_ast(compiler_ctx, ast, ast_info);
        if (!SIMPLEJS_SUCCESS(status))
        {
            goto result;
        }

        current_ast = current_ast->next;
    }

result:
    return status;
}

simplejs_status_t simplejs_compile_ast_function(simplejs_compiler_ctx_t *compiler_ctx, simplejs_ast_node_t *function_node)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_ast_function_context_t *function_context = function_node->context;

    simplejs_compiler_instruction_t instruct_tmp = {0};
    simplejs_compiler_ast_info_t ast_info = {0};

    ast_info.return_label_id = (uintptr_t)function_node;

    if (function_context->name)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_FUNCTION;
        instruct_tmp.symbol.label_id = (uintptr_t)function_context->name->buffer;
        simplejs_add_instruction(compiler_ctx, instruct_tmp);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET;
    simplejs_add_instruction(compiler_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET;
    simplejs_add_instruction(compiler_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE;
    instruct_tmp.instruction.imm = function_context->local_var_count;
    simplejs_add_instruction(compiler_ctx, instruct_tmp);

    for (size_t i = 0; i < function_context->local_var_count; i++)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_VAR;
        instruct_tmp.instruction.imm = i;
        simplejs_add_instruction(compiler_ctx, instruct_tmp);
    }

    status = simplejs_compile_ast(compiler_ctx, ast_info, &function_node->children_list_entry);
    if (!SIMPLEJS_SUCCESS(status))
    {
        goto result;
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.type = SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO;
    instruct_tmp.symbol.label_id = ast_info.return_label_id;
    simplejs_add_instruction(compiler_ctx, instruct_tmp);

    for (size_t i = 0; i < function_context->local_var_count; i++)
    {
        memclr(&instruct_tmp, sizeof(instruct_tmp));
        instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_FREE_LOC_VAR;
        instruct_tmp.instruction.imm = i;
        simplejs_add_instruction(compiler_ctx, instruct_tmp);
    }

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE;
    instruct_tmp.instruction.imm = -function_context->local_var_count;
    simplejs_add_instruction(compiler_ctx, instruct_tmp);

    memclr(&instruct_tmp, sizeof(instruct_tmp));
    instruct_tmp.instruction.opcode = SIMPLEJS_BYTECODE_OPCODE_RETURN;

    simplejs_add_instruction(compiler_ctx, instruct_tmp);

result:
    return status;
}

static char *alu_name_table[] = {
    [SIMPLEJS_BYTECODE_OPCODE_INC_VAR] = "inc_var",
    [SIMPLEJS_BYTECODE_OPCODE_DEC_VAR] = "dec_var",

    [SIMPLEJS_BYTECODE_OPCODE_LOGICAL_NOT_VAR] = "logical_not_var",
    [SIMPLEJS_BYTECODE_OPCODE_BITWISE_NOT_VAR] = "bitwise_not_var",
    [SIMPLEJS_BYTECODE_OPCODE_NEG_VAR] = "neg_var",

    [SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR] = "equal_var",
    [SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR] = "not_equal_var",
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR] = "greater_var",
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR] = "below_var",
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR] = "greater_equal_var",
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR] = "below_equal_var",

    [SIMPLEJS_BYTECODE_OPCODE_OR_VAR] = "or_var",
    [SIMPLEJS_BYTECODE_OPCODE_AND_VAR] = "and_var",

    [SIMPLEJS_BYTECODE_OPCODE_SHL_VAR] = "shl_var",
    [SIMPLEJS_BYTECODE_OPCODE_SHR_VAR] = "shr_var",
    [SIMPLEJS_BYTECODE_OPCODE_SAL_VAR] = "sal_var",
    [SIMPLEJS_BYTECODE_OPCODE_SAR_VAR] = "sar_var",

    [SIMPLEJS_BYTECODE_OPCODE_ADD_VAR] = "add_var",
    [SIMPLEJS_BYTECODE_OPCODE_SUB_VAR] = "sub_var",
    [SIMPLEJS_BYTECODE_OPCODE_MUL_VAR] = "mul_var",
    [SIMPLEJS_BYTECODE_OPCODE_DIV_VAR] = "div_var",
    [SIMPLEJS_BYTECODE_OPCODE_MOD_VAR] = "mod_var",
};

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

    case SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR:
        simplejs_printf("save_var v%u", instruction.reg_1);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR:
        simplejs_printf("restore_var v%u", instruction.reg_1);
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
    case SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET:
        simplejs_printf("save_arg_offset");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS:
        simplejs_printf("alloc_args v%u, %u", instruction.reg_1, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS:
        simplejs_printf("free_args");
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_RETURN_VAR:
        simplejs_printf("get_return_var v%u", instruction.reg_1);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_RETURN_VAR:
        simplejs_printf("set_return_var v%u", instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_INIT_VAR:
        simplejs_printf("init_var v%u", instruction.imm);
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

    case SIMPLEJS_BYTECODE_OPCODE_GET_CALL_ARG_VAR:
        simplejs_printf("get_call_arg_var v%u, %u", instruction.reg_1, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_CALL_ARG_VAR:
        simplejs_printf("set_call_arg_var %u, v%u", instruction.imm, instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_GET_FUNC_ARG_VAR:
        simplejs_printf("get_func_arg_var v%u, %u", instruction.reg_1, instruction.imm);
        break;
    case SIMPLEJS_BYTECODE_OPCODE_SET_FUNC_ARG_VAR:
        simplejs_printf("set_func_arg_var %u, v%u", instruction.imm, instruction.reg_1);
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

    case SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO:
        simplejs_printf("jmp_if_not_zero v%u, ", instruction.reg_1);

        if (instruction.imm_signed > 0)
            simplejs_printf("0x%d", instruction.imm_signed);
        else
            simplejs_printf("-0x%x", -instruction.imm_signed);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR:
        simplejs_printf("convert_boolean_var v%u, v%u", instruction.reg_1, instruction.reg_2);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_INC_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_DEC_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_LOGICAL_NOT_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_BITWISE_NOT_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_NEG_VAR:
        simplejs_printf("%s v%u", alu_name_table[instruction.opcode], instruction.reg_1);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR:

    case SIMPLEJS_BYTECODE_OPCODE_OR_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_AND_VAR:

    case SIMPLEJS_BYTECODE_OPCODE_SHL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_SHR_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_SAL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_SAR_VAR:

    case SIMPLEJS_BYTECODE_OPCODE_ADD_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_SUB_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_MUL_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_DIV_VAR:
    case SIMPLEJS_BYTECODE_OPCODE_MOD_VAR:
        simplejs_printf("%s v%u, v%u, v%u", alu_name_table[instruction.opcode], instruction.reg_1, instruction.reg_2, instruction.imm);
        break;

    case SIMPLEJS_BYTECODE_OPCODE_END:
        simplejs_printf("// end of code!");
        break;
    }

    simplejs_printf("\n");
}

void simplejs_ast_dump_bytecode(simplejs_compiler_ctx_t *compiler_ctx)
{
    simplejs_bytecode_header_t bytecode_header;
    simplejs_bytecode_header_decode(compiler_ctx->executable, &bytecode_header);

    uint8_t *start_instruction_pointer = (uint8_t *)compiler_ctx->executable + bytecode_header.code_offset.start;
    uint8_t *instruction_pointer = start_instruction_pointer;

    while (true)
    {
        simplejs_printf("/* ip: 0x%08x */ ", (uint32_t)((uintptr_t)instruction_pointer - (uintptr_t)start_instruction_pointer));

        uint8_t instruction_size;
        simplejs_bytecode_instruction_t instruction;
        simplejs_bytecode_decode(instruction_pointer, &instruction, &instruction_size);

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
            *out = compiler_instruction->symbol.data_offset;
            return true;
        }

    skip_instruction:
        current_instruction = current_instruction->next;
    }

    simplejs_printf("label not found!\n");
    return false;
}

void simplejs_compile_debug_info(
    simplejs_bytecode_header_t *bytecode_header, uint8_t **current_buffer, uint8_t *end_buffer,
    simplejs_compiler_debug_t *parent_debug, simplejs_list_entry_t *parent_list_entry)
{
    SIMPLEJS_ASSERT(current_buffer != NULL);

    simplejs_list_entry_t *end_debug = parent_debug ? &parent_debug->children_list_entry : parent_list_entry;
    simplejs_list_entry_t *current_debug = end_debug->next;

    if (parent_debug)
    {
        uint8_t *next_buffer = &(*current_buffer)[sizeof(simplejs_bytecode_debug_info_t)];
        simplejs_bytecode_debug_info_t debug_info = {0};

        SIMPLEJS_ASSERT(next_buffer <= end_buffer);

        debug_info.flags = parent_debug->flags;
        debug_info.children_debug_count = parent_debug->children_list_count;

        debug_info.code_offset = parent_debug->code_offset;
        debug_info.source_offset = parent_debug->source_offset;

        simplejs_bytecode_debug_info_encode(*current_buffer, &debug_info);
        (*current_buffer) = next_buffer;
    }

    while (current_debug != end_debug)
    {
        simplejs_compiler_debug_t *debug = simplejs_get_list_entry_structure(current_debug);

        simplejs_compile_debug_info(bytecode_header, current_buffer, end_buffer, debug, NULL);

        current_debug = current_debug->next;
    }
}

simplejs_status_t simplejs_compile_instructions(simplejs_compiler_ctx_t *compiler_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uint32_t total_size = 0;

    uint32_t code_offset = 0;
    uint32_t code_size = 0;

    uint32_t data_offset = 0;
    uint32_t data_size = 0;

    uint32_t debug_info_offset = 0;
    uint32_t debug_info_size = 0;

    uint32_t current_code_offset = 0;

    total_size += simplejs_align_to_cacheline(sizeof(simplejs_bytecode_header_t));
    code_offset = total_size;
    code_size = simplejs_align_to_cacheline(compiler_ctx->instruction_list_count * SIMPLEJS_BYTECODE_EXTENDED_INSTRUCTION_SIZE);

    total_size += code_size;
    data_offset = total_size;
    data_size = simplejs_align_to_cacheline(compiler_ctx->data_offset);

    total_size += data_size;
    debug_info_offset = total_size;
    debug_info_size = compiler_ctx->debug_list_total_count * sizeof(simplejs_bytecode_debug_info_t);

    total_size += debug_info_size;

    uint8_t *buffer = simplejs_hook_malloc(total_size);
    if (!buffer)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(buffer, total_size);

    simplejs_bytecode_header_t bytecode_header = {0};

    bytecode_header.size = sizeof(bytecode_header);
    bytecode_header.version = SIMPLEJS_BYTECODE_VERSION;
    bytecode_header.flags = SIMPLEJS_BYTECODE_HEADER_DEBUG_INFO_FLAG;

    bytecode_header.debug_info_entry_size = sizeof(simplejs_bytecode_debug_info_t);

    bytecode_header.code_offset.start = code_offset;
    bytecode_header.code_offset.end = code_offset + code_size;

    bytecode_header.debug_info_offset.start = debug_info_offset;
    bytecode_header.debug_info_offset.end = debug_info_offset + debug_info_size;

    simplejs_bytecode_header_encode(buffer, &bytecode_header);

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
            case SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO:
            {
                uint32_t absolute_offset = 0;
                bool label_result = simplejs_get_label_offset(compiler_ctx, SIMPLEJS_COMPILER_INSTRUCTION_TYPE_LABEL_GOTO, compiler_instruction->symbol.label_id, &absolute_offset);
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

            if (compiler_instruction->compiler_debug)
            {
                simplejs_compiler_debug_t *compiler_debug = compiler_instruction->compiler_debug;

                compiler_debug->code_offset.start = current_code_offset;
                compiler_debug->code_offset.end = current_code_offset + instruction_size;

                if (compiler_debug->diagnostic_token)
                {
                    compiler_debug->flags |= SIMPLEJS_BYTECODE_DEBUG_INFO_DIAGNOSTIC_FLAG;

                    compiler_debug->source_offset = compiler_debug->diagnostic_offset;
                }
                else
                {
                    printf("compiler debug without diagnostic token!\n");
                }
            }

            simplejs_bytecode_encode(&buffer[current_code_offset], instruction, &instruction_size);
            current_code_offset += instruction_size;

        skip_instruction:
            current_instruction = current_instruction->next;
        }
    }

    uint8_t *current_debug_info = &buffer[debug_info_offset];
    uint8_t *end_debug_info = &current_debug_info[debug_info_size];

    simplejs_compile_debug_info(&bytecode_header, &current_debug_info, end_debug_info, NULL, &compiler_ctx->debug_list);

    compiler_ctx->executable = buffer;
    compiler_ctx->executable_size = total_size;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (buffer)
            simplejs_hook_mfree(buffer);
    }

    return status;
}

void SIMPLEJS_API simplejs_free_compiler_ctx(simplejs_compiler_ctx_t *compiler_ctx)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_list_entry_t *end_instruction = &compiler_ctx->instruction_list;
    simplejs_list_entry_t *current_instruction = end_instruction->next;

    while (current_instruction != end_instruction)
    {
        simplejs_list_entry_t *next_instruction = current_instruction->next;
        simplejs_compiler_instruction_t *compiler_instruction = simplejs_get_list_entry_structure(current_instruction);

        simplejs_hook_mfree(compiler_instruction);

        current_instruction = next_instruction;
    }

    simplejs_free_compile_debug_list(&compiler_ctx->debug_list);

    if (compiler_ctx->executable)
        simplejs_hook_mfree(compiler_ctx->executable);

    simplejs_hook_mfree(compiler_ctx);
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
    simplejs_init_list_entry(&compiler_ctx->debug_list, compiler_ctx);

    status = simplejs_compile_ast_function(compiler_ctx, parser_ctx->root_ast);
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

        status = simplejs_compile_ast_function(compiler_ctx, ast);
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
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (compiler_ctx)
            simplejs_free_compiler_ctx(compiler_ctx);
    }
    else
    {
        simplejs_ast_dump_bytecode(compiler_ctx);
    }

    return status;
}

void SIMPLEJS_API simplejs_compiler_ctx_get_executable(simplejs_compiler_ctx_t *compiler_ctx, void **executable_out, uint32_t *size_out)
{
    SIMPLEJS_ASSERT(compiler_ctx != NULL);
    SIMPLEJS_ASSERT(executable_out != NULL);
    SIMPLEJS_ASSERT(size_out != NULL);

    *executable_out = compiler_ctx->executable;
    *size_out = compiler_ctx->executable_size;
}

uintptr_t SIMPLEJS_API simplejs_compiler_get_executable_entry_point(void *executable, uint32_t size)
{
    void *out = NULL;
    void *executable_end = (uint8_t *)executable + size;

    if (size <= sizeof(simplejs_bytecode_header_t))
        goto result;

    simplejs_bytecode_header_t bytecode_header;
    simplejs_bytecode_header_decode(executable, &bytecode_header);

    void *entry_point = (uint8_t *)executable + bytecode_header.code_offset.start;
    if (bytecode_header.version != SIMPLEJS_BYTECODE_VERSION ||
        entry_point >= executable_end)
        goto result;

    out = entry_point;

result:
    return (uintptr_t)out;
}
