#ifndef SGS_FBAA_H
#define SGS_FBAA_H

#include "ring_buffer.h"

int align_size(int size, int alignment);

typedef struct {
    int size, block_size;
    ring_buffer_t flist_rb;
    int* flist_mem;
    int* full_blocks;
    void* memory;
} fbaa_t;

unsigned long fbaa_mem_size(int size, int block_size);
fbaa_t* fbaa_new_from_void(void* mem, int size, int block_size);
fbaa_t* fbaa_new(
    void*(*malloc)(unsigned long),
    int size,
    int block_size
);
void fbaa_destroy(void(*free)(void*), fbaa_t* allocator);
void fbaa_clear(fbaa_t* allocator);
void fbaa_init(
    fbaa_t* allocator,
    int* flist_mem,
    int* fblock_mem,
    void* memory,
    int size,
    int block_size
);
void* fbaa_malloc(fbaa_t* allocator);
int fbaa_malloc_index(fbaa_t* allocator);
void fbaa_free(
    fbaa_t* allocator,
    void* mem
);
void fbaa_free_index(
    fbaa_t* allocator,
    int block_index
);
int fbaa_available(fbaa_t* allocator);
int fbaa_size(fbaa_t* allocator);

#endif // SGS_FBAA_H
