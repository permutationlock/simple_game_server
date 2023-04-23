#ifndef SGS_FIXED_BLOCK_ARRAY_ALLOC_H
#define SGS_FIXED_BLOCK_ARRAY_ALLOC_H

#include "ring_buffer.h"

typedef struct {
    unsigned int block_size, size;
    ring_buffer_t flist_rb;
    unsigned int* flist_mem;
    unsigned int* full_blocks;
    void* memory;
} fixed_block_array_alloc_t;

void fbaa_clear(
    fixed_block_array_alloc_t* allocator
);
void fbaa_init(
    fixed_block_array_alloc_t* allocator,
    unsigned int* flist_mem,
    unsigned int* fblock_mem,
    void* mem,
    unsigned int size,
    unsigned int block_size
);
void* fbaa_malloc(fixed_block_array_alloc_t* allocator);
void fbaa_free(fixed_block_array_alloc_t* allocator, void* mem);
unsigned int fbaa_available(fixed_block_array_alloc_t* allocator);

#endif // SGS_FIXED_BLOCK_ARRAY_ALLOC_H
