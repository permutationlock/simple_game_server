clang -c -O3 ring_buffer.c
clang -c -O3 linked_list.c
clang -c -O3 rbt.c
clang -c -O3 fbaa.c
clang -c -O3 ring_fbaa.c
clang -c -O3 rb_malloc_bench.c
clang -c rb_benchmark.c
clang -c -O3 rbt_benchmark.c
clang rb_malloc_bench.o rb_benchmark.o ring_buffer.o ring_fbaa.o \
    linked_list.o fbaa.o -o rb_benchmark
clang rbt_benchmark.o ring_buffer.o rbt.o \
    fbaa.o -o rbt_benchmark
