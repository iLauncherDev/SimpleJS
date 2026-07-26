#include <vm.h>
#include <compiler.h>

void SIMPLEJS_API simplejs_vm_executable_get_pointer(
    simplejs_vm_executable_t *vm_executable, void **out_executable, size_t *out_executable_size)
{
    *out_executable = (uint8_t*)vm_executable + vm_executable->header_size;
    *out_executable_size = vm_executable->executable_size;
}

psimplejs_vm_executable_t SIMPLEJS_API simplejs_vm_upload_executable(
    simplejs_vm_memory_t *vm_memory, simplejs_linemap_ctx_t *linemap_ctx, void *executable, size_t executable_size)
{
    simplejs_vm_executable_t *vm_executable = simplejs_vm_memory_alloc(vm_memory, sizeof(*vm_executable) + executable_size);
    if (!vm_executable)
    {
        goto result;
    }
    memclr(vm_executable, sizeof(*vm_executable));

    vm_executable->header_size = sizeof(*vm_executable);
    vm_executable->executable_size = executable_size;
    vm_executable->linemap_ctx = linemap_ctx;
    memcpy((uint8_t *)vm_executable + vm_executable->header_size, executable, vm_executable->executable_size);

result:
    return vm_executable;
}
