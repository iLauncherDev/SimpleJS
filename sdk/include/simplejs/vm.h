#pragma once
#include "object.h"

typedef struct simplejs_bytecode_vm simplejs_bytecode_vm_t;

simplejs_status_t SIMPLEJS_API simplejs_create_vm(simplejs_bytecode_vm_t **out);
void SIMPLEJS_API simplejs_destroy_vm(simplejs_bytecode_vm_t *vm);
void SIMPLEJS_API simplejs_reset_vm(simplejs_bytecode_vm_t *vm);
simplejs_status_t SIMPLEJS_API simplejs_execute_vm(simplejs_bytecode_vm_t *vm);
