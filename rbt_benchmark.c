#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "rbt.h"
#include "fbaa.h"

#define MAX_NODES 1024

fbaa_t* rbt_allocator;
rbt_t rbt;
int lookup_index;

rbt_node_t* rbt_malloc() {
    return (rbt_node_t*) fbaa_malloc(rbt_allocator);
}

void rbt_free(rbt_node_t* node) {
    fbaa_free(rbt_allocator, (void*) node);
}

double time_fn(void (*f)(void)) {
  int nruns = 1;
  int msec = 0;
  while(msec < 1000) {
    nruns *= 2;

    clock_t start = clock(), diff;
    for(int i = 0; i < nruns; ++i) {
        f();
    }
    diff = clock() - start;

    msec = diff * 1000 / (CLOCKS_PER_SEC);
  }
  return ((double)msec * 1000 * 1000) / nruns;
}

void rbt_bench_insert_all() {
    rbt_node_t* nodes = (rbt_node_t*)rbt_allocator->memory;
    rbt_init(&rbt);
    for(int i = 0; i < MAX_NODES; ++i) {
        //printf("INSERTING NODE %d with KEY %d\n", i, nodes[i].key);
        rbt_init_node(&nodes[i], nodes[i].key, nodes[i].val);
        rbt_insert(&rbt, &nodes[i]);
    }
}

void rbt_bench_delete_all() {
    rbt_node_t* nodes = (rbt_node_t*)rbt_allocator->memory;
    for(int i = 0; i < MAX_NODES; ++i) {
        rbt_delete(&rbt, i);
    }
}

void rbt_bench_lookup() {
    rbt_find(&rbt, lookup_index);
    lookup_index = (lookup_index + 23) % MAX_NODES;
}

void rbt_bench_insert_delete() {
    rbt_node_t* node = rbt_delete(&rbt, lookup_index);
    rbt_init_node(node, lookup_index, lookup_index);
    rbt_insert(&rbt, node);
    lookup_index = (lookup_index + 23) % MAX_NODES;
}

void rbt_bench() {
    printf("RBT TEST:\n");
    rbt_allocator = fbaa_new(malloc, MAX_NODES, sizeof(rbt_node_t));
    rbt_node_t* nodes = (rbt_node_t*)rbt_allocator->memory;
    rbt_init(&rbt);
    printf("STARTING\n");
    for(int i = 0; i < MAX_NODES; ++i) {
        int j = fbaa_malloc_index(rbt_allocator);
        int key = (17 * j) % MAX_NODES;
        //printf("INITIALIZING NODE %d with KEY %d\n", j, key);
        rbt_init_node(&nodes[j], key, key);
    }
    printf(
        "Time for %d insertions: %fns\n",
        MAX_NODES,
        time_fn(rbt_bench_insert_all)
    );
    lookup_index = 23;
    printf(
        "Time for lookup: %fns\n",
        time_fn(rbt_bench_lookup)
    );
    printf(
        "Time for delete + insert: %fns\n",
        time_fn(rbt_bench_insert_delete)
    );
    //printf(
    //    "Time for %d deletions: %fms\n",
    //    MAX_NODES,
    //    time_fn(rbt_bench_delete_all)
    //);
    fbaa_destroy(free, rbt_allocator);
}

int main() {
    rbt_bench();
}
