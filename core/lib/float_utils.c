#include <lib/float_utils.h>

#define F64_EPSILON 0.00000000000000015
//                    23456788999999989

void SIMPLEJS_API simplejs_convert_double_to_string(char *buffer, size_t buffer_size, double value)
{
    size_t value_offset = 0;
    size_t index = 0;

    if (value < 0.0)
    {
        value = -value;

        buffer[value_offset++] = '-';
    }

    bool will_start_loop = true;
    uint64_t integer_part = (uint64_t)value;

    while (integer_part != 0 || will_start_loop)
    {
        will_start_loop = false;

        if (value_offset + index + 1 >= buffer_size)
            goto result;

        char digit = '0' + (integer_part % 10);

        memcpy(&buffer[value_offset + 1], &buffer[value_offset], index);

        buffer[value_offset] = '0' + (integer_part % 10);
        index++;

        integer_part /= 10;
    }

    bool initial_decimal_loop = true;
    double decimal_part = value - (uint64_t)value;
    double current_epsilon = F64_EPSILON;

    double pow = 10.0;

    while (decimal_part > current_epsilon)
    {
        if (initial_decimal_loop)
        {
            initial_decimal_loop = false;

            if (value_offset + index + 1 >= buffer_size)
                goto result;

            buffer[value_offset + index] = '.';
            index++;
        }

        if (value_offset + index >= buffer_size)
            goto result;

        decimal_part *= 10.0;

        char digit = '0' + ((uint8_t)decimal_part % 10);

        // printf("decimal_part = %.*f, current_epsilon = %.*f\n", 17, decimal_part, 17, current_epsilon);

        decimal_part += current_epsilon;
        current_epsilon *= pow;

        buffer[value_offset + index] = digit;
        index++;

        decimal_part -= (uint64_t)decimal_part;
    }

result:
    if (value_offset + index < buffer_size)
    {
        buffer[value_offset + index] = '\0';
    }
}
