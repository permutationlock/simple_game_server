#include "array_rbt.h"

unsigned long arbt_mem_size(int size) {
    const int STRUCT_SIZE = align_size(
        sizeof(array_rbt_t),
        16
    );
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);

    int mem_size = STRUCT_SIZE;
    mem_size += 2 * IARRAY_SIZE;
    mem_size += size * sizeof(rbt_node_t);

    return mem_size;
}

array_rbt_t* arbt_new_from_void(void* mem, int size) {
    const int STRUCT_SIZE = align_size(
        sizeof(array_rbt_t),
        16
    );
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);

    array_rbt_t* arbt = (array_rbt_t*)(mem);
    mem += STRUCT_SIZE;
    int* free_list_mem = (int*) mem;
    mem += IARRAY_SIZE;
    int* full_block_mem = (int*) mem;
    mem += IARRAY_SIZE;

    rbt_init(&(arbt->rbt));
    fbaa_init(
        &(arbt->node_allocator),
        free_list_mem,
        full_block_mem,
        mem,
        size,
        sizeof(rbt_node_t)
    );
    return arbt;
}

array_rbt_t* arbt_new(void*(*malloc)(unsigned long), int size) {
    const int STRUCT_SIZE = align_size(
        sizeof(array_rbt_t),
        16
    );
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);

    void* mem = malloc(arbt_mem_size(size));

    if(mem == (void*)0) {
        return (array_rbt_t*)0;
    }

    return arbt_new_from_void(mem, size);
}

void arbt_destroy(void(*free)(void*), array_rbt_t* arbt) {
    free((void*)arbt);
}

int arbt_contains(array_rbt_t* arbt, long key) {
    return !(rbt_find(&(arbt->rbt), key) == (rbt_node_t*)0);
}

int arbt_get(array_rbt_t* arbt, long key) {
    return rbt_find(&(arbt->rbt), key)->val;
}

void arbt_insert(array_rbt_t* arbt, long key, int val) {
    rbt_node_t* node =
        (rbt_node_t*)fbaa_malloc(&(arbt->node_allocator));
    rbt_init_node(node, key, val);
    rbt_insert(&(arbt->rbt), node);
}

void arbt_modify(array_rbt_t* arbt, long key, int val) { 
    rbt_find(&(arbt->rbt), key)->val = val;
}

void arbt_delete(array_rbt_t* arbt, long key) {
    rbt_node_t* node = rbt_delete(&(arbt->rbt), key);
    fbaa_free(&(arbt->node_allocator), (void*)node);
}

void arbt_clear(array_rbt_t* arbt) {
    fbaa_clear(&(arbt->node_allocator));
    rbt_init(&(arbt->rbt));
}

int size(array_rbt_t* arbt) {
    return arbt->rbt.size;
}

int available(array_rbt_t* arbt) {
    return fbaa_available(&(arbt->node_allocator));
}
