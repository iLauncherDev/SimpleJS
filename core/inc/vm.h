#pragma once
#include "bytecode.h"
#include "object.h"
#include "compiler.h"
#include <simplejs/vm.h>

typedef struct simplejs_vm_context
{
    uintptr_t saved_argument_offset;
    uintptr_t argument_offset;
    uintptr_t local_var_offset;
} simplejs_vm_context_t;

typedef struct simplejs_vm_return
{
    uintptr_t saved_argument_offset, argument_offset;

    uintptr_t instruction_pointer;
    bool vm_exit, free_args;
} simplejs_vm_return_t;

typedef struct simplejs_vm_state
{
    simplejs_status_t status;

    simplejs_variable_t return_variable;
    simplejs_variable_t variables[16];
    simplejs_variable_t global_variable;

    uintptr_t stack_offset;
    uintptr_t saved_argument_offset;
    uintptr_t argument_offset;
    uintptr_t local_var_offset;

    uintptr_t instruction_pointer;

    bool vm_stopped;
} simplejs_vm_state_t;

typedef struct simplejs_reverse_vm_state
{
    uintptr_t stack_offset;

    bool block_restore_ctx;
    bool block_free_args;

    bool vm_stopped;
} simplejs_reverse_vm_state_t;

#define SIMPLEJS_VM_MEMORY_PAGE_ALLOCATED_FLAG (1 << 0)

typedef struct simplejs_vm_memory_page
{
    uint16_t allocated_pages;
    uint16_t flags;
} simplejs_vm_memory_page_t;

struct simplejs_vm_memory
{
    uint32_t default_permissions;

    uintptr_t memory_size_in_bytes, memory_size_in_pages;

    simplejs_spinlock_t memory_pages_lock;
    simplejs_vm_memory_page_t *memory_pages;

    uint8_t *memory_data;
};

struct simplejs_vm
{
    simplejs_vm_memory_t *memory;

    simplejs_vm_state_t state;
    simplejs_reverse_vm_state_t reverse_state;

    uint8_t *stack;
    size_t stack_size;

    uint8_t *reverse_stack;
    size_t reverse_stack_size;
};

typedef simplejs_status_t (*simplejs_bytecode_opcode_jumptable_t)(simplejs_vm_t *vm, simplejs_bytecode_instruction_t *instruction);

simplejs_status_t simplejs_execute_reverse_vm(simplejs_vm_t *vm);
