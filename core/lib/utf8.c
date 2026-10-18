#include <lib/utf8.h>

static const uint32_t b1_min = 0x00;
static const uint32_t b1_max = 0x7F;

static const uint32_t b2_min = 0x80;
static const uint32_t b2_max = 0x7FF;

static const uint32_t b3_min = 0x800;
static const uint32_t b3_max = 0xFFFF;

static const uint32_t b4_min = 0x10000;
static const uint32_t b4_max = 0x10FFFF;

static const uint8_t b1_mask = 0b01111111;
static const uint8_t b2_mask = 0b00011111;
static const uint8_t b3_mask = 0b00001111;
static const uint8_t b4_mask = 0b00000111;

static const uint8_t b1_sign_mask = 0b10000000;
static const uint8_t b2_sign_mask = 0b11100000;
static const uint8_t b3_sign_mask = 0b11110000;
static const uint8_t b4_sign_mask = 0b11111000;

static const uint8_t b1_sign = 0b00000000;
static const uint8_t b2_sign = 0b11000000;
static const uint8_t b3_sign = 0b11100000;
static const uint8_t b4_sign = 0b11110000;

static const uint8_t b1_shift = 0;
static const uint8_t b2_shift = 6;
static const uint8_t b3_shift = 6 + 6;
static const uint8_t b4_shift = 6 + 6 + 6;

static const uint8_t bcb_shift = 6;

static const uint8_t bcb_sign = 0b10000000;
static const uint8_t bcr_mask = 0b00111111;

#define IS_SURROGATE_CHAR(unicode) (unicode >= SIMPLEJS_UTF8_SURROGATE_LOW_MIN && unicode <= SIMPLEJS_UTF8_SURROGATE_HIGH_MAX)

static inline uint32_t branchless_choice(bool condition_value, uint32_t if_value, uint32_t else_value)
{
    uint32_t condition_mask = -(uint32_t)condition_value;

    return (if_value & condition_mask) | (else_value & ~condition_mask);
}

static inline uint32_t branchless_choice_else_zero(bool condition_value, uint32_t if_value)
{
    return branchless_choice(condition_value, if_value, 0);
}

static inline uint32_t fix_unicode(uint32_t unicode)
{
    uint32_t new_unicode = unicode;

    if ((new_unicode == '\0') | IS_SURROGATE_CHAR(new_unicode))
        new_unicode = SIMPLEJS_UTF8_REPLACEMENT_CHAR;

    return new_unicode;
}

bool SIMPLEJS_API simplejs_utf8_is_continuation_byte(uint8_t value)
{
    return (value & ~bcr_mask) == bcb_sign;
}

uint8_t SIMPLEJS_API simplejs_utf8_get_unicode_char_size(uint32_t unicode)
{
    unicode = fix_unicode(unicode);

    uint8_t size = 1;

    bool is_b2c = unicode >= b2_min;
    bool is_b3c = unicode >= b3_min;
    bool is_b4c = unicode >= b4_min;

    size += is_b2c;
    size += is_b3c;
    size += is_b4c;

    return size;
}

