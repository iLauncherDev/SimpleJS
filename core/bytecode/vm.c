#include <vm.h>

#define STACK_SIZE (64 * 1024)

simplejs_status_t SIMPLEJS_API simplejs_create_vm(simplejs_vm_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_vm_t *ret = simplejs_hook_malloc(sizeof(*ret));
    if (!ret)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    memclr(ret, sizeof(*ret));

    ret->stack_size = STACK_SIZE;
    ret->stack = simplejs_hook_malloc(ret->stack_size);
    if (!ret->stack)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    ret->reverse_stack_size = STACK_SIZE;
    ret->reverse_stack = simplejs_hook_malloc(ret->reverse_stack_size);
    if (!ret->reverse_stack)
    {
        status = SIMPLEJS_STATUS_ALLOCATION_ERROR;
        goto result;
    }

    *out = ret;

result:
    if (!SIMPLEJS_SUCCESS(status))
    {
        if (ret)
        {
            if (ret->stack)
                simplejs_hook_mfree(ret->stack);
            if (ret->reverse_stack)
                simplejs_hook_mfree(ret->reverse_stack);

            simplejs_hook_mfree(ret);
        }
    }

    return status;
}

void SIMPLEJS_API simplejs_reset_vm(simplejs_vm_t *vm)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (vm->state.instruction_pointer &&
        vm->state.stack_offset)
    {
        simplejs_reset_reverse_vm(vm);

        while (SIMPLEJS_SUCCESS(status))
            status = simplejs_execute_reverse_vm(vm);
    }

    simplejs_variable_dereference(&vm->state.return_variable);

    for (size_t i = 0; i < 16; i++)
    {
        simplejs_variable_dereference(&vm->state.variables[i]);
    }

    simplejs_variable_dereference(&vm->state.global_variable);

    memclr(&vm->state, sizeof(vm->state));
}

void SIMPLEJS_API simplejs_destroy_vm(simplejs_vm_t *vm)
{
    simplejs_reset_vm(vm);

    if (vm->stack)
        simplejs_hook_mfree(vm->stack);

    if (vm->reverse_stack)
        simplejs_hook_mfree(vm->reverse_stack);

    simplejs_hook_mfree(vm);
}

void SIMPLEJS_API simplejs_vm_set_memory(simplejs_vm_t *vm, simplejs_vm_memory_t *vm_memory)
{
    SIMPLEJS_ASSERT(vm != NULL);
    SIMPLEJS_ASSERT(vm_memory != NULL);

    vm->memory = vm_memory;
}

#include "vm-inline-functions.c"

simplejs_status_t simplejs_bytecode_opcode_return(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction);

void SIMPLEJS_API simplejs_vm_set_global_variable(simplejs_vm_t *vm, simplejs_variable_t *global_variable)
{
    SIMPLEJS_ASSERT(vm != NULL);
    SIMPLEJS_ASSERT(global_variable != NULL);

    simplejs_variable_assign(&vm->state.global_variable, global_variable);
}

simplejs_status_t simplejs_vm_push_args_to_stack(
    simplejs_vm_t *vm, uintptr_t argument_offset,
    simplejs_variable_t *return_variable,
    simplejs_variable_t *arguments, uint32_t argument_count)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;

    uintptr_t function_header_size = sizeof(*function_header) + (sizeof(*function_header->arguments) * argument_count);
    function_header = (void *)(vm->stack + argument_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    memclr(function_header, function_header_size);

    function_header->return_variable = return_variable;
    function_header->argument_count = argument_count;
    if (arguments)
    {
        for (int i = 0; i < function_header->argument_count; i++)
            simplejs_variable_assign(&function_header->arguments[i], &arguments[i]);
    }

    simplejs_vm_add_stack(&vm->state.stack_offset, function_header_size);

result:
    return status;
}

