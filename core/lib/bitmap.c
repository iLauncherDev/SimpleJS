#include <lib/bitmap.h>

bool simplejs_bitmap_get_bit(uint8_t *bitmap, size_t entries, size_t entry)
{
    size_t index = BITMAP_INDEX_OFFSET(entry);
    size_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    return bitmap[index] & mask;
}

void simplejs_bitmap_set_bit(uint8_t *bitmap, size_t entries, size_t entry)
{
    size_t index = BITMAP_INDEX_OFFSET(entry);
    size_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    bitmap[index] |= mask;
}

void simplejs_bitmap_clear_bit(uint8_t *bitmap, size_t entries, size_t entry)
{
    size_t index = BITMAP_INDEX_OFFSET(entry);
    size_t bit = BITMAP_BIT_OFFSET(entry);

    uint8_t mask = 1 << bit;

    SIMPLEJS_ASSERT(entries > index);

    bitmap[index] &= ~mask;
}

bool simplejs_bitmap_find_filtered_entry(uint8_t *bitmap, size_t entries, bool is_free, size_t required_entries, size_t *out_entry)
{
    SIMPLEJS_ASSERT(required_entries != 0);

    size_t free_entries = 0;

    for (size_t i = 0; i < entries; i++)
    {
        bool state = !simplejs_bitmap_get_bit(bitmap, entries, i);

        free_entries = state == is_free ? (free_entries + 1) : 0;

        if (free_entries >= required_entries)
        {
            *out_entry = i - (free_entries - 1);

            return true;
        }
    }

    return false;
}
