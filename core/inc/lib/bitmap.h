#pragma once
#include "../default.h"

#define BITMAP_INDEX_OFFSET(number) ((number) >> 3)
#define BITMAP_BIT_OFFSET(number) ((number) & (8 - 1))
#define BITMAP_CALCULATE_SIZE(size) (BITMAP_INDEX_OFFSET((size + (8 - 1))))

bool simplejs_bitmap_get_bit(uint8_t *bitmap, size_t entries, size_t entry);
void simplejs_bitmap_set_bit(uint8_t *bitmap, size_t entries, size_t entry);
void simplejs_bitmap_clear_bit(uint8_t *bitmap, size_t entries, size_t entry);
bool simplejs_bitmap_find_filtered_entry(uint8_t *bitmap, size_t entries, bool is_free, size_t required_entries, size_t *out_entry);
