#include <vm.h>

#define STACK_SIZE (64 * 1024)

simplejs_status_t SIMPLEJS_API simplejs_create_vm(simplejs_bytecode_vm_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_bytecode_vm_t *ret = simplejs_hook_malloc(sizeof(*ret));
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

    *out = ret;

result:
    return status;
}

void SIMPLEJS_API simplejs_destroy_vm(simplejs_bytecode_vm_t *vm)
{
    simplejs_variable_t undef_var;
    undef_var.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    undef_var.value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;

    simplejs_variable_assign(&vm->state.global_variable, &undef_var);

    if (vm->stack)
        simplejs_hook_mfree(vm->stack);

    simplejs_hook_mfree(vm);
}

static simplejs_status_t simplejs_check_stack_pointer(simplejs_bytecode_vm_t *vm, void *pointer, uintptr_t structure_size)
{
    simplejs_status_t status = SIMPLEJS_STATUS_STACK_OUT_OF_BOUNDS;
    uintptr_t stack_start = (uintptr_t)vm->stack;
    uintptr_t stack_end = stack_start + vm->stack_size;
    uintptr_t structure = (uintptr_t)pointer;

    if ((uintptr_t)(structure - stack_start) <= (uintptr_t)(stack_end - stack_start - structure_size))
    {
        status = SIMPLEJS_STATUS_SUCCESS;
        goto result;
    }

    printf("%p out of bounds of %p, %p\n", (void *)structure, (void *)stack_start, (void *)stack_end);

result:
    return status;
}

#define simplejs_check_stack_struct(vm, pointer) simplejs_check_stack_pointer(vm, pointer, sizeof(*pointer))

void SIMPLEJS_API simplejs_reset_vm(simplejs_bytecode_vm_t *vm)
{
    simplejs_variable_t undef_variable;
    undef_variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    undef_variable.value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;

    for (size_t i = 0; i < 16; i++)
    {
        simplejs_variable_assign(&vm->state.variables[i], &undef_variable);
    }

    simplejs_variable_assign(&vm->state.global_variable, &undef_variable);

    memclr(&vm->state, sizeof(vm->state));
}

void simplejs_disasm_bytecode(simplejs_bytecode_instruction_t instruction, uintptr_t instruction_pointer);

static inline simplejs_status_t simplejs_get_function_header(simplejs_bytecode_vm_t *vm, simplejs_function_header_t **out, uintptr_t offset)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header = (void *)(vm->stack + offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, function_header), result, status);

    uint32_t function_header_size = sizeof(simplejs_function_header_t) + (sizeof(simplejs_variable_t) * function_header->argument_count);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    *out = function_header;

result:
    return status;
}

static inline void simplejs_vm_add_stack(uintptr_t *stack, intptr_t offset)
{
    intptr_t alignment = sizeof(uintptr_t) * 2;

    intptr_t mask = offset >> (sizeof(offset) * 8 - 1);
    intptr_t abs_offset = (offset ^ mask) - mask;

    abs_offset = (abs_offset + (alignment - 1)) & ~(alignment - 1);

    intptr_t new_offset = (abs_offset ^ mask) - mask;

    (*stack) += new_offset;
}

