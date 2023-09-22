#ifndef SGS_SIMPLE_GAME_SERVER_H
#define SGS_SIMPLE_GAME_SERVER_H

#include "ring_fbaa.h"
#include "array_rbt.h"

#include <stddef.h>
#include <netinet/in.h> // needed for sockaddr_in and htonl functions
#include <sys/epoll.h> // needed for poll syscall and struct pollfd

#ifndef SGS_PLAYER_DATA_SIZE
    #ifndef SGS_PLAYER_DATA_T
        #define SGS_PLAYER_DATA_SIZE 256
    #else
        #define SGS_PLAYER_DATA_SIZE sizeof(SGS_PLAYER_DATA_T)
    #endif
#endif
#ifndef SGS_PLAYER_DATA_T
    #define SGS_PLAYER_DATA_T sgs_player_data_t
    typedef struct {
        uint8_t data[SGS_PLAYER_DATA_SIZE];
    } sgs_player_data_t;
#endif
#ifndef SGS_GAME_DATA_SIZE
    #ifndef SGS_GAME_DATA_T
        #define SGS_GAME_DATA_SIZE 1024
    #else
        #define SGS_GAME_DATA_SIZE sizeof(SGS_GAME_DATA_T)
    #endif
#endif
#ifndef SGS_GAME_DATA_T
    #define SGS_GAME_DATA_T sgs_game_data_t
    typedef struct {
        uint8_t data[SGS_GAME_DATA_SIZE];
    } sgs_game_data_t;
#endif
#ifndef SGS_GAME_MAX_PLAYERS
    #define SGS_GAME_MAX_PLAYERS 4
#endif
#ifndef SGS_GAME_MESSAGE_SIZE
    #ifndef SGS_GAME_MESSAGE_T
        #define SGS_GAME_MESSAGE_SIZE 16
    #else
        #define SGS_GAME_MESSAGE_SIZE sizeof(SGS_GAME_MESSAGE_T)
    #endif
#endif
#ifndef SGS_GAME_MESSAGE_T
    #define SGS_GAME_MESSAGE_T sgs_game_message_t
    typedef struct {
        uint8_t data[SGS_GAME_DATA_SIZE];
    } sgs_game_message_t;
#endif

enum sgs_message_enctype_t {
    SGS_MET_PLAINTEXT,
    SGS_MET_ENCRYPTED
};

enum sgs_message_type_t {
    SGS_MT_PING,
    SGS_MT_SUCCESS,
    SGS_MT_FAILURE,
    SGS_MT_PUBKEY,
    SGS_MT_VERIFY,
    SGS_MT_CREATE,
    SGS_MT_LOGIN,
    SGS_MT_RESUME,
    SGS_MT_LOGOUT,
    SGS_MT_GAME
};

typedef struct {
    uint8_t enctype;
    uint8_t type;
    int length;
} sgs_msg_info_t;

typedef struct {
    sgs_msg_info_t info;
    uint8_t message[];
} sgs_msg_t;

#define SGS_MESSAGE_T(SIZE) struct {\
    sgs_msg_info_t info;\
    uint8_t message[SIZE];\
}

#define SGS_MESSAGE_SIZE(SIZE) (SIZE + sizeof(sgs_message_info_t))

typedef struct {
    uint8_t nonce[24];
    uint8_t mac[16];
} sgs_msg_encdata_t;

typedef struct {
    sgs_msg_info_t info;
    sgs_msg_encdata_t encdata;
    uint8_t message[];
} sgs_sec_msg_t;

#define SGS_SEC_MESSAGE_T(SIZE) struct {\
    sgs_msg_info_t info;\
    sgs_msg_encdata_t encdata;\
    uint8_t message[SIZE];\
}

#define SGS_SEC_MESSAGE_SIZE(SIZE) \
    (SGS_MESSAGE_SIZE(SIZE) + sizeof(sgs_msg_enc_datat))

#define SGS_MAX_MESSAGE_LENGTH 512;

#define SGS_MAX_CONNECTIONS 1024
#define SGS_CONN_BACKLOG 16
#define SGS_MAX_EVENTS 64
#define SGS_MAX_EMPTY_PINGS 8

typedef struct {
    SGS_PLAYER_DATA_T data;
    uint8_t nonce[16];
} sgs_player_data_extended_t;

typedef struct {
    int account_id;
    sgs_player_data_extended_t data_extended;
} sgs_player_login_data_t;

typedef struct {
    int account_id;
    SGS_PLAYER_DATA_T data;
} sgs_player_logout_data_t;

typedef struct {
    uint8_t nonce[16];
    uint8_t hash[32];
} sgs_account_t;

typedef struct {
    int size;
    ring_fbaa_t* account_allocator;
    sgs_account_t* accounts;
} sgs_account_database_t;

int sgs_adb_new(sgs_account_database_t* adb, int fd, int size);
void sgs_adb_destroy(sgs_account_database_t* adb);
int sgs_adb_create_account(sgs_account_database_t* adb);
int sgs_adb_sync(sgs_account_database_t* adb);
void sgs_adb_close_account(sgs_account_database_t* adb, int id);
int sgs_adb_verify_data(
    sgs_account_database_t* adb,
    int id,
    sgs_player_data_extended_t* data
);
int sgs_adb_set_data(
    sgs_account_database_t* adb,
    int id,
    sgs_player_data_extended_t* data
);

