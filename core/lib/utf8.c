#include <lib/utf8.h>

simplejs_status_t SIMPLEJS_API simplejs_encode_utf8_char(uint32_t unicode, uint8_t *len, char *bytes)
{
    simplejs_status_t status = SIMPLEJS_STATUS_SUCCESS;

    uint8_t b1 = 0;
    uint8_t b2 = 0;
    uint8_t b3 = 0;
    uint8_t b4 = 0;

    *len = 0;

    if (unicode <= 0x7F)
    {
        *len = 1;
        b1 = (uint8_t)unicode;
    }
    else if (unicode <= 0x7FF)
    {
        *len = 2;
        b1 = (uint8_t)(0xC0 | (unicode >> 6));
        b2 = (uint8_t)(0x80 | (unicode & 0x3F));
    }
    else if (unicode <= 0xFFFF)
    {
        *len = 3;
        b1 = (uint8_t)(0xE0 | (unicode >> 12));
        b2 = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
        b3 = (uint8_t)(0x80 | (unicode & 0x3F));
    }
    else if (unicode <= 0x10FFFF)
    {
        *len = 4;
        b1 = (uint8_t)(0xF0 | (unicode >> 18));
        b2 = (uint8_t)(0x80 | ((unicode >> 12) & 0x3F));
        b3 = (uint8_t)(0x80 | ((unicode >> 6) & 0x3F));
        b4 = (uint8_t)(0x80 | (unicode & 0x3F));
    }
    else
    {
        status = SIMPLEJS_STATUS_ENCODING_ERROR;
    }

    uint8_t buf[4] = {
        b1,
        b2,
        b3,
        b4,
    };

    if (SIMPLEJS_SUCCESS(status))
    {
        for (uint8_t i = 0; i < *len; i++)
            *bytes++ = buf[i];
    }

    return status;
}
