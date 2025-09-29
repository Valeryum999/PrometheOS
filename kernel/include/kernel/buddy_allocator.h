#ifndef _KERNEL_BUDDY_ALLOCATOR_H
#define _KERNEL_BUDDY_ALLOCATOR_H

#include <stdint.h>
#include <stddef.h>

#define HEADER_SIZE 8
#define MIN_ALLOC_LOG2 4
#define MIN_ALLOC ((size_t)1 << MIN_ALLOC_LOG2)
#define MAX_ALLOC_LOG2 31
#define MAX_ALLOC ((size_t)1 << MAX_ALLOC_LOG2)

#define BUCKET_COUNT (MAX_ALLOC_LOG2 - MIN_ALLOC_LOG2 + 1)

typedef struct list_t {
  struct list_t *prev, *next;
} list_t;

static list_t buckets[BUCKET_COUNT];
static size_t bucket_limit = 0;
/*
 * - Move to parent:         index = (index - 1) / 2;
 * - Move to left child:     index = index * 2 + 1;
 * - Move to right child:    index = index * 2 + 2;
 * - Move to sibling:        index = ((index - 1) ^ 1) + 1;
 */
static uint8_t node_is_split[(1 << (BUCKET_COUNT - 1)) / 8];
static uint8_t *base_ptr;
static uint8_t *max_ptr;

static int update_max_ptr(uint8_t *new_value);
static void list_init(list_t *list);
static void list_push(list_t *list, list_t *entry);
static void list_remove(list_t *entry);
static list_t *list_pop(list_t *list);
static uint8_t *ptr_for_node(size_t index, size_t bucket);
static size_t node_for_ptr(uint8_t *ptr, size_t bucket);
static int parent_is_split(size_t index);
static void flip_parent_is_split(size_t index);
static size_t bucket_for_request(size_t request);
static int lower_bucket_limit(size_t bucket);
void *malloc(size_t request);
void free(void *ptr);

#endif