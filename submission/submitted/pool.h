#ifndef POOL_H
#define POOL_H

#include <stddef.h>

typedef struct pool pool_t;

pool_t *pool_create(size_t nmemb, size_t size);
void pool_destroy(pool_t *pool);

void *pool_alloc(pool_t *pool);
void pool_free(pool_t *pool, void *ptr);

#endif
