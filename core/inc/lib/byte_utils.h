#pragma once
#include "../default.h"

#define simplejs_read_little_endian_field(field) simplejs_read_little_endian_value(field, sizeof(*(field)))
#define simplejs_write_little_endian_field(field, value) simplejs_write_little_endian_value(field, sizeof(*(field)), value)

#define simplejs_buffer_to_struct_field(ptr, field, cast, buffer) \
    *(field) = (cast)simplejs_read_little_endian_value(&((uint8_t *)(buffer))[(size_t)(field) - (size_t)(ptr)], sizeof(*(field)))
#define simplejs_struct_field_to_buffer(ptr, field, cast, buffer) \
    simplejs_write_little_endian_value(&((uint8_t *)(buffer))[(size_t)(field) - (size_t)(ptr)], sizeof(*(field)), (cast)(*(field)))

uint64_t simplejs_read_little_endian_value(void *ptr, uint8_t bytes);
void simplejs_write_little_endian_value(void *ptr, uint8_t bytes, uint64_t value);
