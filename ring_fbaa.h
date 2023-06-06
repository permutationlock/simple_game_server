#ifndef SGS_RING_FBAA_H
#define SGS_RING_FBAA_H

#include "fbaa.h"
#include "linked_list.h"

typedef struct ring_fbaa_ll_node_t {
    struct ring_fbaa_ll_node_t* prev;
    struct ring_fbaa_ll_node_t* next;
    int block_index;
} ring_fbaa_ll_node_t;

typedef struct {
    fbaa_t mem_alloc, ll_alloc;
    linked_list_t ll;
    ring_fbaa_ll_node_t** ll_nodes;
} ring_fbaa_t;

unsigned long rfbaa_mem_size(int size, int block_size);
ring_fbaa_t* rfbaa_new_from_void(
    void* mem,
    int size,
    int block_size
);
ring_fbaa_t* rfbaa_new(
    void*(*malloc)(unsigned long),
    int size,
    int block_size
);
void rfbaa_destroy(void(*free)(void*), ring_fbaa_t* allocator);

void rfbaa_clear(
    ring_fbaa_t* allocator
);
void rfbaa_init(
    ring_fbaa_t* allocator,
    int* free_list_mem,
    int* full_block_mem,
    int* ll_free_list_mem,
    int* ll_full_block_mem,
    ring_fbaa_ll_node_t* ll_mem,
    ring_fbaa_ll_node_t** lu_mem,
    void* memory,
    int size,
    int block_size
);

void* rfbaa_malloc(ring_fbaa_t* allocator);
int rfbaa_malloc_index(ring_fbaa_t* allocator);

void rfbaa_free(
    ring_fbaa_t* allocator,
    void* mem
);
void rfbaa_free_index(
    ring_fbaa_t* allocator,
    int block_index
);

void* rfbaa_oldest(ring_fbaa_t* allocator);
int rfbaa_oldest_index(ring_fbaa_t* allocator);

void rfbaa_free_oldest(ring_fbaa_t* allocator);

void rfbaa_renew(ring_fbaa_t* allocator, void* mem);
void rfbaa_renew_index(ring_fbaa_t* allocator, int block_index);

int rfbaa_available(ring_fbaa_t* allocator);
int rfbaa_size(ring_fbaa_t* allocator);

#endif // SGS_RING_FBAA_H
