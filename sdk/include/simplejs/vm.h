#pragma once
#include "object.h"

typedef struct simplejs_vm_memory simplejs_vm_memory_t;
typedef struct simplejs_vm simplejs_vm_t;

simplejs_status_t SIMPLEJS_API simplejs_create_vm_memory(size_t size, simplejs_vm_memory_t **out);
void SIMPLEJS_API simplejs_destroy_vm_memory(simplejs_vm_memory_t *vm_memory);
bool SIMPLEJS_API simplejs_vm_memory_find_alloc_start(simplejs_vm_memory_t *vm_memory, pvoid address, void **out, size_t *out_size);
pvoid SIMPLEJS_API simplejs_vm_memory_alloc(simplejs_vm_memory_t *vm_memory, size_t size_in_bytes);
void SIMPLEJS_API simplejs_vm_memory_free(simplejs_vm_memory_t *vm_memory, void *ptr);

simplejs_status_t SIMPLEJS_API simplejs_create_vm(simplejs_vm_t **out);
void SIMPLEJS_API simplejs_reset_vm(simplejs_vm_t *vm);
void SIMPLEJS_API simplejs_destroy_vm(simplejs_vm_t *vm);

void SIMPLEJS_API simplejs_vm_set_memory(simplejs_vm_t *vm, simplejs_vm_memory_t *vm_memory);

void SIMPLEJS_API simplejs_vm_set_global_variable(simplejs_vm_t *vm, simplejs_variable_t *global_variable);

simplejs_status_t SIMPLEJS_API simplejs_vm_call_function(simplejs_vm_t *vm,
                                                         simplejs_function_t *function,
                                                         simplejs_variable_t *return_variable,
                                                         simplejs_variable_t *arguments, uint32_t argument_count,
                                                         bool vm_exit);

simplejs_status_t SIMPLEJS_API simplejs_execute_vm(simplejs_vm_t *vm);

void SIMPLEJS_API simplejs_reset_reverse_vm(simplejs_vm_t *vm);
