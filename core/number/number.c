#include <number.h>

void simplejs_number_encode(uint8_t *buffer, simplejs_number_t *number)
{
    *buffer++ = (number->type >> 0) & 0xFF;
    *buffer++ = (number->type >> 8) & 0xFF;
    buffer += 6;

    *buffer++ = (number->value.generic >> 0) & 0xFF;
    *buffer++ = (number->value.generic >> 8) & 0xFF;
    *buffer++ = (number->value.generic >> 16) & 0xFF;
    *buffer++ = (number->value.generic >> 24) & 0xFF;
    *buffer++ = (number->value.generic >> 32) & 0xFF;
    *buffer++ = (number->value.generic >> 40) & 0xFF;
    *buffer++ = (number->value.generic >> 48) & 0xFF;
    *buffer++ = (number->value.generic >> 56) & 0xFF;
}

void simplejs_number_decode(simplejs_number_t *number, uint8_t *buffer)
{
    uint16_t b1 = *buffer++;
    uint16_t b2 = *buffer++;
    buffer += 6;

    uint64_t b9 = *buffer++;
    uint64_t b10 = *buffer++;
    uint64_t b11 = *buffer++;
    uint64_t b12 = *buffer++;
    uint64_t b13 = *buffer++;
    uint64_t b14 = *buffer++;
    uint64_t b15 = *buffer++;
    uint64_t b16 = *buffer++;

    number->type = (b1 << 0) | (b2 << 8);

    number->value.generic = (b9 << 0) | (b10 << 8) | (b11 << 16) | (b12 << 24) |
                            (b13 << 32) | (b14 << 40) | (b15 << 48) | (b16 << 56);
}
