#include <lib/float_utils.h>

static double simple_pow(double pow, double n)
{
    double val = 1;
    while (n--)
        val *= pow;

    return val;
}

static int count_decimals(uint64_t value)
{
    int decimals = 0;
    while (value > 0)
        value /= 10, decimals++;

    return decimals;
}

static int get_first_valid_decimal(uint64_t value)
{
    int decimals = 0;
    while (value > 0)
    {
        uint64_t digit = value % 10;
        if (digit != 0)
            break;

        value /= 10, decimals++;
    }

    return decimals;
}

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

    double scale9 = simple_pow(10, 9);
    uint64_t scale9_int = (uint64_t)scale9;
    uint64_t scale8_int = scale9_int / 10;
    double scale9_fract = (value - integer_part) * scale9;

    uint64_t decimal_part = 0;

    uint64_t part1 = (uint64_t)scale9_fract;
    uint64_t part2 = (scale9_fract - part1) * scale9;

    uint64_t round_digit = part2 % 10;
    part2 /= 10;

    if (round_digit >= 5)
    {
        part2++;

        if (part2 >= scale8_int)
        {
            part2 -= scale8_int;

            part1++;
            if (part1 >= scale9_int)
            {
                part1 -= scale9_int;

                integer_part++;
            }
        }
    }

    decimal_part = (part1 * scale8_int) + part2;

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

    int max_decimals = 17;

    int decimals_part_count = count_decimals(decimal_part);
    int first_valid_decimal = decimals_part_count - get_first_valid_decimal(decimal_part);
    int decimals_left = max_decimals - decimals_part_count;
    int decimal_offset = value_offset + index + 1;

    bool initial_decimal_loop = true;
    bool initial_index_setup = true;

    while (decimals_part_count > -decimals_left)
    {
        if (initial_decimal_loop)
        {
            initial_decimal_loop = false;

            if (value_offset + index + 2 >= buffer_size)
                goto result;

            buffer[value_offset + index] = '.';
            index++;
        }

        char digit = '0' + (decimal_part % 10);

        if (decimals_part_count <= first_valid_decimal)
        {
            int decimal_index = decimal_offset + (decimals_part_count + decimals_left) - 1;
            if (decimal_index + 1 >= buffer_size)
                goto next_decimal;

            buffer[decimal_index] = digit;

            if (initial_index_setup)
            {
                initial_index_setup = false;
                index = (decimal_index - value_offset) + 1;
            }
        }

    next_decimal:
        decimal_part /= 10;
        decimals_part_count--;
    }

result:
    if (value_offset + index < buffer_size)
    {
        buffer[value_offset + index] = '\0';
    }
}
