#include <stdlib.h>
#include <stdio.h>
#include "ring_fbaa.h"

#define MEM_SIZE 8

typedef struct {
    int x, y;
} point_t;

ring_fbaa_t* point_allocator;

point_t* point_malloc() {
    return (point_t*)rfbaa_malloc(point_allocator);
}

void point_free(point_t* point) {
    rfbaa_free(point_allocator, (void*)point);
}

void print_mem(int* mem, int size) {
    for(int i = 0; i < size; ++i) {
        printf("\t%d\n", mem[i]);
    }
}

void ring_fbaa_test() {
    point_allocator = rfbaa_new(
        malloc,
        MEM_SIZE,
        sizeof(point_t)
    );

    printf("free list:\n");
    print_mem(point_allocator->mem_alloc.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator->mem_alloc.full_blocks, MEM_SIZE);

    printf(
        "free space: %d / %d\n",
        rfbaa_available(point_allocator),
        MEM_SIZE
    );

    point_t* p1 = point_malloc();
    p1->x = 1;
    p1->y = 13;

    point_t* p2 = point_malloc();
    p2->x = 23;
    p2->y = 2337;

    point_t* p3 = point_malloc();
    p3->x = 313;
    p3->y = 3321;

    point_t* p4 = point_malloc();
    p4->x = 413;
    p4->y = 489;

    point_t* p5 = point_malloc();
    p5->x = 513;
    p5->y = 589;

    point_t* p6 = point_malloc();
    p6->x = 613;
    p6->y = 689;

    point_t* p7 = point_malloc();
    p7->x = 713;
    p7->y = 789;

    point_t* p8 = point_malloc();
    p8->x = 813;
    p8->y = 889;
 
    point_t* p9 = point_malloc();
    printf("allocating 9th point: %p\n", p9);

    printf("free list:\n");
    print_mem(point_allocator->mem_alloc.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator->mem_alloc.full_blocks, MEM_SIZE);
    printf("memory:\n");
    print_mem((int*)point_allocator->mem_alloc.memory, MEM_SIZE * 2);
    
    printf(
        "free space: %d / %d\n",
        rfbaa_available(point_allocator),
        MEM_SIZE
    );

    point_free(p4);
    point_free(p1);
    point_free(p3);
    
    p1 = point_malloc();
    p1->x = 1337;
    p1->y = 7331;
    p3 = point_malloc();
    p3->x = 1337;
    p3->y = 7331;
    p4 = point_malloc();
    p4->x = 1337;
    p4->y = 7331;

    printf(
        "free space: %d / %d\n",
        rfbaa_available(point_allocator),
        MEM_SIZE
    );
    printf("free list:\n");
    print_mem(point_allocator->mem_alloc.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator->mem_alloc.full_blocks, MEM_SIZE);
    printf("memory:\n");
    print_mem((int*)point_allocator->mem_alloc.memory, MEM_SIZE * 2);
    
    rfbaa_destroy(free, point_allocator);
}

int main() {
    ring_fbaa_test();
}
