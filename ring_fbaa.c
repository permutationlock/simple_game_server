#include "ring_fbaa.h"
//#include <stdio.h>

unsigned long rfbaa_mem_size(int size, int block_size) {
    const int ALLOC_SIZE = align_size(sizeof(ring_fbaa_t), 16);
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);
    const int NARRAY_SIZE = align_size(
        size * sizeof(ring_fbaa_ll_node_t),
        16
    );
    const int PARRAY_SIZE = align_size(
        size * sizeof(ring_fbaa_ll_node_t*),
        16
    );

    int mem_size = ALLOC_SIZE;
    mem_size += 4 * IARRAY_SIZE;
    mem_size += PARRAY_SIZE;
    mem_size += NARRAY_SIZE;
    mem_size += size * block_size;

    return mem_size;
}

ring_fbaa_t* rfbaa_new_from_void(
    void* mem,
    int size,
    int block_size
) {
    const int ALLOC_SIZE = align_size(sizeof(ring_fbaa_t), 16);
    const int IARRAY_SIZE = align_size(size * sizeof(int), 16);
    const int NARRAY_SIZE = align_size(
        size * sizeof(ring_fbaa_ll_node_t),
        16
    );
    const int PARRAY_SIZE = align_size(
        size * sizeof(ring_fbaa_ll_node_t*),
        16
    );

    ring_fbaa_t* allocator = (ring_fbaa_t*)(mem);
    mem += ALLOC_SIZE;
    int* free_list_mem = (int*) mem;
    mem += IARRAY_SIZE;
    int* full_block_mem = (int*) mem;
    mem += IARRAY_SIZE;
    int* ll_free_list_mem = (int*) mem;
    mem += IARRAY_SIZE;
    int* ll_full_block_mem = (int*) mem;
    mem += IARRAY_SIZE;

    ring_fbaa_ll_node_t* ll_mem = (ring_fbaa_ll_node_t*)mem;
    mem += NARRAY_SIZE;

    ring_fbaa_ll_node_t** lu_mem = (ring_fbaa_ll_node_t**)mem;
    mem += PARRAY_SIZE;

    rfbaa_init(
        allocator,
        free_list_mem,
        full_block_mem,
        ll_free_list_mem,
        ll_full_block_mem,
        ll_mem,
        lu_mem,
        mem,
        size,
        block_size
    );
    return allocator;
}

ring_fbaa_t* rfbaa_new(
    void*(*malloc)(unsigned long),
    int size,
    int block_size
) {
    void* mem = malloc(rfbaa_mem_size(size, block_size));

    if(mem == (void*)0) {
        return (ring_fbaa_t*)0;
    }

    return rfbaa_new_from_void(mem, size, block_size);
}

void rfbaa_destroy(void(*free)(void*), ring_fbaa_t* allocator) {
    free((void*)allocator);
}

void rfbaa_clear(
    ring_fbaa_t* allocator
) {
    fbaa_clear((&allocator->mem_alloc));
    fbaa_clear((&allocator->ll_alloc));
    ll_init(&(allocator->ll));
}

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
) {
    fbaa_init(
        &(allocator->mem_alloc),
        free_list_mem,
        full_block_mem,
        memory,
        size,
        block_size
    );
    fbaa_init(
        &(allocator->ll_alloc),
        ll_free_list_mem,
        ll_full_block_mem,
        ll_mem,
        size,
        sizeof(ring_fbaa_ll_node_t)
    );
    ll_init(&(allocator->ll));
    allocator->ll_nodes = lu_mem;
}

void* rfbaa_malloc(ring_fbaa_t* allocator) {
    int block_index = rfbaa_malloc_index(allocator);
    if(block_index == -1) {
        return (void*)0;
    }
    return allocator->mem_alloc.memory
        + block_index * allocator->mem_alloc.block_size;
}

int rfbaa_malloc_index(ring_fbaa_t* allocator) {
    int block_index = fbaa_malloc_index(&(allocator->mem_alloc));
    if(block_index == -1) {
        return -1;
    }

    ring_fbaa_ll_node_t* n = (ring_fbaa_ll_node_t*)fbaa_malloc(
        &(allocator->ll_alloc)
    );
    n->block_index = block_index;
    ll_push_back(&(allocator->ll), (ll_node_t*)n);

    allocator->ll_nodes[block_index] = n;

    return block_index;
}

void rfbaa_free(ring_fbaa_t* allocator, void* mem) {
    int block_index = ((long)(mem - allocator->mem_alloc.memory))
        / allocator->mem_alloc.block_size;
    rfbaa_free_index(allocator, block_index);
}

void rfbaa_free_index(ring_fbaa_t* allocator, int block_index) {
    if(allocator->mem_alloc.full_blocks[block_index] == 0) {
        return;
    }

    ring_fbaa_ll_node_t* n = allocator->ll_nodes[block_index];
    ll_delete(&(allocator->ll), (ll_node_t*)n);
    fbaa_free(&(allocator->ll_alloc), (void*)n);

    fbaa_free_index(&(allocator->mem_alloc), block_index);
}

void* rfbaa_oldest(ring_fbaa_t* allocator) {
    ring_fbaa_ll_node_t* n = (ring_fbaa_ll_node_t*)ll_first(
        &(allocator->ll)
    );
    if(n == (ring_fbaa_ll_node_t*)0) {
        return (void*)0;
    }
    return allocator->mem_alloc.memory
        + n->block_index * allocator->mem_alloc.block_size;
}

int rfbaa_oldest_index(ring_fbaa_t* allocator) {
    ring_fbaa_ll_node_t* n = (ring_fbaa_ll_node_t*)ll_first(
        &(allocator->ll)
    );
    if(n == (ring_fbaa_ll_node_t*)0) {
        return -1;
    }
    return n->block_index;
}

void rfbaa_free_oldest(ring_fbaa_t* allocator) {
    ring_fbaa_ll_node_t* n = (ring_fbaa_ll_node_t*)ll_first(
        &(allocator->ll)
    );
    if(n == (ring_fbaa_ll_node_t*)0) {
        return;
    }
    fbaa_free_index(&(allocator->mem_alloc), n->block_index);
    ll_delete(&(allocator->ll), (ll_node_t*)n);
    fbaa_free(&(allocator->ll_alloc), (void*)n);
}

void rfbaa_renew(ring_fbaa_t* allocator, void* mem) {
    int block_index = ((long)(mem - allocator->mem_alloc.memory))
        / allocator->mem_alloc.block_size;
    rfbaa_renew_index(allocator, block_index);
}

void rfbaa_renew_index(ring_fbaa_t* allocator, int block_index) {
    if(allocator->mem_alloc.full_blocks[block_index] == 0) {
        return;
    }
    ring_fbaa_ll_node_t* n = allocator->ll_nodes[block_index];
    ll_delete(&(allocator->ll), (ll_node_t*)n);
    ll_push_back(&(allocator->ll), (ll_node_t*)n);
}

int rfbaa_available(ring_fbaa_t* allocator) {
    return fbaa_available(&(allocator->mem_alloc));
}

int rfbaa_size(ring_fbaa_t* allocator) {
    return fbaa_size(&(allocator->mem_alloc));
}
