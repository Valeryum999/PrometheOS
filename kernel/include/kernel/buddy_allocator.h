#ifndef _KERNEL_BUDDY_ALLOCATOR_H
#define _KERNEL_BUDDY_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#define MIN_ALLOC_LOG2 4
#define MIN_ALLOC ((size_t)1 << MIN_ALLOC_LOG2)
#define MAX_ALLOC_LOG2 13
#define MAX_ALLOC ((size_t)1 << MAX_ALLOC_LOG2)

#define BUCKET_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1)

typedef struct list_t {
  struct list_t *prev, *next;
} list_t;

/*
 * - Move to parent:         index = (index - 1) / 2;
 * - Move to left child:     index = index * 2 + 1;
 * - Move to right child:    index = index * 2 + 2;
 * - Move to sibling:        index = ((index - 1) ^ 1) + 1;
 */

void init_free_lists();
void *malloc(size_t request);
void free(void *ptr);

#endif