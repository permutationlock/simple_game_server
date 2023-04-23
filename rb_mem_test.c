#include <stdlib.h>

#include "ring_buffer.h"
#include "fixed_block_array_allocator.h"

#define BLOCKS 1000

typedef struct {
    int x, y;
    int blocks[64];
} point_t;

point_t* points[BLOCKS];

void** malloc_test() {
    for(int i = 0; i < BLOCKS; ++i) {
        points[i] = (point_t*)malloc(sizeof(point_t));
        points[i]->x = i;
        points[i]->y = 3 * i;
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
    unsigned int flist_mem[BLOCKS];
    unsigned int fblock_mem[BLOCKS];
    point_t memory[BLOCKS];
} point_allocator_t;

point_allocator_t point_allocator;

point_t* point_malloc() {
    return (point_t*) fbaa_malloc(&(point_allocator.alloc));
}

void point_free(point_t* point) {
    fbaa_free(&(point_allocator.alloc), (void*) point);
}

void** fbaa_malloc_test() {
    fbaa_init(
        &(point_allocator.alloc),
        (void*) point_allocator.flist_mem,
        point_allocator.fblock_mem,
        point_allocator.memory,
        BLOCKS,
        sizeof(point_t)
    );

    for(int i = 0; i < BLOCKS; ++i) {
        points[i] = (point_t*)point_malloc();
        points[i]->x = i;
        points[i]->y = 3 * i;
    }
    for(int i = 0; i < BLOCKS / 2; ++i) {
        points[i]->x += points[2*i]->y;
        points[i]->y -= points[2*i]->x;
    }
    
    return (void**) points;
}

void fbaa_free_test(void** mem) {
    for(int i = 0; i < BLOCKS; ++i) {
        point_free((point_t*)mem[i]);
    }
}
