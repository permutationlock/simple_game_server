#ifndef SGS_SIMPLE_GAME_SERVER_H
#define SGS_SIMPLE_GAME_SERVER_H

#define PKEY_BYTES 16
#define MEM_SIZE

typename struct {
     unsigned int count;
     unsigned char key[PKEY_BYTES];
} player_t;

typedef struct {
    ring_fbaa_t* alloc;
    player_t* players;
} player_allocator_t;

#endif //SGS_SIMPLE_GAME_SERVER_H
