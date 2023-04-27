#ifndef SGS_FIXED_BLOCK_ARRAY_ALLOC_H
#define SGS_FIXED_BLOCK_ARRAY_ALLOC_H

#include "ring_buffer.h"

typedef struct {
    int size, block_size;
    ring_buffer_t flist_rb;
    int* flist_mem;
    int* full_blocks;
    void* memory;
} fixed_block_array_alloc_t;

void fbaa_clear(
    fixed_block_array_alloc_t* allocator
);
void fbaa_init(
    fixed_block_array_alloc_t* allocator,
    int* flist_mem,
    int* fblock_mem,
    void* memory,
    int size,
    int block_size
);
void* fbaa_malloc(fixed_block_array_alloc_t* allocator);
int fbaa_malloc_index(fixed_block_array_alloc_t* allocator);
void fbaa_free(
    fixed_block_array_alloc_t* allocator,
    void* mem
);
void fbaa_free_index(
    fixed_block_array_alloc_t* allocator,
    int block_index
);
int fbaa_available(fixed_block_array_alloc_t* allocator);

#endif // SGS_FIXED_BLOCK_ARRAY_ALLOC_H
