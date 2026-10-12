#include <lib/fnv1a_hash.h>

uint64_t SIMPLEJS_API simplejs_generate_fnv1a_hash(const void *data, size_t data_size, bool has_null_terminator)
{
    const uint8_t *current_data = data;
    size_t remaining_size = data_size;

    uint64_t hash = 14695981039346656037ULL;
    uint64_t hash_mul = 1099511628211ULL;

    while (remaining_size > 0)
    {
        uint8_t value = *(uint8_t *)current_data;
        if (has_null_terminator && value == '\0')
            break;

        hash ^= value;
        hash *= hash_mul;

        remaining_size--, current_data++;
    }

    return hash;
}
