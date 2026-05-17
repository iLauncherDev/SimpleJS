#pragma once
#include "bytecode.h"
#include "object.h"
#include <simplejs/vm.h>

typedef struct simplejs_bytecode_context
{
    simplejs_variable_t variables[16];

    uintptr_t saved_argument_offset;
    uintptr_t argument_offset;
    uintptr_t local_var_offset;
} simplejs_bytecode_context_t;

typedef struct simplejs_bytecode_vm_state
{
    bool vm_stopped;

    simplejs_variable_t variables[16];

    uintptr_t stack_offset;
    uintptr_t saved_argument_offset;
    uintptr_t argument_offset;
    uintptr_t local_var_offset;

    uintptr_t instruction_pointer;

    simplejs_variable_t global_variable;
} simplejs_bytecode_vm_state_t;

struct simplejs_bytecode_vm
{
    simplejs_bytecode_vm_state_t state;

    uint8_t *stack;
    size_t stack_size;
};