simplejs_status_t simplejs_vm_init_args(simplejs_vm_t *vm,
                                        simplejs_variable_t *return_variable,
                                        simplejs_variable_t *arguments, uint32_t argument_count)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.argument_offset = vm->state.stack_offset;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_vm_push_args_to_stack(vm, vm->state.argument_offset,
                                                            return_variable,
                                                            arguments, argument_count),
                             result, status);

result:
    return status;
}

simplejs_status_t simplejs_vm_push_return_to_stack(
    simplejs_vm_t *vm,
    uintptr_t instruction_pointer,
    uintptr_t saved_argument_offset, uintptr_t argument_offset,
    bool vm_exit, bool free_args)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_vm_return_t *vm_return = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, vm_return), result, status);

    vm_return->saved_argument_offset = saved_argument_offset;
    vm_return->argument_offset = argument_offset;
    vm_return->instruction_pointer = instruction_pointer;
    vm_return->vm_exit = vm_exit;
    vm_return->free_args = free_args;

    simplejs_vm_add_stack(&vm->state.stack_offset, sizeof(*vm_return));

result:
    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_vm_call_function(simplejs_vm_t *vm,
                                                         simplejs_function_t *function,
                                                         simplejs_variable_t *return_variable,
                                                         simplejs_variable_t *arguments, uint32_t argument_count,
                                                         bool vm_exit)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uintptr_t saved_argument_offset = vm->state.saved_argument_offset;
    uintptr_t argument_offset = vm->state.argument_offset;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_vm_init_args(vm, return_variable, arguments, argument_count), result, status);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_vm_push_return_to_stack(vm,
                                                              vm->state.instruction_pointer,
                                                              saved_argument_offset, argument_count,
                                                              vm_exit, true),
                             result, status);

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    switch (function->type)
    {
    case SIMPLEJS_FUNCTION_TYPE_NATIVE:
        vm->state.instruction_pointer = function->value.instruction_pointer;
        break;

    case SIMPLEJS_FUNCTION_TYPE_PROXY:
        SIMPLEJS_ASSERT(function->value.proxy != NULL);

        SIMPLEJS_REQUIRE_SUCCESS(function->value.proxy(function_header), result, status);

        SIMPLEJS_REQUIRE_SUCCESS(simplejs_bytecode_opcode_return(vm, NULL), result, status);
        break;

    default:
        break;
    }

result:
    return status;
}

void simplejs_disasm_bytecode(simplejs_bytecode_instruction_t instruction, uintptr_t instruction_pointer);

simplejs_status_t simplejs_bytecode_opcode_nop(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    return SIMPLEJS_STATUS_SUCCESS;
}

