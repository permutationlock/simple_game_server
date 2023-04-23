#include <stdio.h>
#include "ring_buffer.h"
#include "linked_list.h"

#define MEM_SIZE 8

typedef struct {
    fixed_block_array_alloc_t alloc;
    unsigned int flist_mem[MEM_SIZE];
    unsigned int fblock_mem[MEM_SIZE];
    point_t memory[MEM_SIZE];
} point_allocator_t;

point_allocator_t point_allocator;

point_t* point_malloc() {
    return (point_t*) fbaa_malloc(&(point_allocator.alloc));
}

void point_free(point_t* point) {
    fbaa_free(&(point_allocator.alloc), (void*) point);
}

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
            asize
        );
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
}

typedef struct {
    fixed_block_array_alloc_t alloc;
    unsigned int flist_mem[MEM_SIZE];
    unsigned int fblock_mem[MEM_SIZE];
    point_t memory[MEM_SIZE];
} point_allocator_t;

point_allocator_t point_allocator;

point_t* point_malloc() {
    return (point_t*) fbaa_malloc(&(point_allocator.alloc));
}

void point_free(point_t* point) {
    fbaa_free(&(point_allocator.alloc), (void*) point);
}

void print_mem(unsigned int* mem, unsigned int size) {
    for(int i = 0; i < size; ++i) {
        printf("\t%d\n", mem[i]);
    }
}

int main() {
    irb_test();
    fbaa_test();
}
