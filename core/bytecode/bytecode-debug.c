#include <bytecode.h>

pvoid simplejs_bytecode_find_debug_info_by_stop_pointer(void *executable, size_t executable_size, void *stop_pointer)
{
    uint32_t relative_stop_pointer = (uint32_t)((uintptr_t)stop_pointer - (uintptr_t)executable);

    simplejs_bytecode_header_t bytecode_header = {0};
    simplejs_bytecode_header_decode(executable, &bytecode_header);

    if (!bytecode_header.debug_info_entry_size ||
        (~bytecode_header.flags & SIMPLEJS_BYTECODE_HEADER_DEBUG_INFO_FLAG))
        return NULL;

    uint8_t *start_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.start;
    uint8_t *end_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.end - bytecode_header.debug_info_entry_size;
    uint8_t *current_debug_info = start_debug_info;

    while (current_debug_info >= start_debug_info &&
           current_debug_info <= end_debug_info)
    {
        simplejs_bytecode_debug_info_t bytecode_debug_info = {0};
        simplejs_bytecode_debug_info_decode(current_debug_info, &bytecode_debug_info);

        if (relative_stop_pointer >= bytecode_debug_info.code_offset.start &&
            relative_stop_pointer < bytecode_debug_info.code_offset.end)
            return current_debug_info;

        current_debug_info += bytecode_header.debug_info_entry_size;
    }

    return NULL;
}

pvoid simplejs_bytecode_find_debug_info_by_source_offset(void *executable, size_t executable_size, simplejs_linemap_offset_t source_offset, uint32_t required_flags)
{
    simplejs_bytecode_header_t bytecode_header = {0};
    simplejs_bytecode_header_decode(executable, &bytecode_header);

    if (!bytecode_header.debug_info_entry_size ||
        (~bytecode_header.flags & SIMPLEJS_BYTECODE_HEADER_DEBUG_INFO_FLAG))
        return NULL;

    uint8_t *start_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.start;
    uint8_t *end_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.end - bytecode_header.debug_info_entry_size;
    uint8_t *current_debug_info = start_debug_info;

    while (current_debug_info >= start_debug_info &&
           current_debug_info <= end_debug_info)
    {
        simplejs_bytecode_debug_info_t bytecode_debug_info = {0};
        simplejs_bytecode_debug_info_decode(current_debug_info, &bytecode_debug_info);

        if (bytecode_debug_info.source_offset.start == source_offset.start &&
            bytecode_debug_info.source_offset.end == source_offset.end &&
            bytecode_debug_info.flags == required_flags)
            return current_debug_info;

        current_debug_info += bytecode_header.debug_info_entry_size;
    }

    return NULL;
}

pvoid simplejs_bytecode_find_children_debug_info(void *executable, size_t executable_size, pvoid debug_info, uint32_t required_debug_flags, uint32_t children_debug_flags)
{
    required_debug_flags |= SIMPLEJS_BYTECODE_DEBUG_INFO_DIAGNOSTIC_FLAG;
    children_debug_flags |= SIMPLEJS_BYTECODE_DEBUG_INFO_DIAGNOSTIC_FLAG;

    simplejs_bytecode_header_t bytecode_header = {0};
    simplejs_bytecode_header_decode(executable, &bytecode_header);

    if (!bytecode_header.debug_info_entry_size ||
        (~bytecode_header.flags & SIMPLEJS_BYTECODE_HEADER_DEBUG_INFO_FLAG))
        return NULL;

    uint8_t *start_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.start;
    uint8_t *end_debug_info = (uint8_t *)executable + bytecode_header.debug_info_offset.end - bytecode_header.debug_info_entry_size;
    if (debug_info < (pvoid)start_debug_info &&
        debug_info > (pvoid)end_debug_info)
        return NULL;

    simplejs_bytecode_debug_info_t bytecode_debug_info = {0};
    simplejs_bytecode_debug_info_decode(debug_info, &bytecode_debug_info);
    if ((bytecode_debug_info.flags & required_debug_flags) != required_debug_flags)
        return NULL;

    uint32_t current_children_count = bytecode_debug_info.children_debug_count;
    uint8_t *current_debug_info = (uint8_t *)debug_info + bytecode_header.debug_info_entry_size;

    while (current_debug_info >= start_debug_info &&
           current_debug_info <= end_debug_info &&
           current_children_count > 0)
    {
        simplejs_bytecode_debug_info_decode(current_debug_info, &bytecode_debug_info);

        if ((bytecode_debug_info.flags & children_debug_flags) == children_debug_flags)
            return current_debug_info;

        current_debug_info += bytecode_header.debug_info_entry_size * (bytecode_debug_info.children_debug_count + 1);
        current_children_count--;
    }

    return NULL;
}
