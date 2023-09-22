gcc -c -O3 ring_buffer.c
gcc -c -O3 linked_list.c
gcc -c -O3 rbt.c
gcc -c -O3 fbaa.c
gcc -c -O3 ring_fbaa.c
gcc -c -O3 rb_malloc_bench.c
gcc -c rb_benchmark.c
gcc -c -O3 rbt_benchmark.c
gcc rb_malloc_bench.o rb_benchmark.o ring_buffer.o ring_fbaa.o \
    linked_list.o fbaa.o -o rb_benchmark
gcc rbt_benchmark.o ring_buffer.o rbt.o \
    fbaa.o -o rbt_benchmark