simplejs_status_t simplejs_bytecode_opcode_mov_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_2];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_add_stack(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_add_stack_var_size(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed * sizeof(simplejs_variable_t));

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_loc_offset(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.local_var_offset = vm->state.stack_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_arg_offset(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.argument_offset = vm->state.stack_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_save_arg_offset(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.saved_argument_offset = vm->state.argument_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_save_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *reg_var, *stack_var;

    reg_var = &vm->state.variables[instruction->reg_1];
    stack_var = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, stack_var), result, status);

    simplejs_variable_init_undefined(stack_var);
    simplejs_variable_assign(stack_var, reg_var);

    simplejs_vm_add_stack(&vm->state.stack_offset, sizeof(*stack_var));

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_restore_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *stack_var, *reg_var;

    simplejs_vm_add_stack(&vm->state.stack_offset, -sizeof(*stack_var));

    reg_var = &vm->state.variables[instruction->reg_1];
    stack_var = (void *)(vm->stack + vm->state.stack_offset);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, stack_var), result, status);

    simplejs_variable_assign(reg_var, stack_var);
    simplejs_variable_dereference(stack_var);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_save_ctx(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_context_t *context = (void *)(vm->stack + vm->state.stack_offset);
    simplejs_number_t undef_number = {.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED};

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, context), result, status);

    context->saved_argument_offset = vm->state.saved_argument_offset;
    context->argument_offset = vm->state.argument_offset;
    context->local_var_offset = vm->state.local_var_offset;

    simplejs_vm_add_stack(&vm->state.stack_offset, sizeof(*context));

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_restore_ctx(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_context_t *context;

    simplejs_vm_add_stack(&vm->state.stack_offset, -sizeof(*context));

    context = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, context), result, status);

    vm->state.saved_argument_offset = context->saved_argument_offset;
    vm->state.argument_offset = context->argument_offset;
    vm->state.local_var_offset = context->local_var_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_alloc_args(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *return_variable = &vm->state.variables[instruction->reg_1];

    simplejs_function_header_t *function_header;

    uintptr_t function_header_size = sizeof(*function_header) + (sizeof(*function_header->arguments) * instruction->imm);
    function_header = (void *)(vm->stack + vm->state.argument_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    memclr(function_header, function_header_size);

    function_header->return_variable = return_variable;
    function_header->argument_count = instruction->imm;

    simplejs_vm_add_stack(&vm->state.stack_offset, function_header_size);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_free_args(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    uintptr_t function_header_size = sizeof(*function_header) + (sizeof(*function_header->arguments) * function_header->argument_count);
    simplejs_vm_add_stack(&vm->state.stack_offset, -function_header_size);

    simplejs_variable_dereference(&function_header->this_variable);

    for (size_t i = 0; i < function_header->argument_count; i++)
    {
        simplejs_variable_dereference(&function_header->arguments[i]);
    }

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_return_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    simplejs_variable_t *out = function_header->return_variable;
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    SIMPLEJS_ASSERT(out != NULL);

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];

    simplejs_variable_dereference(variable);
    simplejs_variable_init_undefined(variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_loc_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, variable), result, status);

    simplejs_variable_init_undefined(variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_free_loc_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, variable), result, status);

    simplejs_variable_dereference(variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_var_prop(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];
    simplejs_object_t *variable_object = variable->value.object;

    simplejs_variable_t *property = &vm->state.variables[instruction->reg_2];

    simplejs_variable_t *output = &vm->state.variables[instruction->imm & 0x0F];

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_object_get_property_value(variable_object, property, output), result, status);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_var_prop(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];
    simplejs_object_t *variable_object = variable->value.object;

    simplejs_variable_t *property = &vm->state.variables[instruction->reg_2];

    simplejs_variable_t *input = &vm->state.variables[instruction->imm & 0x0F];

    if (variable->type != SIMPLEJS_VARIABLE_TYPE_OBJECT)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_object_set_property_value(variable_object, property, input), result, status);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_global_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.global_variable;

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_loc_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, in), result, status);

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_loc_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, out), result, status);

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_call_arg_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &function_header->arguments[instruction->imm];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_call_arg_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    simplejs_variable_t *out = &function_header->arguments[instruction->imm];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_func_arg_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.saved_argument_offset), result, status);

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &function_header->arguments[instruction->imm];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_func_arg_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.saved_argument_offset), result, status);

    simplejs_variable_t *out = &function_header->arguments[instruction->imm];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_var_number(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t variable;
    variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    simplejs_number_decode(&variable.value.number, (uint8_t *)(vm->state.instruction_pointer + instruction->imm_signed));

    simplejs_variable_assign(&vm->state.variables[instruction->reg_1], &variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_var_native_function(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t variable;
    variable.type = SIMPLEJS_VARIABLE_TYPE_FUNCTION;

    simplejs_function_t *function = &variable.value.function;

    function->type = SIMPLEJS_FUNCTION_TYPE_NATIVE;
    function->value.instruction_pointer = vm->state.instruction_pointer + instruction->imm_signed;

    simplejs_variable_assign(&vm->state.variables[instruction->reg_1], &variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_var_fast_string(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t variable;
    variable.type = SIMPLEJS_VARIABLE_TYPE_FAST_STRING;
    variable.value.fast_string = (char *)(vm->state.instruction_pointer + instruction->imm_signed);

    simplejs_variable_assign(&vm->state.variables[instruction->reg_1], &variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_call_native(
    simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_vm_push_return_to_stack(vm,
                                                              vm->state.instruction_pointer,
                                                              vm->state.saved_argument_offset, vm->state.argument_offset,
                                                              false, false),
                             result, status);

    vm->state.instruction_pointer = function->value.instruction_pointer;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_call_proxy(
    simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.return_variable;

    SIMPLEJS_ASSERT(function->value.proxy != NULL);

    simplejs_variable_assign(&function_header->this_variable, &vm->state.variables[SIMPLEJS_BYTECODE_VARIABLE_THIS]);
    SIMPLEJS_REQUIRE_SUCCESS(function->value.proxy(function_header), result, status);

result:
    return status;
}

typedef simplejs_status_t (*simplejs_bytecode_opcode_call_jumptable_t)(
    simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function);

simplejs_bytecode_opcode_call_jumptable_t simplejs_bytecode_opcode_call_jumptable[SIMPLEJS_FUNCTION_TYPE_END] = {
    [SIMPLEJS_FUNCTION_TYPE_NATIVE] = simplejs_bytecode_opcode_call_native,
    [SIMPLEJS_FUNCTION_TYPE_PROXY] = simplejs_bytecode_opcode_call_proxy,
};

simplejs_status_t simplejs_bytecode_opcode_call(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    if (in->type != SIMPLEJS_VARIABLE_TYPE_FUNCTION)
    {
        simplejs_printf("variable is not function!\n");

        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    simplejs_function_t *function = &in->value.function;
    if (function->type >= SIMPLEJS_FUNCTION_TYPE_END)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    simplejs_bytecode_opcode_call_jumptable_t call_handler = simplejs_bytecode_opcode_call_jumptable[function->type];
    if (!call_handler)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    status = call_handler(vm, instruction, function_header, function);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_return(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_return_t *vm_return = NULL;

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_vm_pop_return_from_stack(vm, &vm_return), result, status);

    uintptr_t mask = -(vm_return->free_args == true);
    simplejs_bytecode_opcode_jumptable_t free_args = (void *)(((uintptr_t)simplejs_bytecode_opcode_free_args & mask) |
                                                              ((uintptr_t)simplejs_bytecode_opcode_nop & ~mask));

    free_args(vm, NULL);

    vm->state.saved_argument_offset = vm_return->saved_argument_offset;
    vm->state.argument_offset = vm_return->argument_offset;

    vm->state.instruction_pointer = vm_return->instruction_pointer;
    if (vm_return->vm_exit)
    {
        status = SIMPLEJS_STATUS_PROGRAM_EXITED;
        goto result;
    }

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.instruction_pointer += instruction->imm_signed;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp_if_zero(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];

    uint64_t value = simplejs_variable_get_int(variable);
    int32_t condition = !value;

    vm->state.instruction_pointer += instruction->imm_signed * condition;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp_if_not_zero(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];

    uint64_t value = simplejs_variable_get_int(variable);
    int32_t condition = !!value;

    vm->state.instruction_pointer += instruction->imm_signed * condition;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_convert_boolean_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_2];

    simplejs_variable_t temp_out_var;
    temp_out_var.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;

    simplejs_number_t *temp_num = &temp_out_var.value.number;
    temp_num->type = SIMPLEJS_NUMBER_TYPE_BOOLEAN;
    temp_num->value.boolean = !!simplejs_variable_get_int(in);

    simplejs_variable_assign(out, &temp_out_var);

    return status;
}

#define simplejs_bytecode_opcode_unary_var(low_name, upper_name)                                                                 \
    simplejs_status_t simplejs_bytecode_opcode_##low_name##_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction) \
    {                                                                                                                            \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                                      \
        simplejs_variable_t *v_a = &vm->state.variables[instruction->reg_1];                                                     \
                                                                                                                                 \
        if (v_a->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)                                                                          \
        {                                                                                                                        \
            simplejs_printf("v_a->type is not number (%u)\n", v_a->type);                                                        \
            status = SIMPLEJS_STATUS_PROGRAM_CRASHED;                                                                            \
            goto result;                                                                                                         \
        }                                                                                                                        \
                                                                                                                                 \
        simplejs_number_t *op_a = &v_a->value.number;                                                                            \
        simplejs_number_t op_b = {.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED};                                                       \
                                                                                                                                 \
        simplejs_number_execute_alu(SIMPLEJS_NUMBER_ALU_##upper_name, NULL, op_a, &op_b);                                        \
                                                                                                                                 \
    result:                                                                                                                      \
        return status;                                                                                                           \
    }

#define simplejs_bytecode_opcode_binary_var(low_name, upper_name)                                                                                  \
    simplejs_status_t simplejs_bytecode_opcode_##low_name##_var(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction)                   \
    {                                                                                                                                              \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                                                        \
        simplejs_variable_t *v_a = &vm->state.variables[instruction->reg_1];                                                                       \
        simplejs_variable_t *v_b = &vm->state.variables[instruction->reg_2];                                                                       \
        simplejs_variable_t *v_out = &vm->state.variables[instruction->imm & 0x0F];                                                                \
                                                                                                                                                   \
        if (v_a->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)                                                                                            \
        {                                                                                                                                          \
            simplejs_printf("v_a->type is not number (%u)\n", v_a->type);                                                                          \
            status = SIMPLEJS_STATUS_PROGRAM_CRASHED;                                                                                              \
            goto result;                                                                                                                           \
        }                                                                                                                                          \
                                                                                                                                                   \
        if (v_b->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)                                                                                            \
        {                                                                                                                                          \
            simplejs_printf("v_b->type is not number (%u)\n", v_b->type);                                                                          \
            status = SIMPLEJS_STATUS_PROGRAM_CRASHED;                                                                                              \
            goto result;                                                                                                                           \
        }                                                                                                                                          \
                                                                                                                                                   \
        simplejs_number_t *op_a = &v_a->value.number;                                                                                              \
        simplejs_number_t *op_b = &v_b->value.number;                                                                                              \
        simplejs_variable_t tmp_out = {.type = SIMPLEJS_VARIABLE_TYPE_NUMBER};                                                                     \
                                                                                                                                                   \
        simplejs_number_execute_alu(SIMPLEJS_NUMBER_ALU_##upper_name, &tmp_out.value.number, op_a, op_b);                                          \
        simplejs_variable_assign(v_out, &tmp_out);                                                                                                 \
        /*printf("result (type %u) = %f\n", v_out->type, simplejs_number_get_float64_jumptable[v_out->value.number.type](&v_out->value.number));*/ \
                                                                                                                                                   \
    result:                                                                                                                                        \
        return status;                                                                                                                             \
    }

simplejs_bytecode_opcode_unary_var(inc, INC);
simplejs_bytecode_opcode_unary_var(dec, DEC);

simplejs_bytecode_opcode_unary_var(not, NOT);
simplejs_bytecode_opcode_unary_var(neg, NEG);

simplejs_bytecode_opcode_binary_var(equal, EQUAL);
simplejs_bytecode_opcode_binary_var(not_equal, NOT_EQUAL);
simplejs_bytecode_opcode_binary_var(greater, GREATER);
simplejs_bytecode_opcode_binary_var(below, BELOW);
simplejs_bytecode_opcode_binary_var(greater_equal, GREATER_EQUAL);
simplejs_bytecode_opcode_binary_var(below_equal, BELOW_EQUAL);

simplejs_bytecode_opcode_binary_var(or, OR);
simplejs_bytecode_opcode_binary_var(and, AND);

simplejs_bytecode_opcode_binary_var(shl, SHL);
simplejs_bytecode_opcode_binary_var(shr, SHR);
simplejs_bytecode_opcode_binary_var(sal, SAL);
simplejs_bytecode_opcode_binary_var(sar, SAR);

simplejs_bytecode_opcode_binary_var(add, ADD);
simplejs_bytecode_opcode_binary_var(sub, SUB);
simplejs_bytecode_opcode_binary_var(mul, MUL);
simplejs_bytecode_opcode_binary_var(div, DIV);
simplejs_bytecode_opcode_binary_var(mod, MOD);

simplejs_bytecode_opcode_jumptable_t simplejs_bytecode_opcode_jumptable[SIMPLEJS_BYTECODE_OPCODE_END] = {
    [SIMPLEJS_BYTECODE_OPCODE_MOV_VAR] = simplejs_bytecode_opcode_mov_var,

    [SIMPLEJS_BYTECODE_OPCODE_ADD_STACK] = simplejs_bytecode_opcode_add_stack,
    [SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE] = simplejs_bytecode_opcode_add_stack_var_size,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_VAR] = simplejs_bytecode_opcode_save_var,
    [SIMPLEJS_BYTECODE_OPCODE_RESTORE_VAR] = simplejs_bytecode_opcode_restore_var,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX] = simplejs_bytecode_opcode_save_ctx,
    [SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX] = simplejs_bytecode_opcode_restore_ctx,

    [SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET] = simplejs_bytecode_opcode_init_loc_offset,
    [SIMPLEJS_BYTECODE_OPCODE_INIT_ARG_OFFSET] = simplejs_bytecode_opcode_init_arg_offset,
    [SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET] = simplejs_bytecode_opcode_save_arg_offset,

    [SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS] = simplejs_bytecode_opcode_alloc_args,
    [SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS] = simplejs_bytecode_opcode_free_args,

    [SIMPLEJS_BYTECODE_OPCODE_SET_RETURN_VAR] = simplejs_bytecode_opcode_set_return_var,

    [SIMPLEJS_BYTECODE_OPCODE_INIT_VAR] = simplejs_bytecode_opcode_init_var,
    [SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_VAR] = simplejs_bytecode_opcode_init_loc_var,
    [SIMPLEJS_BYTECODE_OPCODE_FREE_LOC_VAR] = simplejs_bytecode_opcode_free_loc_var,

    [SIMPLEJS_BYTECODE_OPCODE_GET_VAR_PROP] = simplejs_bytecode_opcode_get_var_prop,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_PROP] = simplejs_bytecode_opcode_set_var_prop,

    [SIMPLEJS_BYTECODE_OPCODE_GET_GLOBAL_VAR] = simplejs_bytecode_opcode_get_global_var,

    [SIMPLEJS_BYTECODE_OPCODE_GET_LOC_VAR] = simplejs_bytecode_opcode_get_loc_var,
    [SIMPLEJS_BYTECODE_OPCODE_SET_LOC_VAR] = simplejs_bytecode_opcode_set_loc_var,

    [SIMPLEJS_BYTECODE_OPCODE_GET_CALL_ARG_VAR] = simplejs_bytecode_opcode_get_call_arg_var,
    [SIMPLEJS_BYTECODE_OPCODE_SET_CALL_ARG_VAR] = simplejs_bytecode_opcode_set_call_arg_var,

    [SIMPLEJS_BYTECODE_OPCODE_GET_FUNC_ARG_VAR] = simplejs_bytecode_opcode_get_func_arg_var,
    [SIMPLEJS_BYTECODE_OPCODE_SET_FUNC_ARG_VAR] = simplejs_bytecode_opcode_set_func_arg_var,

    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NUMBER] = simplejs_bytecode_opcode_set_var_number,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_NATIVE_FUNCTION] = simplejs_bytecode_opcode_set_var_native_function,
    [SIMPLEJS_BYTECODE_OPCODE_SET_VAR_FAST_STRING] = simplejs_bytecode_opcode_set_var_fast_string,

    [SIMPLEJS_BYTECODE_OPCODE_CALL] = simplejs_bytecode_opcode_call,
    [SIMPLEJS_BYTECODE_OPCODE_RETURN] = simplejs_bytecode_opcode_return,

    [SIMPLEJS_BYTECODE_OPCODE_JMP] = simplejs_bytecode_opcode_jmp,
    [SIMPLEJS_BYTECODE_OPCODE_JMP_IF_ZERO] = simplejs_bytecode_opcode_jmp_if_zero,
    [SIMPLEJS_BYTECODE_OPCODE_JMP_IF_NOT_ZERO] = simplejs_bytecode_opcode_jmp_if_not_zero,

    [SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR] = simplejs_bytecode_opcode_convert_boolean_var,

    [SIMPLEJS_BYTECODE_OPCODE_INC_VAR] = simplejs_bytecode_opcode_inc_var,
    [SIMPLEJS_BYTECODE_OPCODE_DEC_VAR] = simplejs_bytecode_opcode_dec_var,

    [SIMPLEJS_BYTECODE_OPCODE_NOT_VAR] = simplejs_bytecode_opcode_not_var,
    [SIMPLEJS_BYTECODE_OPCODE_NEG_VAR] = simplejs_bytecode_opcode_neg_var,

    [SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR] = simplejs_bytecode_opcode_equal_var,
    [SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR] = simplejs_bytecode_opcode_not_equal_var,
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR] = simplejs_bytecode_opcode_greater_var,
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR] = simplejs_bytecode_opcode_below_var,
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_EQUAL_VAR] = simplejs_bytecode_opcode_greater_equal_var,
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_EQUAL_VAR] = simplejs_bytecode_opcode_below_equal_var,

    [SIMPLEJS_BYTECODE_OPCODE_OR_VAR] = simplejs_bytecode_opcode_or_var,
    [SIMPLEJS_BYTECODE_OPCODE_AND_VAR] = simplejs_bytecode_opcode_and_var,

    [SIMPLEJS_BYTECODE_OPCODE_SHL_VAR] = simplejs_bytecode_opcode_shl_var,
    [SIMPLEJS_BYTECODE_OPCODE_SHR_VAR] = simplejs_bytecode_opcode_shr_var,
    [SIMPLEJS_BYTECODE_OPCODE_SAL_VAR] = simplejs_bytecode_opcode_sal_var,
    [SIMPLEJS_BYTECODE_OPCODE_SAR_VAR] = simplejs_bytecode_opcode_sar_var,

    [SIMPLEJS_BYTECODE_OPCODE_ADD_VAR] = simplejs_bytecode_opcode_add_var,
    [SIMPLEJS_BYTECODE_OPCODE_SUB_VAR] = simplejs_bytecode_opcode_sub_var,
    [SIMPLEJS_BYTECODE_OPCODE_MUL_VAR] = simplejs_bytecode_opcode_mul_var,
    [SIMPLEJS_BYTECODE_OPCODE_DIV_VAR] = simplejs_bytecode_opcode_div_var,
    [SIMPLEJS_BYTECODE_OPCODE_MOD_VAR] = simplejs_bytecode_opcode_mod_var,
};

simplejs_status_t SIMPLEJS_API simplejs_execute_vm(simplejs_vm_t *vm)
{
    uint8_t instruction_size = 0;
    simplejs_bytecode_instruction_t instruction;

    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (vm->state.vm_stopped)
    {
        status = vm->state.status;
        goto result;
    }

    SIMPLEJS_ASSERT(vm != NULL);
    SIMPLEJS_ASSERT(vm->memory != NULL);
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

    simplejs_bytecode_decode(&instruction, (uint8_t *)vm->state.instruction_pointer, &instruction_size);

    // simplejs_printf("/* ip: %p */ ", (void *)vm->state.instruction_pointer);
    // simplejs_disasm_bytecode(instruction, vm->state.instruction_pointer + instruction_size);

    vm->state.instruction_pointer += instruction_size;

    if (instruction.opcode >= SIMPLEJS_BYTECODE_OPCODE_END)
    {
        status = SIMPLEJS_STATUS_INVALID_OPCODE;
        goto result;
    }

    simplejs_bytecode_opcode_jumptable_t opcode_handler = simplejs_bytecode_opcode_jumptable[instruction.opcode];
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
        vm->state.status = status;
        vm->state.vm_stopped = true;
    }

    return status;
}
