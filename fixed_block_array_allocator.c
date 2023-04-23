#include "fixed_block_array_allocator.h"
#include <stdio.h>

void fbaa_clear(
    fixed_block_array_alloc_t* allocator
) {
    rb_clear(&(allocator->flist_rb));
    for(unsigned int i = 0; i < allocator->size; ++i) {
        allocator->full_blocks[i] = 0;
        allocator->flist_mem[
            rb_push_back(&(allocator->flist_rb))
        ] = i;
    }
}

void fbaa_init(
    fixed_block_array_alloc_t* allocator,
    unsigned int* flist_mem,
    unsigned int* fblock_mem,
    void* mem,
    unsigned int size,
    unsigned int block_size
) {
    allocator->memory = mem;
    allocator->flist_mem = flist_mem;
    allocator->full_blocks = fblock_mem;
    allocator->block_size = block_size;
    allocator->size = size;

    printf("initializing rb\n");
    rb_init(
        &(allocator->flist_rb),
        size
    );
    for(unsigned int i = 0; i < allocator->size; ++i) {
        printf("pushing free block %d\n", i);
        allocator->full_blocks[i] = 0;
        unsigned int bi = rb_push_back(&(allocator->flist_rb));
        printf("at index %d\n", bi);
        (allocator->flist_mem)[
            bi
        ] = i;
    }
}

void* fbaa_malloc(fixed_block_array_alloc_t* allocator) {
    if(rb_size(&(allocator->flist_rb)) == 0) {
        return (void*)0;
    }

    unsigned int block_index = allocator->flist_mem[
        rb_front(&(allocator->flist_rb))
    ];
    printf("allocating block %d\n", block_index);
    rb_pop_front(&(allocator->flist_rb));
    allocator->full_blocks[block_index] = 1;
    return allocator->memory + block_index * allocator->block_size;
}

void fbaa_free(fixed_block_array_alloc_t* allocator, void* mem) {
    unsigned int block_index =
        ((unsigned int)(mem - allocator->memory))
        / allocator->block_size;
    printf("freeing block %d\n", block_index);
    if(allocator->full_blocks[block_index] != 0) {
        allocator->flist_mem[
            rb_push_back(&(allocator->flist_rb))
        ] = block_index;
        allocator->full_blocks[block_index] = 0;
    }
}

unsigned int fbaa_available(fixed_block_array_alloc_t* allocator) {
    return rb_size(&(allocator->flist_rb));
}
