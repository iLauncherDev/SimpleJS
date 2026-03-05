#pragma once
#include "default.h"

#define BITMAP_BITS 8
#define BITMAP_INDEX_OFFSET(number) ((number) / BITMAP_BITS)
#define BITMAP_BIT_OFFSET(number) ((number) & (BITMAP_BITS - 1))
#define BITMAP_CALCULATE_SIZE(size) (BITMAP_INDEX_OFFSET((size + (BITMAP_BITS - 1)) & ~(BITMAP_BITS - 1)))

bool simplejs_bitmap_get_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry);
void simplejs_bitmap_set_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry);
void simplejs_bitmap_clear_bit(uint8_t *bitmap, uint32_t entries, uint32_t entry);
simplejs_status_t simplejs_bitmap_find_filtered_entry(uint8_t *bitmap, uint32_t entries, bool isFree, uint32_t *outEntry);
