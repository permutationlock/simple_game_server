#ifndef SGS_SIMPLE_GAME_SERVER_H
#define SGS_SIMPLE_GAME_SERVER_H

#include "cards.h"

#ifndef SGS_PLAYER_DATA_SIZE
    #define PLAYER_DATA_SIZE 256
#endif
#ifndef SGS_GAME_DATA_SIZE
    #define SGS_GAME_DATA_SIZE 1024
#endif
#ifndef SGS_GAME_MAX_PLAYERS
    #define SGS_GAME_MAX_PLAYERS 4
#endif

typedef struct {
    unsigned char data[SGS_PLAYER_DATA_SIZE];
} player_data_t;

typedef struct {
    player_t players[SGS_GAME_MAX_PLAYERS];
    unsigned char data[SGS_GAME_DATA_SIZE];
} game_data_t;

enum player_id_type_t {
    NONE,
    PC,
    NPC
};

typedef struct {
    enum player_id_type_t type;
    int local;
    int global;
    unsigned int count;
} player_id_t;

enum game_type_t {
    NONE,
    SINGLEPLAYER,
    MULTIPLAYER
};

typedef struct {
    int id;
    unsigned int count;
} game_id_t;

typedef struct {
    player_id_t id;
    game_id_t gid;
    player_data_t data;
} player_t;

typedef struct {
    game_id_t id;
    int player_count;
    player_t players[SGS_MAX_PLAYERS];
    game_data_t game_data;
} game_t;

typedef struct {
    ring_fbaa_t* account_allocator, account_data_allocator;
    ring_fbaa_t* player_allocator, player_data_allocator;
    fbaa_t* game_allocator;
    player_t* accounts, players;
    game_t* games;
} sgs_t;

#endif //SGS_SIMPLE_GAME_SERVER_H
