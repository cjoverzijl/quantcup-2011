#include <assert.h>
#include <stdlib.h>

#include "pool.h"

struct pool {
  void *first;
};

pool_t * pool_create(size_t nmemb, size_t size) {
  assert(nmemb > 0);

  // Each chunk starts with a pointer to the next free chunk.
  size += sizeof(void *);
  pool_t *pool = malloc(sizeof(pool_t) + nmemb * size + sizeof(void *));
  if (!pool)
    goto error_malloc_pool;

  // The first free chunk is at the start of the allocated pool.
  pool->first = (void *) ((char *) pool + sizeof(pool_t));

  // Create a singly linked list of free chunks, terminated by NULL.
  void **next = (void **) pool->first;
  while (--nmemb) {
    *next = (void *) ((char *) next + size);
    next = (void **) *next;
  }
  *next = NULL;

  return pool;

  error_malloc_pool: return NULL;
}

void pool_destroy(pool_t *pool) {
  free(pool);
}

void * pool_alloc(pool_t *pool) {
  assert(pool);

  // If no free chunk exists, return NULL.
  if (!pool->first)
    return NULL;

  // Pop the first chunk of the front of the free list and return the
  // address of the memory following the list pointer.
  void **first = (void **) pool->first;
  pool->first = *first++;
  return first;
}

void pool_free(pool_t *pool, void *ptr) {
  assert(pool);

  // Push the chunk to the front of the free list.
  void **first = (void **) ptr;
  *--first = pool->first;
  pool->first = first;
}