simplejs_status_t simplejs_bytecode_opcode_mov_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_2];

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_add_stack(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_add_stack_var_size(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_vm_add_stack(&vm->state.stack_offset, instruction->imm_signed * sizeof(simplejs_variable_t));

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_loc_offset(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.local_var_offset = vm->state.stack_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_arg_offset(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.argument_offset = vm->state.stack_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_save_arg_offset(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.saved_argument_offset = vm->state.argument_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_save_ctx(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_bytecode_context_t *context = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, context), result, status);

    for (size_t i = SIMPLEJS_BYTECODE_VARIABLE_FUNC_RETURN + 1; i < 16; i++)
    {
        simplejs_variable_assign(&context->variables[i], &vm->state.variables[i]);
    }

    context->saved_argument_offset = vm->state.saved_argument_offset;
    context->argument_offset = vm->state.argument_offset;
    context->local_var_offset = vm->state.local_var_offset;

    simplejs_vm_add_stack(&vm->state.stack_offset, sizeof(*context));

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_restore_ctx(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_bytecode_context_t *context;

    simplejs_vm_add_stack(&vm->state.stack_offset, -sizeof(*context));

    context = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, context), result, status);

    simplejs_variable_t undef_variable;
    undef_variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    undef_variable.value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;

    for (size_t i = SIMPLEJS_BYTECODE_VARIABLE_FUNC_RETURN + 1; i < 16; i++)
    {
        simplejs_variable_assign(&vm->state.variables[i], &context->variables[i]);
        simplejs_variable_assign(&context->variables[i], &undef_variable);
    }

    vm->state.saved_argument_offset = context->saved_argument_offset;
    vm->state.argument_offset = context->argument_offset;
    vm->state.local_var_offset = context->local_var_offset;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_alloc_args(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uintptr_t function_header_size = sizeof(simplejs_function_header_t) + (sizeof(simplejs_variable_t) * instruction->imm);
    simplejs_function_header_t *function_header = (void *)(vm->stack + vm->state.argument_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    function_header->argument_count = instruction->imm;

    simplejs_vm_add_stack(&vm->state.stack_offset, function_header_size);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_free_args(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uintptr_t function_header_size = sizeof(simplejs_function_header_t) + (sizeof(simplejs_variable_t) * instruction->imm);
    simplejs_vm_add_stack(&vm->state.stack_offset, -function_header_size);

    simplejs_function_header_t *function_header = (void *)(vm->stack + vm->state.argument_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    simplejs_variable_t undef_variable;
    undef_variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    undef_variable.value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;

    simplejs_variable_assign(&function_header->this_variable, &undef_variable);

    for (size_t i = 0; i < instruction->imm; i++)
    {
        simplejs_variable_assign(&function_header->arguments[i], &undef_variable);
    }

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &vm->state.variables[instruction->imm & 0x0F];

    simplejs_number_t tmp_num = {.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED};

    simplejs_variable_dereference(variable);
    simplejs_variable_init_number(variable, &tmp_num);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_init_loc_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *variable = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, variable), result, status);

    simplejs_number_t tmp_num = {.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED};

    simplejs_variable_dereference(variable);
    simplejs_variable_init_number(variable, &tmp_num);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_free_loc_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t undef_variable;
    undef_variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    undef_variable.value.number.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED;

    simplejs_variable_t *variable = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, variable), result, status);

    simplejs_variable_assign(variable, &undef_variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_var_prop(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_set_var_prop(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_get_global_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.global_variable;

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_loc_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, in), result, status);

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_loc_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &((simplejs_variable_t *)(vm->stack + vm->state.local_var_offset))[instruction->imm];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, out), result, status);

    simplejs_variable_assign(out, in);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_get_call_arg_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_set_call_arg_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_get_func_arg_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_set_func_arg_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_set_var_number(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t variable;
    variable.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;
    simplejs_number_decode(&variable.value.number, (uint8_t *)(vm->state.instruction_pointer + instruction->imm_signed));

    simplejs_variable_assign(&vm->state.variables[instruction->reg_1], &variable);

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_set_var_native_function(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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

simplejs_status_t simplejs_bytecode_opcode_set_var_fast_string(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
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
    simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    uintptr_t *ip_backup = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, ip_backup), result, status);

    *ip_backup = vm->state.instruction_pointer;
    simplejs_vm_add_stack(&vm->state.stack_offset, sizeof(*ip_backup));

    vm->state.instruction_pointer = function->value.instruction_pointer;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_call_proxy(
    simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_variable_t *out = &vm->state.variables[SIMPLEJS_BYTECODE_VARIABLE_FUNC_RETURN];

    SIMPLEJS_ASSERT(function->value.proxy != NULL);

    simplejs_variable_assign(&function_header->this_variable, &vm->state.variables[SIMPLEJS_BYTECODE_VARIABLE_THIS]);
    SIMPLEJS_REQUIRE_SUCCESS(function->value.proxy(function_header, out), result, status);

result:
    return status;
}

typedef simplejs_status_t (*simplejs_bytecode_opcode_call_jumptable_t)(
    simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction,
    simplejs_function_header_t *function_header, simplejs_function_t *function);

simplejs_bytecode_opcode_call_jumptable_t simplejs_bytecode_opcode_call_jumptable[SIMPLEJS_FUNCTION_TYPE_END] = {
    [SIMPLEJS_FUNCTION_TYPE_NATIVE] = simplejs_bytecode_opcode_call_native,
    [SIMPLEJS_FUNCTION_TYPE_PROXY] = simplejs_bytecode_opcode_call_proxy,
};

simplejs_status_t simplejs_bytecode_opcode_call(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header;
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_get_function_header(vm, &function_header, vm->state.argument_offset), result, status);

    simplejs_variable_t *in = &vm->state.variables[instruction->reg_1];

    if (in->type != SIMPLEJS_VARIABLE_TYPE_FUNCTION)
    {
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

simplejs_status_t simplejs_bytecode_opcode_return(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uintptr_t *ip_backup;

    simplejs_vm_add_stack(&vm->state.stack_offset, -sizeof(*ip_backup));

    ip_backup = (void *)(vm->stack + vm->state.stack_offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, ip_backup), result, status);

    vm->state.instruction_pointer = *ip_backup;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    vm->state.instruction_pointer += instruction->imm_signed;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp_if_zero(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];

    uint64_t value = simplejs_variable_get_int(variable);
    int32_t condition = !value;

    vm->state.instruction_pointer += instruction->imm_signed * condition;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_jmp_if_not_zero(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *variable = &vm->state.variables[instruction->reg_1];

    uint64_t value = simplejs_variable_get_int(variable);
    int32_t condition = !!value;

    vm->state.instruction_pointer += instruction->imm_signed * condition;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_exit(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_PROGRAM_EXITED;

    simplejs_printf("program exited!\n");

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_convert_boolean_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_variable_t *out = &vm->state.variables[instruction->reg_1];
    simplejs_variable_t *in = &vm->state.variables[instruction->reg_2];

    simplejs_variable_t temp_out_var;
    temp_out_var.type = SIMPLEJS_VARIABLE_TYPE_NUMBER;

    simplejs_number_t *temp_num = &temp_out_var.value.number;
    temp_num->type = SIMPLEJS_NUMBER_TYPE_UI32;
    temp_num->value.ui32 = !!simplejs_variable_get_int(in);

    simplejs_variable_assign(out, &temp_out_var);

    return status;
}

#define simplejs_bytecode_opcode_unary_var(low_name, upper_name)                                                                          \
    simplejs_status_t simplejs_bytecode_opcode_##low_name##_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction) \
    {                                                                                                                                     \
        simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;                                                                               \
        simplejs_variable_t *v_a = &vm->state.variables[instruction->reg_1];                                                              \
                                                                                                                                          \
        if (v_a->type != SIMPLEJS_VARIABLE_TYPE_NUMBER)                                                                                   \
        {                                                                                                                                 \
            simplejs_printf("v_a->type is not number (%u)\n", v_a->type);                                                                 \
            status = SIMPLEJS_STATUS_PROGRAM_CRASHED;                                                                                     \
            goto result;                                                                                                                  \
        }                                                                                                                                 \
                                                                                                                                          \
        simplejs_number_t *op_a = &v_a->value.number;                                                                                     \
        simplejs_number_t op_b = {.type = SIMPLEJS_NUMBER_TYPE_UNDEFINED};                                                                \
                                                                                                                                          \
        simplejs_number_execute_alu(SIMPLEJS_NUMBER_ALU_##upper_name, NULL, op_a, &op_b);                                                 \
                                                                                                                                          \
    result:                                                                                                                               \
        return status;                                                                                                                    \
    }

#define simplejs_bytecode_opcode_binary_var(low_name, upper_name)                                                                                  \
    simplejs_status_t simplejs_bytecode_opcode_##low_name##_var(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction)          \
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
                                                                                                                                                   \
        v_out->type = SIMPLEJS_VARIABLE_TYPE_NUMBER;                                                                                               \
                                                                                                                                                   \
        simplejs_number_execute_alu(SIMPLEJS_NUMBER_ALU_##upper_name, &v_out->value.number, op_a, op_b);                                           \
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

typedef simplejs_status_t (*simplejs_bytecode_opcode_jumptable_t)(simplejs_bytecode_vm_t *vm, simplejs_bytecode_instruction_t *instruction);

simplejs_bytecode_opcode_jumptable_t simplejs_bytecode_opcode_jumptable[SIMPLEJS_BYTECODE_OPCODE_END] = {
    [SIMPLEJS_BYTECODE_OPCODE_MOV_VAR] = simplejs_bytecode_opcode_mov_var,

    [SIMPLEJS_BYTECODE_OPCODE_ADD_STACK] = simplejs_bytecode_opcode_add_stack,
    [SIMPLEJS_BYTECODE_OPCODE_ADD_STACK_VAR_SIZE] = simplejs_bytecode_opcode_add_stack_var_size,

    [SIMPLEJS_BYTECODE_OPCODE_SAVE_CTX] = simplejs_bytecode_opcode_save_ctx,
    [SIMPLEJS_BYTECODE_OPCODE_RESTORE_CTX] = simplejs_bytecode_opcode_restore_ctx,

    [SIMPLEJS_BYTECODE_OPCODE_INIT_LOC_OFFSET] = simplejs_bytecode_opcode_init_loc_offset,
    [SIMPLEJS_BYTECODE_OPCODE_INIT_ARG_OFFSET] = simplejs_bytecode_opcode_init_arg_offset,
    [SIMPLEJS_BYTECODE_OPCODE_SAVE_ARG_OFFSET] = simplejs_bytecode_opcode_save_arg_offset,

    [SIMPLEJS_BYTECODE_OPCODE_ALLOC_ARGS] = simplejs_bytecode_opcode_alloc_args,
    [SIMPLEJS_BYTECODE_OPCODE_FREE_ARGS] = simplejs_bytecode_opcode_free_args,

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

    [SIMPLEJS_BYTECODE_OPCODE_EXIT] = simplejs_bytecode_opcode_exit,

    [SIMPLEJS_BYTECODE_OPCODE_CONVERT_BOOLEAN_VAR] = simplejs_bytecode_opcode_convert_boolean_var,

    [SIMPLEJS_BYTECODE_OPCODE_INC_VAR] = simplejs_bytecode_opcode_inc_var,
    [SIMPLEJS_BYTECODE_OPCODE_DEC_VAR] = simplejs_bytecode_opcode_dec_var,

    [SIMPLEJS_BYTECODE_OPCODE_NOT_VAR] = simplejs_bytecode_opcode_not_var,
    [SIMPLEJS_BYTECODE_OPCODE_NEG_VAR] = simplejs_bytecode_opcode_neg_var,

    [SIMPLEJS_BYTECODE_OPCODE_EQUAL_VAR] = simplejs_bytecode_opcode_equal_var,
    [SIMPLEJS_BYTECODE_OPCODE_NOT_EQUAL_VAR] = simplejs_bytecode_opcode_not_equal_var,
    [SIMPLEJS_BYTECODE_OPCODE_GREATER_VAR] = simplejs_bytecode_opcode_greater_var,
    [SIMPLEJS_BYTECODE_OPCODE_BELOW_VAR] = simplejs_bytecode_opcode_below_var,

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

simplejs_status_t SIMPLEJS_API simplejs_execute_vm(simplejs_bytecode_vm_t *vm)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    if (vm->state.vm_stopped)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
        goto result;
    }

    uint8_t instruction_size;
    simplejs_bytecode_instruction_t instruction;

    SIMPLEJS_ASSERT(vm != NULL);
    SIMPLEJS_ASSERT(vm->state.instruction_pointer != (uintptr_t)NULL);

    // simplejs_printf("/* ip: %p */ ", (void *)vm->state.instruction_pointer);

    simplejs_bytecode_decode(&instruction, (uint8_t *)vm->state.instruction_pointer, &instruction_size);
    vm->state.instruction_pointer += instruction_size;

    // simplejs_disasm_bytecode(instruction, vm->state.instruction_pointer);

    if (instruction.opcode >= SIMPLEJS_BYTECODE_OPCODE_END)
    {
        status = SIMPLEJS_STATUS_PROGRAM_CRASHED;
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
        vm->state.vm_stopped = true;
    }

    return status;
}