bool SIMPLEJS_API simplejs_utf8_decode_char(const char *buffer, const char *end_buffer, uint8_t *len, uint32_t *out)
{
    bool status = true;

    bool has_first_byte = buffer < end_buffer;

    uint32_t b1 = has_first_byte ? (uint8_t)*buffer++ : '\0';
    uint32_t b2 = (buffer < end_buffer & b1 != '\0') ? (uint8_t)*buffer++ : '\0';
    uint32_t b3 = (buffer < end_buffer & b2 != '\0') ? (uint8_t)*buffer++ : '\0';
    uint32_t b4 = (buffer < end_buffer & b3 != '\0') ? (uint8_t)*buffer++ : '\0';

    uint32_t current_shift = 0;
    uint32_t unicode = 0;

    bool is_b4s = (b1 & b4_sign_mask) == b4_sign;
    bool is_b3s = (b1 & b3_sign_mask) == b3_sign;
    bool is_b2s = (b1 & b2_sign_mask) == b2_sign;
    bool is_b1s = (!is_b2s & !is_b3s & !is_b4s) & ((b1 & b1_sign_mask) == b1_sign);

    bool is_b2c = (is_b4s | is_b3s | is_b2s) & simplejs_utf8_is_continuation_byte(b2);
    bool is_b3c = (is_b2c & !is_b2s) & simplejs_utf8_is_continuation_byte(b3);
    bool is_b4c = (is_b3c & !is_b3s) & simplejs_utf8_is_continuation_byte(b4);

    (*len) = has_first_byte;

    /* decode first byte start */
    current_shift += branchless_choice_else_zero(is_b4s, b4_shift);
    unicode |= branchless_choice_else_zero(is_b4s, (b1 & b4_mask) << current_shift);

    current_shift += branchless_choice_else_zero(is_b3s, b3_shift);
    unicode |= branchless_choice_else_zero(is_b3s, (b1 & b3_mask) << current_shift);

    current_shift += branchless_choice_else_zero(is_b2s, b2_shift);
    unicode |= branchless_choice_else_zero(is_b2s, (b1 & b2_mask) << current_shift);

    current_shift += branchless_choice_else_zero(is_b1s, b1_shift);
    unicode |= branchless_choice_else_zero(is_b1s, (b1 & b1_mask) << current_shift);
    /* decode first byte end */

    status &= is_b4s | is_b3s | is_b2s | is_b1s;

    current_shift -= branchless_choice_else_zero(is_b2c, bcb_shift);
    unicode |= branchless_choice_else_zero(is_b2c, (b2 & bcr_mask) << current_shift);
    (*len) += branchless_choice_else_zero(is_b2c, 1);

    status &= ((is_b4s | is_b3s | is_b2s) & is_b2c) | (is_b1s);

    current_shift -= branchless_choice_else_zero(is_b3c, bcb_shift);
    unicode |= branchless_choice_else_zero(is_b3c, (b3 & bcr_mask) << current_shift);
    (*len) += branchless_choice_else_zero(is_b3c, 1);

    status &= ((is_b4s | is_b3s | is_b2s) & is_b3c) | (is_b2s | is_b1s);

    current_shift -= branchless_choice_else_zero(is_b4c, bcb_shift);
    unicode |= branchless_choice_else_zero(is_b4c, (b4 & bcr_mask) << current_shift);
    (*len) += branchless_choice_else_zero(is_b4c, 1);

    status &= ((is_b4s | is_b3s | is_b2s) & is_b4c) | (is_b3s | is_b2s | is_b1s);

    status &= (unicode <= b4_max) & (unicode != '\0') & !IS_SURROGATE_CHAR(unicode);

    unicode = branchless_choice(status, unicode, SIMPLEJS_UTF8_REPLACEMENT_CHAR);
    *out = unicode;

    return status;
}

simplejs_status_t SIMPLEJS_API simplejs_utf8_encode_char(uint32_t unicode, char *buffer, const char *buffer_end, uint8_t *len)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uint8_t temp_buffer[4] = {0};
    uint8_t *temp_ptr = temp_buffer;

    unicode = fix_unicode(unicode);

    bool is_b1s = unicode >= b1_min & unicode <= b1_max;
    bool is_b2s = unicode >= b2_min & unicode <= b2_max;
    bool is_b3s = unicode >= b3_min & unicode <= b3_max;
    bool is_b4s = unicode >= b4_min & unicode <= b4_max;

    bool is_b2c = unicode >= b2_min;
    bool is_b3c = unicode >= b3_min;
    bool is_b4c = unicode >= b4_min;

    uint32_t current_shift = 0;

    if (unicode <= b4_max)
    {
        (*len) = 1;

        /* encode first byte start */
        current_shift += branchless_choice_else_zero(is_b4s, b4_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b4s, b4_sign | ((unicode >> current_shift) & b4_mask));

        current_shift += branchless_choice_else_zero(is_b3s, b3_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b3s, b3_sign | ((unicode >> current_shift) & b3_mask));

        current_shift += branchless_choice_else_zero(is_b2s, b2_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b2s, b2_sign | ((unicode >> current_shift) & b2_mask));

        current_shift += branchless_choice_else_zero(is_b1s, b1_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b1s, b1_sign | ((unicode >> current_shift) & b1_mask));
        /* encode first byte start */

        temp_ptr += 1;

        current_shift -= branchless_choice_else_zero(is_b2c, bcb_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b2c, bcb_sign | ((unicode >> current_shift) & bcr_mask));
        temp_ptr += branchless_choice_else_zero(is_b2c, 1);
        (*len) += branchless_choice_else_zero(is_b2c, 1);

        current_shift -= branchless_choice_else_zero(is_b3c, bcb_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b3c, bcb_sign | ((unicode >> current_shift) & bcr_mask));
        temp_ptr += branchless_choice_else_zero(is_b3c, 1);
        (*len) += branchless_choice_else_zero(is_b3c, 1);

        current_shift -= branchless_choice_else_zero(is_b4c, bcb_shift);
        (*temp_ptr) |= branchless_choice_else_zero(is_b4c, bcb_sign | ((unicode >> current_shift) & bcr_mask));
        temp_ptr += branchless_choice_else_zero(is_b4c, 1);
        (*len) += branchless_choice_else_zero(is_b4c, 1);
    }
    else
    {
        status = SIMPLEJS_STATUS_ENCODING_ERROR;
    }

    if (SIMPLEJS_SUCCESS(status))
    {
        for (uint8_t i = 0; buffer < buffer_end && i < *len; i++)
            *buffer++ = temp_buffer[i];
    }

    return status;
}
