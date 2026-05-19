#include <vm.h>

static simplejs_status_t simplejs_check_memory_pointer_raw(
    simplejs_status_t error_status,
    void *stack, uintptr_t stack_size, void *pointer, uintptr_t structure_size)
{
    simplejs_status_t status = error_status;
    uintptr_t stack_start = (uintptr_t)stack;
    uintptr_t stack_end = stack_start + stack_size;
    uintptr_t structure = (uintptr_t)pointer;

    if ((uintptr_t)(structure - stack_start) <= (uintptr_t)(stack_end - stack_start - structure_size))
    {
        status = SIMPLEJS_STATUS_SUCCESS;
        goto result;
    }

    printf("%p out of (%p, %p) boundary\n", (void *)structure, (void *)stack_start, (void *)stack_end);

result:
    return status;
}

static simplejs_status_t simplejs_check_stack_pointer(simplejs_vm_t *vm, void *pointer, uintptr_t structure_size)
{
    return simplejs_check_memory_pointer_raw(
        SIMPLEJS_STATUS_STACK_OUT_OF_BOUNDS,
        vm->stack, vm->stack_size, pointer, structure_size);
}

static simplejs_status_t simplejs_check_memory_pointer(simplejs_vm_t *vm, void *pointer, uintptr_t structure_size)
{
    return simplejs_check_memory_pointer_raw(
        SIMPLEJS_STATUS_MEMORY_OUT_OF_BOUNDS,
        vm->memory->memory_data, vm->memory->memory_size_in_bytes, pointer, structure_size);
}

#define simplejs_check_stack_struct(vm, pointer) simplejs_check_stack_pointer(vm, pointer, sizeof(*pointer))
#define simplejs_check_memory_struct(vm, pointer) simplejs_check_memory_pointer(vm, pointer, sizeof(*pointer))

static simplejs_status_t simplejs_reverse_check_stack_pointer(simplejs_vm_t *vm, void *pointer, uintptr_t structure_size)
{
    return simplejs_check_memory_pointer_raw(
        SIMPLEJS_STATUS_STACK_OUT_OF_BOUNDS,
        vm->reverse_stack, vm->reverse_stack_size, pointer, structure_size);
}

#define simplejs_reverse_check_stack_struct(vm, pointer) simplejs_reverse_check_stack_pointer(vm, pointer, sizeof(*pointer))

static inline void simplejs_vm_add_stack(uintptr_t *stack, intptr_t offset)
{
    intptr_t alignment = sizeof(uintptr_t) * 2;

    intptr_t mask = offset >> (sizeof(offset) * 8 - 1);
    intptr_t abs_offset = (offset ^ mask) - mask;

    abs_offset = (abs_offset + (alignment - 1)) & ~(alignment - 1);

    intptr_t new_offset = (abs_offset ^ mask) - mask;

    (*stack) += new_offset;
}

static inline simplejs_status_t simplejs_get_function_header(simplejs_vm_t *vm, simplejs_function_header_t **out, uintptr_t offset)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    simplejs_function_header_t *function_header = (void *)(vm->stack + offset);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, function_header), result, status);

    uint32_t function_header_size = sizeof(*function_header) + (sizeof(*function_header->arguments) * function_header->argument_count);

    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_pointer(vm, function_header, function_header_size), result, status);

    *out = function_header;

result:
    return status;
}

static inline simplejs_status_t simplejs_vm_pop_return_from_stack(simplejs_vm_t *vm, simplejs_vm_return_t **out)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;
    simplejs_vm_return_t *vm_return = NULL;

    simplejs_vm_add_stack(&vm->state.stack_offset, -sizeof(*vm_return));

    vm_return = (void *)(vm->stack + vm->state.stack_offset);
    SIMPLEJS_REQUIRE_SUCCESS(simplejs_check_stack_struct(vm, vm_return), result, status);

    *out = vm_return;

result:
    return status;
}

simplejs_status_t simplejs_bytecode_opcode_restore_ctx(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction);
simplejs_status_t simplejs_bytecode_opcode_free_args(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction);
simplejs_status_t simplejs_bytecode_opcode_return(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction);
