clang -c -O3 ring_buffer.c
clang -c -O3 linked_list.c
clang -c -O3 rbt.c
clang -c -O3 fbaa.c
clang -c -O3 ring_fbaa.c
clang -c -O3 include/monocypher.c
clang -c -O3 -Iinclude/ simple_game_server.c
clang ring_buffer.o linked_list.o rbt.o fbaa.o ring_Fbaa.o \
    monocypher.o simple_game_server.o -o simple_game_server
