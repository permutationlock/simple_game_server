clang -c ring_buffer.c
clang -c linked_list.c
clang -c fbaa.c
clang -c ring_fbaa.c
#clang -c rb_test.c
clang -c ll_test.c
clang -c fbaa_test.c
clang -c ring_fbaa_test.c
#clang rb_test.o ring_buffer.o fixed_block_array_allocator.o \
#    -o rb_test
clang ll_test.o ring_buffer.o fbaa.o \
    linked_list.o -o ll_test
clang ring_buffer.o fbaa.o fbaa_test.o \
    -o fbaa_test
clang ring_buffer.o fbaa.o linked_list.o ring_fbaa.o \
    ring_fbaa_test.o -o ring_fbaa_test