typedef struct {
    int session_id;
} sgs_player_t;

enum sgs_game_state_t {
    SGS_GS_PLAYING,
    SGS_GS_DONE
};

typedef struct {
    enum sgs_game_state_t state;
    int player_count;
    int player_session_ids[SGS_GAME_MAX_PLAYERS];
    SGS_GAME_MESSAGE_T messages[SGS_GAME_MAX_PLAYERS];
    SGS_GAME_DATA_T game_data;
} sgs_game_t;

typedef struct {
    int size;
    fbaa_t* game_allocator;
    sgs_game_t* games;
} sgs_game_server_t;

int sgs_gs_new(
    sgs_game_server_t* gs, int size
);
void sgs_gs_destroy(sgs_game_server_t* gs);
int sgs_gs_create(sgs_game_server_t* gs);
void sgs_gs_delete(sgs_game_server_t* gs, int game_id);

typedef struct { uint8_t data[12]; } sgs_session_nonce_t;

typedef enum {
    SGS_PDS_EMPTY = 0,
    SGS_PDS_CURRENT,
    SGS_PDS_ADVANCED
} sgs_player_data_state_t;

typedef struct {
    int socket_fd;
    int account_id;
    int party_id;
    int game_id;
    int player_id;
    int verified;
    sgs_player_data_state_t data_state;
    sgs_session_nonce_t nonce;
    sgs_player_data_extended_t data;
} sgs_session_t;

typedef struct {
    int size;
    ring_fbaa_t* session_allocator;
    array_rbt_t* session_map;
    sgs_session_t* sessions;
} sgs_session_server_t;

int sgs_ss_new(
    sgs_session_server_t* ss,
    int size
);
void sgs_ss_destroy(sgs_session_server_t* ss);
int sgs_ss_create(
    sgs_session_server_t* ss,
    int sfd,
    int account_id
);
void sgs_ss_delete(
    sgs_session_server_t* gs,
    int session_id
);
int sgs_ss_lookup(
    sgs_session_server_t* ss,
    int account_id
);

enum connection_state_t {
    SGS_CS_CLOSED,
    SGS_CS_HANDSHAKE,
    SGS_CS_SECURE
};

typedef struct {
    int session_id;
    int nonce;
} sgs_resume_token_t;

typedef struct {
    int session_id;
    enum connection_state_t state;
    int pings;
    uint8_t key[32];
} sgs_connection_t;

enum sgs_error_t {
    SGS_CONN_CLOSED,
    SGS_INV_MSG_TYPE,
    SGS_INV_MSG_ENCTYPE,
    SGS_INV_MSG_LEN,
    SGS_NO_ROOM_CONN,
    SGS_NO_ROOM_SS,
    SGS_INV_PDATA,
    SGS_INV_RTOKEN,
    SGS_SESSION_IN_PROGRESS,
    SGS_GAME_IN_PROGRESS
};

typedef struct epoll_event epoll_event_t;
typedef struct sockaddr_in sockaddr_in_t;

typedef struct {
    sgs_account_database_t account_db;
    sgs_game_server_t game_server;
    sgs_session_server_t session_server;
    int epoll_fd, server_fd, adb_fd;
    sockaddr_in_t socket_address;
    epoll_event_t events[SGS_MAX_EVENTS];
    sgs_connection_t connections[SGS_MAX_CONNECTIONS];
} sgs_t;

sgs_t* sgs_new(
    int accounts,
    int players,
    int games,
    const char* adb_fname
);
void sgs_destroy(sgs_t* sgs);
int sgs_listen(
    sgs_t* sgs,
    int port_number
);
int sgs_open_connection(
    sgs_t* sgs,
    int sfd
);
int sgs_send_message(
    sgs_t* sgs,
    int sfd,
    uint8_t* buffer,
    int size
);
int sgs_send_sec_message(
    sgs_t* sgs,
    int sfd,
    sgs_sec_msg_t* msg,
    int len
);
int sgs_send_ping(
    sgs_t* sgs,
    int sfd
);
void sgs_close_connection(
    sgs_t* sgs,
    int sfd
);
int sgs_connect_session(
    sgs_t* sgs,
    int sfd,
    int session_id
);
void sgs_disconnect_session(
    sgs_t* sgs,
    int session_id
);
int sgs_session_in_game(
    sgs_t* sgs,
    int session_id
);
int sgs_create_game(
    sgs_t* sgs,
    int player_ids[SGS_GAME_MAX_PLAYERS],
    int nplayers
);
void sgs_close_game(
    sgs_t* sgs,
    int game_id
);
void sgs_force_close_game(
    sgs_t* sgs,
    int game_id
);
int sgs_send_resume_token(
    sgs_t* sgs,
    int session_id,
    enum sgs_error_t* err
);
int sgs_handle_login(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
);
int sgs_handle_resume(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
);
int sgs_handle_logout(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
);
int sgs_handle_create_account(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
);
int sgs_handle_message(
    sgs_t* sgs,
    int sfd,
    enum sgs_message_type_t type,
    enum sgs_error_t* err
);
void sgs_ping_all_connections(
    sgs_t* sgs
);
void sgs_process_events(
    sgs_t* sgs
);

#endif //SGS_SIMPLE_GAME_SERVER_H
