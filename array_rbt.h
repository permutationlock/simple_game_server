#ifndef SGS_ARRAY_RBT_H
#define SGS_ARRAY_RBT_H

#include "rbt.h"
#include "fbaa.h"

typedef struct {
    rbt_t rbt;
    fbaa_t node_allocator;
} array_rbt_t;

unsigned long arbt_mem_size(int size);
array_rbt_t* arbt_new_from_void(void* mem, int size);
array_rbt_t* arbt_new(void*(*malloc)(unsigned long), int size);
void arbt_destroy(void(*free)(void*), array_rbt_t* arbt);
int arbt_contains(array_rbt_t* arbt, long key);
int arbt_get(array_rbt_t* arbt, long key);
void arbt_insert(
    array_rbt_t* arbt, long key, int val
);
void arbt_modify(
    array_rbt_t* arbt, long key, int val
);
void arbt_delete(array_rbt_t* arbt, long key);
int arbt_size(array_rbt_t* arbt);
int arbt_available(array_rbt_t* arbt);

#endif // SGS_RBT_H
