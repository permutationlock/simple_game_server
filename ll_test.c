#include <stdio.h>
#include "linked_list.h"
#include "fixed_block_array_allocator.h"

#define MEM_SIZE 8

typedef struct ll_in_node_t {
    struct ll_in_node_t* prev;
    struct ll_in_node_t* next;
    int val;
} ll_int_node_t;

typedef struct {
    fixed_block_array_alloc_t alloc;
    int flist_mem[MEM_SIZE];
    int fblock_mem[MEM_SIZE];
    ll_int_node_t memory[MEM_SIZE];
} ll_allocator_t;

ll_allocator_t ll_allocator;

ll_int_node_t* ll_malloc() {
    return (ll_int_node_t*) fbaa_malloc(&(ll_allocator.alloc));
}

void ll_free(ll_int_node_t* node) {
    fbaa_free(&(ll_allocator.alloc), (void*) node);
}

void assert_sfb(
    linked_list_t* ll,
    int size,
    int front,
    int back
) {
    int afront, aback;
    ll_int_node_t* node;

    int asize = ll_size(ll);
    if(size != asize) {
        printf(
            "ERROR: list size not %d: %d\n",
            size,
            asize
        );
    }
    node  = (ll_int_node_t*)ll_first(ll);
    if(node != (ll_int_node_t*)0) {
        afront = node->val;
    }
    if(node == (ll_int_node_t*)0) {
        if(front != -1) {
            printf(
                "ERROR: list first was null\n"
            );
        }
    } else if(front != afront) {
        printf(
            "ERROR: list first not %d: %d\n",
            front,
            afront
        );
    }
    node  = (ll_int_node_t*)ll_last(ll);
    if(node != (ll_int_node_t*)0) {
        aback = node->val;
    }
    if(node == (ll_int_node_t*)0) {
        if(back != -1) {
            printf(
                "ERROR: list last was null\n"
            );
        }
    } else if(back != aback) {
        printf(
            "ERROR: list last not %d: %d\n",
            front,
            afront
        );
    }
}

void ll_test() {
    printf("LL TEST:\n");
    fbaa_init(
        &(ll_allocator.alloc),
        ll_allocator.flist_mem,
        ll_allocator.fblock_mem,
        ll_allocator.memory,
        MEM_SIZE,
        sizeof(ll_int_node_t)
    );
    linked_list_t list;
    printf("STEP: initializing list\n");
    ll_init(&list);
    int size = ll_size(&list);
    if(size != 0) {
        printf("ERROR: initial list size nonzero: %d\n", size);
    }
    printf("STEP: pushing 11\n");
    ll_int_node_t* node = ll_malloc();
    node->val = 11;
    ll_push_back(&list, (linked_list_node_t*)node);
    assert_sfb(&list, 1, 11, 11);
    printf("STEP: pushing 22\n");
    node = (ll_int_node_t*)ll_malloc();
    node->val = 22;
    ll_push_back(&list, (linked_list_node_t*)node);
    assert_sfb(&list, 2, 11, 22);
    printf("STEP: popping front\n");
    node = (ll_int_node_t*)ll_first(&list);
    ll_delete(&list, (linked_list_node_t*)node);
    ll_free(node);
    assert_sfb(&list, 1, 22, 22);
    printf("STEP: popping front\n");
    node = (ll_int_node_t*)ll_last(&list);
    ll_delete(&list, (linked_list_node_t*)node);
    ll_free(node);
    assert_sfb(&list, 0, -1, -1);
}

int main() {
    ll_test();
}
