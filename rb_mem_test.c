#include <stdlib.h>

#include "ring_buffer.h"
#include "fixed_block_array_allocator.h"
#include "ring_fbaa.h"

#define BLOCKS 1000
#define DATA_SIZE 256

typedef struct {
    int x, y;
    int data[DATA_SIZE];
} point_t;

point_t* points[BLOCKS];

void** malloc_test() {
    for(int i = 0; i < BLOCKS; ++i) {
        points[i] = (point_t*)malloc(sizeof(point_t));
        points[i]->x = i;
        points[i]->y = 3 * i;
        points[i]->data[i % DATA_SIZE] = 7 * i;
    }
    for(int i = 0; i < BLOCKS / 2; ++i) {
        points[i]->x += points[2*i]->y;
        points[i]->y -= points[2*i]->x;
    }
    return (void**) points;
}

void free_test(void** mem) {
    for(int i = 0; i < BLOCKS; ++i) {
        free(mem[i]);
    }
}

typedef struct {
    fixed_block_array_alloc_t alloc;
    int flist_mem[BLOCKS];
    int fblock_mem[BLOCKS];
    point_t memory[BLOCKS];
} point_allocator_t;

point_allocator_t point_allocator;

point_t* point_malloc() {
    return (point_t*)fbaa_malloc(&(point_allocator.alloc));
}

void point_free(point_t* mem) {
    fbaa_free(&(point_allocator.alloc), (void*)mem);
}

void** fbaa_malloc_test() {
    fbaa_init(
        &(point_allocator.alloc),
        (void*) point_allocator.flist_mem,
        point_allocator.fblock_mem,
        (void*) point_allocator.memory,
        BLOCKS,
        sizeof(point_t)
    );
    /*for(int i = 0; i < BLOCKS; ++i) {
        points[i] = &(point_allocator.memory[i]);
    }*/

    for(int i = 0; i < BLOCKS; ++i) {
        points[i] = point_malloc();
        points[i]->x = i;
        points[i]->y = 3 * i;
        points[i]->data[i % DATA_SIZE] = 7 * i;
    }
    for(int i = 0; i < BLOCKS / 2; ++i) {
        points[i]->x += points[2*i]->y;
        points[i]->y -= points[2*i]->x;
    }
    return (void**) points;
}

void fbaa_free_test(void** mem) {
    for(int i = 0; i < BLOCKS; ++i) {
        point_free((point_t*)(mem[i]));
    }
}

typedef struct {
    ring_fbaa_t alloc;
    int free_list_mem[BLOCKS];
    int full_block_mem[BLOCKS];
    point_t memory[BLOCKS];
    int ll_free_list_mem[BLOCKS];
    int ll_full_block_mem[BLOCKS];
    ring_fbaa_ll_node_t ll_mem[BLOCKS];
    ring_fbaa_ll_node_t* lu_mem[BLOCKS];
} point_ring_allocator_t;

point_ring_allocator_t ring_allocator;

point_t* point_ring_malloc() {
    return (point_t*)rfbaa_malloc(&(ring_allocator.alloc));
}

void point_ring_free(point_t* mem) {
    rfbaa_free(&(ring_allocator.alloc), (void*)mem);
}

void** ring_fbaa_malloc_test() {
    rfbaa_init(
        &(ring_allocator.alloc),
        ring_allocator.free_list_mem,
        ring_allocator.full_block_mem,
        ring_allocator.ll_free_list_mem,
        ring_allocator.ll_full_block_mem,
        ring_allocator.ll_mem,
        ring_allocator.lu_mem,
        (void*) ring_allocator.memory,
        BLOCKS,
        sizeof(point_t)
    );

    for(int i = 0; i < BLOCKS; ++i) {
        points[i] = point_ring_malloc();
        points[i]->x = i;
        points[i]->y = 3 * i;
        points[i]->data[i % DATA_SIZE] = 7 * i;
    }
    for(int i = 0; i < BLOCKS / 2; ++i) {
        points[i]->x += points[2*i]->y;
        points[i]->y -= points[2*i]->x;
    }
    return (void**) points;
}

void ring_fbaa_free_test(void** mem) {
    for(int i = 0; i < BLOCKS; ++i) {
        point_ring_free((point_t*)(mem[i]));
    }
}
