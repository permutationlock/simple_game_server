#include "fixed_block_array_allocator.h"
//#include <stdio.h>

void fbaa_clear(
    fixed_block_array_alloc_t* allocator
) {
    rb_clear(&(allocator->flist_rb));
    for(int i = 0; i < allocator->size; ++i) {
        allocator->full_blocks[i] = 0;
        allocator->flist_mem[
            rb_push_back(&(allocator->flist_rb))
        ] = i;
    }
}

void fbaa_init(
    fixed_block_array_alloc_t* allocator,
    int* flist_mem,
    int* fblock_mem,
    void* memory,
    int size,
    int block_size
) {
    allocator->memory = memory;
    allocator->flist_mem = flist_mem;
    allocator->full_blocks = fblock_mem;
    allocator->size = size;
    allocator->block_size = block_size;

    //printf("initializing rb\n");
    rb_init(
        &(allocator->flist_rb),
        size
    );
    for(int i = 0; i < allocator->size; ++i) {
        //printf("pushing free block %d\n", i);
        allocator->full_blocks[i] = 0;
        int bi = rb_push_back(&(allocator->flist_rb));
        //printf("at index %d\n", bi);
        (allocator->flist_mem)[
            bi
        ] = i;
    }
}

void* fbaa_malloc(fixed_block_array_alloc_t* allocator) {
    int block_index = fbaa_malloc_index(allocator); 
    if(block_index < 0) {
        return (void*)0;
    }
    return allocator->memory + block_index * allocator->block_size;
}

int fbaa_malloc_index(fixed_block_array_alloc_t* allocator) {
    if(rb_size(&allocator->flist_rb) == 0) {
        return -1;
    }
    int block_index = allocator->flist_mem[
        rb_front(&(allocator->flist_rb))
    ];
    rb_pop_front(&(allocator->flist_rb));
    allocator->full_blocks[block_index] = 1;
    return block_index;
}

void fbaa_free(
    fixed_block_array_alloc_t* allocator,
    void* mem
) {
    int block_index = ((long)(mem - allocator->memory))
        / allocator->block_size;
    fbaa_free_index(allocator, block_index);
}

void fbaa_free_index(
    fixed_block_array_alloc_t* allocator,
    int block_index
) {
    if(allocator->full_blocks[block_index] != 0) {
        allocator->flist_mem[
            rb_push_back(&(allocator->flist_rb))
        ] = block_index;
        allocator->full_blocks[block_index] = 0;
    }
}

int fbaa_available(fixed_block_array_alloc_t* allocator) {
    return rb_size(&(allocator->flist_rb));
}
