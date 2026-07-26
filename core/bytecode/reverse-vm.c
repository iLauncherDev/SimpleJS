#include <vm.h>

#include "vm-inline-functions.c"

void SIMPLEJS_API simplejs_reset_reverse_vm(simplejs_vm_t *vm)
{
    memclr(&vm->reverse_state, sizeof(vm->reverse_state));
}

simplejs_status_t simplejs_reverse_bytecode_opcode_nop(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_add_stack(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed);

    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_add_stack_var_size(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed * sizeof(simplejs_variable_t));

    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_save_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->reverse_state.block_restore_var++;

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_restore_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (vm->reverse_state.block_restore_var > 0)
    {
        vm->reverse_state.block_restore_var--;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_restore_var(vm, instruction), result, status);

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_save_ctx(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->reverse_state.block_restore_ctx++;

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_restore_ctx(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (vm->reverse_state.block_restore_ctx > 0)
    {
        vm->reverse_state.block_restore_ctx--;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_restore_ctx(vm, instruction), result, status);

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_alloc_vars(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->reverse_state.block_free_vars++;

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_free_vars(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (vm->reverse_state.block_free_vars > 0)
    {
        vm->reverse_state.block_free_vars--;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_free_vars(vm, instruction), result, status);

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_alloc_args(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->reverse_state.block_free_args++;

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_free_args(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    if (vm->reverse_state.block_free_args > 0)
    {
        vm->reverse_state.block_free_args--;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_free_args(vm, instruction), result, status);

result:
    return status;
}

simplejs_status_t simplejs_reverse_bytecode_opcode_return(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_return(vm, instruction), result, status);

result:
    return status;
}

simplejs_bytecode_opcode_jumptable_t simplejs_reverse_bytecode_opcode_jumptable[SIMPLEJS_BYTECODE_OPCODE_END] = {
    [SIMPLEJS_BYTECODE_OPCODE_MOV_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR] = simplejs_reverse_bytecode_opcode_save_var,
    [SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR] = simplejs_reverse_bytecode_opcode_restore_var,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX] = simplejs_reverse_bytecode_opcode_save_ctx,
    [SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX] = simplejs_reverse_bytecode_opcode_restore_ctx,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_ALLOC_VARS] = simplejs_reverse_bytecode_opcode_alloc_vars,
    [SIMPLEJS_BYTECODE_OPCODE_FREE_VARS] = simplejs_reverse_bytecode_opcode_free_vars,

    [SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS] = simplejs_reverse_bytecode_opcode_alloc_args,
    [SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS] = simplejs_reverse_bytecode_opcode_free_args,

    [SIMPLEJS_BYTECODE_OPCODE_GET_RETURN_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_RETURN_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_INIT_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_CALL_ARG_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_CALL_ARG_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_FUNC_ARG_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_FUNC_ARG_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_CALL] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_RETURN] = simplejs_reverse_bytecode_opcode_return,

    [SIMPLEJS_BYTECODE_OPCODE_JMP] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_INC_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_DEC_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_LOGICAL_NOT_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_BITWISE_NOT_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_NEG_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_OR_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_AND_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_SHL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SHR_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SAL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SAR_VAR] = simplejs_reverse_bytecode_opcode_nop,

    [SIMPLEJS_BYTECODE_OPCODE_ADD_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_SUB_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_MUL_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_DIV_VAR] = simplejs_reverse_bytecode_opcode_nop,
    [SIMPLEJS_BYTECODE_OPCODE_MOD_VAR] = simplejs_reverse_bytecode_opcode_nop,
};

simplejs_status_t simplejs_execute_reverse_vm(simplejs_vm_t *vm)
{
    uint8_t instruction_size = 0;
    simplejs_bytecode_instruction_t instruction;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (vm->reverse_state.vm_stopped)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    SIMPLEJS_ASSERT(vm != NULL);
    SIMPLEJS_ASSERT(vm->state.instruction_pointer != (uintptr_t)NULL);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_memory_pointer(
                                 vm,
                                 (void *)vm->state.instruction_pointer,
                                 SIMPLEJS_BYTECODE_BASE_INSTRUCTION_SIZE),
                             result, status);

    simplejs_bytecode_get_opcode_size((uint8_t *)vm->state.instruction_pointer, &instruction_size);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_memory_pointer(
                                 vm,
                                 (void *)vm->state.instruction_pointer,
                                 instruction_size),
                             result, status);

    simplejs_bytecode_decode((uint8_t *)vm->state.instruction_pointer, &instruction, &instruction_size);

    simplejs_printf("/* reverse ip: %p */ ", (void *)vm->state.instruction_pointer);
    simplejs_disasm_bytecode(instruction, vm->state.instruction_pointer + instruction_size);

    vm->state.instruction_pointer += instruction_size;

    if (instruction.opcode >= SIMPLEJS_BYTECODE_OPCODE_END)
    {
        status = SIMPLEJS_STATUS_INVALID_OPCODE;
        goto result;
    }

    simplejs_bytecode_opcode_jumptable_t opcode_handler = simplejs_reverse_bytecode_opcode_jumptable[instruction.opcode];
    if (!opcode_handler)
    {
        status = SIMPLEJS_STATUS_INVALID_OPCODE;
        goto result;
    }

    status = opcode_handler(vm, &instruction);

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        vm->state.instruction_pointer -= instruction_size;
        vm->reverse_state.vm_stopped = true;
    }

    return status;
}
