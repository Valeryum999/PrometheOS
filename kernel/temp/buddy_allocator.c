#include <kernel/buddy_allocator.h>

static uint8_t node_is_split[(1 << (BUCKET_COUNT - 1)) / 8];
static uint8_t *base_ptr = (uint8_t *)(0x200000);
static uint8_t *max_ptr = (uint8_t *)(0x80000000);

static int update_max_ptr(uint8_t *new_value) {
  if (new_value > max_ptr) {
    // if (brk(new_value)) {
    //   return 0;
    // }
    max_ptr = new_value;
  }
  return 1;
}

static void list_init(list_t *list) {
  list->prev = list;
  list->next = list;
}

static void list_push(list_t *list, list_t *entry) {
  list_t *prev = list->prev;
  entry->prev = prev;
  entry->next = list;
  prev->next = entry;
  list->prev = entry;
}

static void list_remove(list_t *entry) {
  list_t *prev = entry->prev;
  list_t *next = entry->next;
  prev->next = next;
  next->prev = prev;
}

static list_t *list_pop(list_t *list) {
  list_t *back = list->prev;
  if (back == list)
    return NULL;
  list_remove(back);
  return back;
}

static uint8_t *ptr_for_node(size_t index, size_t bucket) {
  return base_ptr + ((index - (1 << bucket) + 1) << (MAX_ALLOC_LOG2 - bucket));
}

static size_t node_for_ptr(uint8_t *ptr, size_t bucket) {
  return ((ptr - base_ptr) >> (MAX_ALLOC_LOG2 - bucket)) + (1 << bucket) - 1;
}

static int parent_is_split(size_t index) {
  index = (index - 1) / 2; // parent
  return (node_is_split[index / 8] >> (index % 8)) & 1;
}

static void flip_parent_is_split(size_t index) {
  index = (index - 1) / 2; // parent
  node_is_split[index / 8] ^= 1 << (index % 8);
}

static size_t bucket_for_request(size_t request) {
  size_t bucket = BUCKET_COUNT - 1;
  size_t size = MIN_ALLOC;

  // going down in buckets, doubling size each time
  while (size < request) {
    bucket--;
    size *= 2;
  }

  return bucket;
}

static int lower_bucket_limit(size_t bucket) {
  while (bucket < bucket_limit) {
    size_t root = node_for_ptr(base_ptr, bucket_limit);
    uint8_t *right_child;

    if (!parent_is_split(root)) {
      list_remove((list_t *)base_ptr);
      list_init(&buckets[--bucket_limit]);
      list_push(&buckets[bucket_limit], (list_t *)base_ptr);
      continue;
    }
    right_child = ptr_for_node(root + 1, bucket_limit);
    if (!update_max_ptr(right_child + sizeof(list_t))) {
      return 0;
    }
    list_push(&buckets[bucket_limit], (list_t *)right_child);
    list_init(&buckets[--bucket_limit]);

    root = (root - 1) / 2;
    if (root != 0) {
      flip_parent_is_split(root);
    }
  }

  return 1;
}

void *malloc(size_t request) {
  size_t original_bucket, bucket;

  if (request + HEADER_SIZE > MAX_ALLOC) {
    return NULL;
  }

  if (base_ptr == NULL) {
    // base_ptr = max_ptr = (uint8_t *)sbrk(0);
    bucket_limit = BUCKET_COUNT - 1;
    update_max_ptr(base_ptr + sizeof(list_t));
    list_init(&buckets[BUCKET_COUNT - 1]);
    list_push(&buckets[BUCKET_COUNT - 1], (list_t *)base_ptr);
  }

  bucket = bucket_for_request(request + HEADER_SIZE);
  original_bucket = bucket;

  while (bucket + 1 != 0) {
    size_t size, bytes_needed, i;
    uint8_t *ptr;

    // if (!lower_bucket_limit(bucket)) {
    //   return NULL;
    // }

    ptr = (uint8_t *)list_pop(&buckets[bucket]);
    if (!ptr) {
      if (bucket != bucket_limit || bucket == 0) {
        bucket--;
        continue;
      }

      if (!lower_bucket_limit(bucket - 1)) {
        return NULL;
      }
      ptr = (uint8_t *)list_pop(&buckets[bucket]);
    }

    // size = (size_t)1 << (MAX_ALLOC_LOG2 - bucket);
    // bytes_needed = bucket < original_bucket ? size / 2 + sizeof(list_t) :
    // size; if (!update_max_ptr(ptr + bytes_needed)) {
    //   list_push(&buckets[bucket], (list_t *)ptr);
    //   return NULL;
    // }

    i = node_for_ptr(ptr, bucket);
    if (i != 0) {
      flip_parent_is_split(i);
    }

    while (bucket < original_bucket) {
      i = i * 2 + 1;
      bucket++;
      flip_parent_is_split(i);
      list_push(&buckets[bucket], (list_t *)ptr_for_node(i + 1, bucket));
    }

    // *(size_t *)ptr = request;
    return ptr + HEADER_SIZE;
  }

  return NULL;
}

void free(void *ptr) {
  size_t bucket, i;

  if (!ptr) {
    return;
  }

  ptr = (uint8_t *)ptr - HEADER_SIZE;
  bucket = bucket_for_request(*(size_t *)ptr + HEADER_SIZE);
  i = node_for_ptr((uint8_t *)ptr, bucket);

  while (i != 0) {
    flip_parent_is_split(i);
bucket_limit) {
      break;
    }

list_remove((list_t *)ptr_for_node(((i - 1) ^ 1) + 1, bucket));
i = (i - 1) / 2;
bucket--;
  }

  list_push(&buckets[bucket], (list_t *)ptr_for_node(i, bucket));
}