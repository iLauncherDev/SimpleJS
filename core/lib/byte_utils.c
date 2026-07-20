#include <lib/byte_utils.h>

uint64_t simplejs_read_little_endian_value(void *ptr, uint8_t bytes)
{
    uint8_t *byte_ptr = ptr;
    size_t bits = bytes * 8;
    uint64_t value = 0;

    for (size_t off = 0; off < bits; off += 8)
    {
        value |= (uint64_t)(*byte_ptr++) << off;
    }

    return value;
}

void simplejs_write_little_endian_value(void *ptr, uint8_t bytes, uint64_t value)
{
    uint8_t *byte_ptr = ptr;
    size_t bits = bytes * 8;

    for (size_t off = 0; off < bits; off += 8)
    {
        (*byte_ptr++) = (value >> off) & 0xff;
    }
}
