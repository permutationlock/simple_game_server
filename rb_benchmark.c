#include<stdio.h>
#include<stdlib.h>
#include<time.h>

#include "ring_fbaa.h"

#define BLOCKS 1024

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

void allocate_blocks(
    void*(*malloc)(),
    void** blocks,
    int size
);

void free_blocks(
    void(*free)(void* p),
    void** blocks,
    int size
);

fbaa_t* fbaa;
ring_fbaa_t* rfbaa;

void* malloc_4B() {
    return malloc(4);
}

void* malloc_64B() {
    return malloc(64);
}

void* malloc_512B() {
    return malloc(512);
}

void* malloc_1024B() {
    return malloc(1024);
}

void* fbaa_tmalloc() {
    return fbaa_malloc(fbaa);
}

void fbaa_tfree(void* p) {
    return fbaa_free(fbaa, p);
}

void* rfbaa_tmalloc() {
    return rfbaa_malloc(rfbaa);
}

void rfbaa_tfree(void* p) {
    return rfbaa_free(rfbaa, p);
}

void test_malloc_4B() {
    void* blocks[BLOCKS];
    allocate_blocks(malloc_4B, blocks, BLOCKS);
    free_blocks(free, blocks, BLOCKS);
}

void test_malloc_64B() {
    void* blocks[BLOCKS];
    allocate_blocks(malloc_64B, blocks, BLOCKS);
    free_blocks(free, blocks, BLOCKS);
}

void test_malloc_512B() {
    void* blocks[BLOCKS];
    allocate_blocks(malloc_512B, blocks, BLOCKS);
    free_blocks(free, blocks, BLOCKS);
}

void test_malloc_1024B() {
    void* blocks[BLOCKS];
    allocate_blocks(malloc_1024B, blocks, BLOCKS);
    free_blocks(free, blocks, BLOCKS);
}

void test_fbaa() {
    void* blocks[BLOCKS];
    allocate_blocks(fbaa_tmalloc, blocks, BLOCKS);
    free_blocks(fbaa_tfree, blocks, BLOCKS);
}

void test_rfbaa() {
    void* blocks[BLOCKS];
    allocate_blocks(rfbaa_tmalloc, blocks, BLOCKS);
    free_blocks(rfbaa_tfree, blocks, BLOCKS);
}

void time_malloc() {
    double times[4];
    times[0] = time_fn(test_malloc_4B);
    times[1] = time_fn(test_malloc_64B);
    times[2] = time_fn(test_malloc_512B);
    times[3] = time_fn(test_malloc_1024B);
    printf(
        "std_malloc\t%f\t%f\t%f\t%f\n",
        times[0], times[1], times[2], times[3]
    );
}

void time_fbaa() {
    double times[4];
    fbaa = fbaa_new(malloc, BLOCKS, 4);
    times[0] = time_fn(test_fbaa);
    fbaa_destroy(free, fbaa);
    fbaa = fbaa_new(malloc, BLOCKS, 64);
    times[1] = time_fn(test_fbaa);
    fbaa_destroy(free, fbaa);
    fbaa = fbaa_new(malloc, BLOCKS, 512);
    times[2] = time_fn(test_fbaa);
    fbaa_destroy(free, fbaa);
    fbaa = fbaa_new(malloc, BLOCKS, 1024);
    times[3] = time_fn(test_fbaa);
    fbaa_destroy(free, fbaa);
    printf(
        "fbaa_malloc\t%f\t%f\t%f\t%f\n",
        times[0], times[1], times[2], times[3]
    );
}

void time_rfbaa() {
    double times[4];
    rfbaa = rfbaa_new(malloc, BLOCKS, 4);
    times[0] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS, 64);
    times[1] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS, 512);
    times[2] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS, 1024);
    times[3] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    printf(
        "rfbaa_malloc\t%f\t%f\t%f\t%f\n",
        times[0], times[1], times[2], times[3]
    );
}

void time_rfbaa_replacement() {
    double times[4];
    rfbaa = rfbaa_new(malloc, BLOCKS / 8, 4);
    times[0] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS / 8, 64);
    times[1] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS / 8, 512);
    times[2] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    rfbaa = rfbaa_new(malloc, BLOCKS / 8, 1024);
    times[3] = time_fn(test_rfbaa);
    rfbaa_destroy(free, rfbaa);
    printf(
        "rfbaa_malloc2\t%f\t%f\t%f\t%f\n",
        times[0], times[1], times[2], times[3]
    );
}

int main() {
    time_malloc();
    time_fbaa();
    time_rfbaa();
    time_rfbaa_replacement();
}
