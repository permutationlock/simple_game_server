#include "fbaa.h"
//#include <stdio.h>

int align_size(int size, int alignment) {
    return size + (alignment - (size % alignment));
}

unsigned long fbaa_mem_size(int size, int block_size) {
    const int ALLOC_SIZE = align_size(
        sizeof(fbaa_t),
        16
    );
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);
    int mem_size = ALLOC_SIZE;
    mem_size += 2 * IARRAY_SIZE;
    mem_size += size * block_size;
    return mem_size;
}

fbaa_t* fbaa_new_from_void(void* mem, int size, int block_size) {
    const int ALLOC_SIZE = align_size(
        sizeof(fbaa_t),
        16
    );
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);
    fbaa_t* allocator =
        (fbaa_t*)(mem);
    mem += ALLOC_SIZE;
    int* free_list_mem = (int*) mem;
    mem += IARRAY_SIZE;
    int* full_block_mem = (int*) mem;
    mem += IARRAY_SIZE;

    fbaa_init(
        allocator,
        free_list_mem,
        full_block_mem,
        mem,
        size,
        block_size
    );
    return allocator;
}

fbaa_t* fbaa_new(
    void*(*malloc)(unsigned long),
    int size,
    int block_size
) {
    void* mem = malloc(fbaa_mem_size(size, block_size));

    if(mem == (void*)0) {
        return (fbaa_t*)0;
    }

    return fbaa_new_from_void(mem, size, block_size);
}

void fbaa_destroy(
    void(*free)(void*),
    fbaa_t* allocator
) {
    free((void*)allocator);
}

void fbaa_clear(
    fbaa_t* allocator
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
    fbaa_t* allocator,
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

void* fbaa_malloc(fbaa_t* allocator) {
    int block_index = fbaa_malloc_index(allocator); 
    if(block_index < 0) {
        return (void*)0;
    }
    return allocator->memory + block_index * allocator->block_size;
}

int fbaa_malloc_index(fbaa_t* allocator) {
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
    fbaa_t* allocator,
    void* mem
) {
    int block_index = ((long)(mem - allocator->memory))
        / allocator->block_size;
    fbaa_free_index(allocator, block_index);
}

void fbaa_free_index(
    fbaa_t* allocator,
    int block_index
) {
    if(allocator->full_blocks[block_index] != 0) {
        allocator->flist_mem[
            rb_push_front(&(allocator->flist_rb))
        ] = block_index;
        allocator->full_blocks[block_index] = 0;
    }
}

int fbaa_available(fbaa_t* allocator) {
    return rb_size(&(allocator->flist_rb));
}

int fbaa_size(fbaa_t* allocator) {
    return allocator->size;
}
