#ifndef SGS_SIMPLE_GAME_SERVER_H
#define SGS_SIMPLE_GAME_SERVER_H

#define PKEY_BYTES 16

typename struct {
     unsigned int count;
     unsigned char key[PKEY_BYTES];
} player_t;

typedef struct {
    fixed_block_array_alloc_t alloc;
    unsigned int free_list[MEM_SIZE];
    unsigned int full_blocks[MEM_SIZE];
    player_t players[MEM_SIZE];
} player_allocator_t;

#endif //SGS_SIMPLE_GAME_SERVER_H
