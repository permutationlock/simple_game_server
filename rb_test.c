#include <stdio.h>
#include "ring_buffer.h"
#include "fixed_block_array_allocator.h"

#define MEM_SIZE 8

/*
void assert_sfb(
    int_ring_buffer_t* buffer,
    unsigned int size,
    unsigned int front,
    unsigned int back
) {
    unsigned int asize = irb_size(buffer);
    if(size != asize) {
        printf(
            "ERROR: buffer size not %d: %d\n",
            size,
            asize);
    }
    unsigned int afront = irb_front(buffer);
    if(front != afront) {
        printf(
            "ERROR: buffer front not %d: %d\n",
            front,
            afront
        );
    }
    unsigned int aback = irb_back(buffer);
    if(back != aback) {
        printf(
            "ERROR: buffer back not %d: %d\n",
            back,
            aback
        );
    }
}

void irb_test() {
    printf("IRB TEST:\n");
    unsigned int buffer_mem[MEM_SIZE];
    int_ring_buffer_t buffer;
    irb_init(&buffer, buffer_mem, MEM_SIZE);
    printf("STEP: initializing irb\n");
    unsigned int size = irb_size(&buffer);
    if(size != 0) {
        printf("ERROR: initial buffer size nonzero: %d\n", size);
    }
    printf("STEP: pushing 11\n");
    irb_push_back(&buffer, 11);
    assert_sfb(&buffer, 1, 11, 11);
    printf("STEP: pushing 22\n");
    irb_push_back(&buffer, 22);
    assert_sfb(&buffer, 2, 11, 22);
    printf("STEP: popping front\n");
    irb_pop_front(&buffer);
    assert_sfb(&buffer, 1, 22, 22);
    printf("STEP: pushing 33\n");
    irb_push_back(&buffer, 33);
    assert_sfb(&buffer, 2, 22, 33);
    printf("STEP: pushing 44\n");
    irb_push_back(&buffer, 44);
    assert_sfb(&buffer, 3, 22, 44);
} */

typedef struct {
    int x, y;
} point_t;

typedef struct {
    fixed_block_array_alloc_t alloc;
    int flist_mem[MEM_SIZE];
    int fblock_mem[MEM_SIZE];
    point_t memory[MEM_SIZE];
} point_allocator_t;

point_allocator_t point_allocator;

point_t* point_malloc() {
    int block_index = fbaa_malloc(&(point_allocator.alloc));
    if(block_index < 0) {
        return NULL;
    }
    printf("allocating block %d\n", block_index);
    return &(point_allocator.memory[block_index]);
}

void point_free(point_t* point) {
    int block_index = (int)((
            (unsigned long)point
            - (unsigned long)point_allocator.memory
        ) / sizeof(point_t));
    printf("freeing block %d\n", block_index);
    fbaa_free(&(point_allocator.alloc), block_index);
}

void print_mem(int* mem, int size) {
    for(int i = 0; i < size; ++i) {
        printf("\t%d\n", mem[i]);
    }
}

void fbaa_test() {
    fbaa_init(
        &(point_allocator.alloc),
        point_allocator.flist_mem,
        point_allocator.fblock_mem,
        MEM_SIZE
    );

    printf("free list:\n");
    print_mem(point_allocator.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator.fblock_mem, MEM_SIZE);

    printf(
        "free space: %d / %d\n",
        fbaa_available(&(point_allocator.alloc)),
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
    print_mem(point_allocator.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator.fblock_mem, MEM_SIZE);
    printf("memory:\n");
    print_mem((int*)point_allocator.memory, MEM_SIZE * 2);
    
    printf(
        "free space: %d / %d\n",
        fbaa_available(&(point_allocator.alloc)),
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
        fbaa_available(&(point_allocator.alloc)),
        MEM_SIZE
    );
    printf("free list:\n");
    print_mem(point_allocator.flist_mem, MEM_SIZE);
    printf("full blocks:\n");
    print_mem(point_allocator.fblock_mem, MEM_SIZE);
    printf("memory:\n");
    print_mem((int*)point_allocator.memory, MEM_SIZE * 2);
}

int main() {
//    irb_test();
    fbaa_test();
}
