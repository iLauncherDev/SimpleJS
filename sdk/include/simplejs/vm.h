#pragma once
#include "object.h"
#include "linemap.h"

typedef struct simplejs_vm_memory simplejs_vm_memory_t;
typedef struct simplejs_vm simplejs_vm_t;

typedef struct simplejs_vm_executable simplejs_vm_executable_t, *psimplejs_vm_executable_t;

simplejs_status_t SIMPLEJS_API simplejs_create_vm_memory(size_t size, simplejs_vm_memory_t **out);
void SIMPLEJS_API simplejs_destroy_vm_memory(simplejs_vm_memory_t *vm_memory);
bool SIMPLEJS_API simplejs_vm_memory_find_alloc_start(simplejs_vm_memory_t *vm_memory, pvoid address, void **out, size_t *out_size);
pvoid SIMPLEJS_API simplejs_vm_memory_alloc(simplejs_vm_memory_t *vm_memory, size_t size_in_bytes);
void SIMPLEJS_API simplejs_vm_memory_free(simplejs_vm_memory_t *vm_memory, void *ptr);

void SIMPLEJS_API simplejs_vm_executable_get_pointer(
    simplejs_vm_executable_t *vm_executable, void **out_executable, size_t *out_executable_size);
psimplejs_vm_executable_t SIMPLEJS_API simplejs_vm_upload_executable(
    simplejs_vm_memory_t *vm_memory, simplejs_linemap_ctx_t *linemap_ctx, void *executable, size_t executable_size);

simplejs_status_t SIMPLEJS_API simplejs_create_vm(simplejs_vm_t **out);
void SIMPLEJS_API simplejs_reset_vm(simplejs_vm_t *vm);
void SIMPLEJS_API simplejs_destroy_vm(simplejs_vm_t *vm);

uintptr_t SIMPLEJS_API simplejs_vm_get_instruction_pointer(simplejs_vm_t *vm);

void SIMPLEJS_API simplejs_vm_set_memory(simplejs_vm_t *vm, simplejs_vm_memory_t *vm_memory);

void SIMPLEJS_API simplejs_vm_set_global_variable(simplejs_vm_t *vm, simplejs_variable_t *global_variable);

simplejs_status_t SIMPLEJS_API simplejs_vm_call_function(simplejs_vm_t *vm,
                                                         simplejs_function_t *function,
                                                         simplejs_variable_t *return_variable,
                                                         simplejs_variable_t *arguments, uint32_t argument_count,
                                                         bool vm_exit);

simplejs_status_t SIMPLEJS_API simplejs_execute_vm(simplejs_vm_t *vm);

void SIMPLEJS_API simplejs_reset_reverse_vm(simplejs_vm_t *vm);
