#include<stdio.h>
#include<time.h>

extern void** malloc_test();
extern void free_test(void**);
extern void** fbaa_malloc_test();
extern void fbaa_free_test(void**);
extern void** ring_fbaa_malloc_test();
extern void ring_fbaa_free_test(void**);

void test_malloc() {
    void** mem = malloc_test();
    free_test(mem);
}

void test_fbaa() {
    void** mem = fbaa_malloc_test();
    fbaa_free_test(mem);
}

void test_ring_fbaa() {
    void** mem = ring_fbaa_malloc_test();
    ring_fbaa_free_test(mem);
}

void time_fn(void (*f)(void)) {
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
  printf("Time taken %.2f nanoseconds", ((double)msec * 1000 * 1000) / nruns);

  printf("\n");
}

int main() {
    printf("TESTING MALLOC:\n");
    time_fn(test_malloc);
    printf("\n\n");
    printf("TESTING FBAA_MALLOC:\n");
    time_fn(test_fbaa);
    printf("\n\n");
    printf("TESTING RING_FBAA_MALLOC:\n");
    time_fn(test_ring_fbaa);
}
