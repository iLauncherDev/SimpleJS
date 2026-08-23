#pragma once
#include "../default.h"

typedef struct simplejs_pool simplejs_pool_t;

void SIMPLEJS_API simplejs_destroy_pool(simplejs_pool_t *pool);
simplejs_status_t SIMPLEJS_API simplejs_create_pool(size_t block_size, simplejs_pool_t **out);

pvoid SIMPLEJS_API simplejs_pool_malloc(simplejs_pool_t *pool, size_t size);
void SIMPLEJS_API simplejs_pool_mfree(simplejs_pool_t *pool, void *ptr);
