#include <bitmap.h>

bool simplejs_bitmap_get_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry)
{
    uint32_t index = BITMAP_INDEX_OFFSET(entry);
    uint32_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    return bitmap[index] & mask;
}

void simplejs_bitmap_set_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry)
{
    uint32_t index = BITMAP_INDEX_OFFSET(entry);
    uint32_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    bitmap[index] |= mask;
}

void simplejs_bitmap_clear_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry)
{
    uint32_t index = BITMAP_INDEX_OFFSET(entry);
    uint32_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    bitmap[index] &= ~mask;
}

simplejs_status_t simplejs_bitmap_find_filtered_entry(uint8_t *bitmap, uint32_t entries, bool isFree, uint32_t *outEntry)
{
    simplejs_status_t status = SIMPLEJS_STATUS_ALLOCATION_ERROR;

    for (uint32_t i = 0; i < entries; i++)
    {
        bool state = !simplejs_bitmap_get_bit(bitmap, entries, i);
        if (state == isFree)
        {
            *outEntry = i;
            status = SIMPLEJS_STATUS_SUCCESS;
            goto result;
        }
    }

result:
    return status;
}
