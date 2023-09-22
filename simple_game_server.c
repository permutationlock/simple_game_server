#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/random.h>
#include <sys/socket.h> // contains socket, connect, accept syscalls
#include <arpa/inet.h> // needed for inet_ntoa

#include <monocypher.h>

#include "simple_game_server.h"

void derive_key(
    uint8_t seckey[32], uint8_t pubkey[32]
) {
    uint8_t data[96];
    crypto_x25519(data, seckey, pubkey);
    memcpy(&(data[32]), seckey, 32);
    memcpy(&(data[64]), pubkey, 32);

    crypto_blake2b(
        seckey, 32,
        (uint8_t*)data, 96
    );
}

int read_bytes(
    uint8_t* data, int sfd, int exp_len, enum sgs_error_t* err
) {
    int len = recv(sfd, data, exp_len, 0);
    if(len < 0) {
        *err = SGS_CONN_CLOSED;
        return -1;
    }
    if(len < exp_len) {
        *err = SGS_INV_MSG_LEN;
        return -1;
    }
    return 0;
}

int read_message_info(
    sgs_msg_info_t* info,
    int sfd,
    enum sgs_error_t* err
) {
    if(
        read_bytes(
            (uint8_t*)msg,
            sfd,
            sizeof(sgs_msg_info_t),
            err
        ) < 0
    ) {
        return -1;
    }
    info->length = ntohl(info->length);

    return 0;
}

int read_message(
    uint8_t* data,
    int data_len,
    int sfd,
    sgs_msg_info_t* info,
    uint8_t key[32],
    enum sgs_error_t* err
) {
    if(info->length != data_len) {
        *err = SGS_INV_MSG_LEN;
        return -1;
    }
    if(info->enctype == SGS_MET_PLAINTEXT) {
        return read_bytes(data, sfd, info->len, err);
    }

    sgs_msg_encdata_t encdata;
    if(
        read_bytes(
            (uint8_t*)&encdata,
            sfd,
            sizeof(sgs_msg_encdata_t),
            err
        ) < 0
    ) {
        return -1;
    }

    if(read_bytes(data, sfd, info->length, err) < 0) {
        return -1;
    }

    crypto_aead_unlock(
        data,
        encdata.mac,
        key,
        msg->encdata.nonce,
        (uint8_t*)0,
        0,
        data,
        info->length
    );

    return 0;
}

int sgs_send_bytes(
    int sfd,
    uint8_t* buffer,
    int size
) {
    int len = send(sfd, buffer, size, 0);
    if(len < size) {
        return -1;
    }

    return 0;
}

int sgs_send_plaintext(
    int sfd,
    sgs_msg_t* msg,
    enum sgs_message_type_t type,
    int len
) {
    msg->info.enctype = (uint8_t)SGS_MET_PLAINTEXT;
    msg->info.type = (uint8_t)type;
    msg->info.length = htonl(len);
    return sgs_send_bytes(
        sfd,
        (uint8_t*)msg,
        SGS_MESSAGE_SIZE(len)
    );
}

int sgs_send_encrypted(
    int sfd,
    uint8_t key[32],
    sgs_sec_msg_t* msg,
    enum sgs_message_type_t type,
    int len
) {
    msg->info.enctype = (uint8_t)SGS_MET_ENCRYPTED;
    msg->info.type = (uint8_t)type;
    msg->info.length = htonl(len);

    uint8_t* msg_body = msg->message;

    if(
        getrandom(
            &(msg->encdata.nonce),
            sizeof(msg->encdata.nonce),
            GRND_NONBLOCK
        ) < sizeof(msg->encdata.nonce)
    ) {
        return -1;
    }

    crypto_aead_lock(
        msg_body,
        msg->encdata.mac,
        key,
        msg->encdata.nonce,
        (uint8_t*)0,
        0,
        msg_body,
        len
    );

    return sgs_send_bytes(
        sfd,
        (uint8_t*)msg,
        SGS_SEC_MESSAGE_SIZE(len)
    );
}

int sgs_adb_new(sgs_account_database_t* adb, int fd, int size) {
    ftruncate(fd, size);
    void* mem = mmap(
        0,
        rfbaa_mem_size(size, sizeof(sgs_account_t)),
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        fd,
        0
    );

    if(mem == (void*) 0) {
        return -1;
    }

    adb->size = size;
    adb->account_allocator = rfbaa_new_from_void(
        mem, size, sizeof(sgs_account_t)
    );
    adb->accounts = (sgs_account_t*)(
        adb->account_allocator->mem_alloc.memory
    );

    return 0;
}

void sgs_adb_destroy(sgs_account_database_t* adb) {
    msync(
        (void*)adb->account_allocator,
        rfbaa_mem_size(adb->size, sizeof(sgs_account_t)),
        MS_SYNC
    );
    munmap(
        (void*)adb->account_allocator,
        rfbaa_mem_size(adb->size, sizeof(sgs_account_t))
    );
}

int sgs_adb_create_account(sgs_account_database_t* adb) {
    int account_id = rfbaa_malloc_index(adb->account_allocator);

    if(account_id < 0) {
        rfbaa_free_oldest(adb->account_allocator);
        account_id = rfbaa_malloc_index(adb->account_allocator);
    }

    return account_id;
}

void sgs_adb_close_account(sgs_account_database_t* adb, int id) {
    rfbaa_free_index(adb->account_allocator, id);
}

int sgs_adb_verify_data(
    sgs_account_database_t* adb,
    int id,
    sgs_player_data_extended_t* data
) {
    if(id < 0 || id >= rfbaa_size(adb->account_allocator)) {
        return 0;
    }
    sgs_account_t* account = &(adb->accounts[id]);
    data->nonce = account->nonce;

    uint8_t hash[32];
    crypto_blake2b(
        hash, sizeof(hash),
        (uint8_t*)data, sizeof(sgs_player_data_extended_t)
    );
    return !crypto_verify32(hash, account->hash);
}

int sgs_adb_set_data(
    sgs_account_database_t* adb,
    int id,
    sgs_player_data_extended_t* data
) {
    if(id < 0 || id >= rfbaa_size(adb->account_allocator)) {
        return -1;
    }

    sgs_account_t* account = &(adb->accounts[id]);
    if(
        getrandom(
            &(data->nonce),
            16,
            GRND_NONBLOCK
        ) < 16
    ) {
        return -1;
    }

    account->nonce = data->nonce;
    crypto_blake2b(
        account->hash, 32,
        (uint8_t*)data, sizeof(sgs_player_data_extended_t)
    );
    
    return 0;
}

int sgs_gs_new(
    sgs_game_server_t* gs, int size
) {
    gs->size = size;
    gs->game_allocator = rfbaa_new(
        malloc, size, sizeof(sgs_game_t)
    );
    if(gs->game_allocator == (ring_fbaa_t*)0) {
        return -1;
    }
    gs->games = (sgs_game_t*)gs->game_allocator->mem_alloc.memory;

    return 0;
}

void sgs_gs_destroy(sgs_game_server_t* gs) {
    rfbaa_destroy(free, gs->game_allocator);
}

int sgs_gs_create(sgs_game_server_t* gs) {
    return rfbaa_malloc_index(gs->game_allocator);
}

void sgs_gs_delete(
    sgs_game_server_t* gs,
    int game_id
) {
    rfbaa_free_index(gs->game_allocator, game_id);
}

int sgs_gs_get_oldest(sgs_game_server_t* gs) {
    return rfbaa_oldest_index(gs->game_allocator);
}

int sgs_ss_new(
    sgs_session_server_t* ss, int size
) {
    ss->size = size;
    ss->session_allocator = fbaa_new(
        malloc, size, sizeof(sgs_session_t)
    );
    if(ss->session_allocator == (fbaa_t*)0) {
        return -1;
    }
    ss->sessions = (sgs_session_t*)ss->session_allocator->memory;

    return 0;
}

void sgs_ss_destroy(sgs_session_server_t* ss) {
    fbaa_destroy(free, (void*)ss->session_allocator);
}

int sgs_ss_create(
    sgs_session_server_t* ss,
    int sfd,
    int account_id
) {
    sgs_session_nonce_t nonce;
    if(
        getrandom(
            &nonce,
            sizeof(nonce),
            GRND_NONBLOCK
        ) < sizeof(nonce)
    ) {
        return -1;
    }

    if(arbt_contains(ss->session_map, account_id)) {
        return arbt_get(ss->session_map, account_id);
    }

    int session_id = fbaa_malloc_index(ss->session_allocator);
    if(session_id < 0) {
        return -1;
    }
    
    if(account_id >= 0) {
        arbt_insert(ss->session_map, account_id, session_id);
    }

    sgs_session_t* session = &(ss->sessions[session_id]);
    session->socket_fd = sfd;
    session->account_id = account_id;
    session->player_id = -1;
    session->game_id = -1;
    session->nonce = nonce;
    session->verified = 0;

    return session_id;
}

void sgs_ss_delete(
    sgs_session_server_t* ss,
    int session_id
) {
    sgs_session_t* session = &(ss->sessions[session_id]);
    if(session->account_id >= 0) {
        arbt_delete(
            ss->session_map,
            session->account_id
        );
    }
    fbaa_free_index(ss->session_allocator, session_id);
}

int sgs_ss_lookup(
    sgs_session_server_t* ss,
    int account_id
) {
    if(arbt_contains(ss->session_map, account_id)) {
        return arbt_get(ss->session_map, account_id);
    }

    return -1;
}

int sgs_ss_is_open(
    sgs_session_server_t* ss,
    int session_id
) {
    if(session_id < 0 || session_id >= ss->size) {
        return 0;
    }
    return ss->session_allocator->full_blocks[session_id];
}

sgs_t* sgs_new(
    int accounts,
    int players,
    int games,
    const char* adb_fname
) {
    int adb_fd = open(
        adb_fname,
        O_RDWR | O_CREAT,
        (mode_t)0600
    );
    if(adb_fd < 0) {
        return (sgs_t*)0;
    } 

    sgs_t* sgs = (sgs_t*)malloc(
        sizeof(sgs_t)
    );
    
    if(sgs == (sgs_t*)0) {
        close(adb_fd);
        return (sgs_t*)0;
    }

    if(sgs_adb_new(&(sgs->account_db), accounts, sgs->adb_fd) < 0) {
        close(adb_fd);
        free((void*)sgs);
        return (sgs_t*)0;
    }
    if(sgs_ss_new(&(sgs->session_server), players) < 0) {
        close(adb_fd);
        sgs_adb_destroy(&(sgs->account_db));
        free((void*)sgs);
        return (sgs_t*)0;
    }
    if(sgs_gs_new(&(sgs->game_server), games) < 0) {
        close(adb_fd);
        sgs_adb_destroy(&(sgs->account_db));
        sgs_ss_destroy(&(sgs->session_server));
        free((void*)sgs);
        return (sgs_t*)0;
    }
    sgs->epoll_fd = -1;
    sgs->adb_fd = adb_fd;
    for(int i = 0; i < SGS_MAX_CONNECTIONS; ++i) {
        sgs_connection_t* connection = &(sgs->connections[i]);
        connection->session_id = -1;
        connection->state = SGS_CS_CLOSED;
        connection->pings = 0;
    }

    return sgs;
}

void sgs_destroy(sgs_t* sgs) {
    sgs_adb_sync(&(sgs->account_db));
    close(sgs->adb_fd);
    close(sgs->server_fd);
    close(sgs->epoll_fd);
    sgs_adb_destroy(&(sgs->account_db));
    sgs_ss_destroy(&(sgs->session_server));
    sgs_gs_destroy(&(sgs->game_server));
    free((void*)sgs);
}

int sgs_listen(sgs_t* sgs, int port_number) {
    memset(&(sgs->socket_address), 0, sizeof(sockaddr_in_t));
    sgs->socket_address.sin_family = AF_INET;                
    sgs->socket_address.sin_addr.s_addr = htonl(INADDR_ANY); 
    sgs->socket_address.sin_port = htons(port_number);

    sgs->server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if(sgs->server_fd == -1) {
        return -1;
    }

    int sopt = 1;
    int err = setsockopt(
        sgs->server_fd,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char *)&sopt,
        sizeof(sopt)
    );
    if(err < 0) {
        close(sgs->server_fd);
        return -1;
    }

    err = bind(
        sgs->server_fd,
        (struct sockaddr *)&(sgs->socket_address),
        sizeof(sockaddr_in_t)
    );

    if(err < 0) {
        close(sgs->server_fd);
        return -1;
    }

    err = listen(sgs->server_fd, SGS_CONN_BACKLOG);

    sgs->epoll_fd = epoll_create(0xf00d);
    if(sgs->epoll_fd < 0) {
        close(sgs->server_fd);
        return -1;
    }

    struct epoll_event ev = { 0 };
    ev.data.fd = sgs->server_fd;
    ev.events = EPOLLIN;

    err = epoll_ctl(
        sgs->epoll_fd,
        EPOLL_CTL_ADD,
        sgs->server_fd,
        &ev
    );

    if(err < 0) {
        close(sgs->server_fd);
        close(sgs->epoll_fd);
        return -1;
    }

    return 0;
}

int sgs_open_connection(
    sgs_t* sgs,
    int sfd
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    connection->session_id = -1; 
    connection->pings = 0;
    connection->state = SGS_CS_HANDSHAKE;

    if(
        getrandom(
            &(connection->key),
            sizeof(connection->key),
            GRND_NONBLOCK
        ) < sizeof(connection->key)
    ) {
        return -1;
    }

    SGS_MESSAGE_T(32) pkey;
    crypto_x25519_public_key(pkey.message, connection->key);
    return sgs_send_plaintext(
        sgs,
        sfd,
        (sgs_msg_t*)&pkey,
        SGS_MT_PUBKEY,
        32
    );
}

sgs_msg_t SGS_PING_MESSAGE = {
    { SGS_MET_PLAINTEXT, SGS_MT_PING, 0 }
};

int sgs_send_ping(
    sgs_t* sgs,
    int sfd
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    if(connection->session_id < 0) {
        if(connection->pings > SGS_MAX_EMPTY_PINGS) {
            sgs_close_connection(sgs, sfd);
        }
    }
    connection->pings += 1;

    int err = sgs_send_bytes(
        sgs,
        sfd,
        (uint8_t*)&SGS_PING_MESSAGE,
        SGS_MESSAGE_SIZE(0)
    );
    if(err < 0) {
        sgs_close_connection(sgs, sfd);
        return -1;
    }
    return 0;
}

void sgs_close_connection(
    sgs_t* sgs,
    int sfd
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    if(connection->session_id >= 0) {
        sgs_disconnect_session(sgs, connection->session_id);
    }
    connection->state = SGS_CS_CLOSED;
    close(sfd);
}

int sgs_connect_session(
    sgs_t* sgs,
    int sfd,
    int session_id
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    sgs_session_t* session = &(
        sgs->session_server.sessions[session_id]
    ); 
    if(session->socket_fd >= 0 || connection->session_id >= 0) {
        return -1;
    }
    connection->session_id = session_id;
    session->socket_fd = sfd;
    return 0;
}

void sgs_disconnect_session(
    sgs_t* sgs,
    int session_id
) {
    sgs_session_t* session = &(
        sgs->session_server.sessions[session_id]
    );
    session->socket_fd = -1;

    if(session->game_id >= 0) {
        return;
    }

    if(session->player_id >= 0) {
        sgs_delete_player(sgs, session->player_id);
    }

    sgs_ss_delete(&(sgs->session_server), session_id);
}

int sgs_send_logout_token(
    sgs_t* sgs,
    int session_id
) {
    sgs_session_t* session = &(
        sgs->session_server.sessions[session_id]
    );
}

int sgs_create_game(
    sgs_t* sgs,
    int session_ids[SGS_GAME_MAX_PLAYERS],
    int nplayers
) {
    int game_id = sgs_gs_create(&(sgs->game_server));
    sgs_game_t* game = &(sgs->game_server.games[game_id]);
    game->player_count = nplayers;
    game->state = SGS_GS_PLAYING;

    for(int i = 0; i < nplayers; ++i) {
        sgs_session_t* session = &(
            sgs->session_server.sessions[session_ids[i]]
        );
        session->game_id = game_id;
        session->player_id = i;
        game->player_session_ids[i] = session_id;
    }

    return game_id;
}

void sgs_close_game(
    sgs_t* sgs,
    int game_id
) {
    sgs_game_t* game = &(sgs->game_server.games[game_id]);

    if(game->state != SGS_GS_DONE) {
        for(int i = 0; i < game->player_count; ++i) {
            sgs_session_t* session = &(
                sgs->session_server.sessions[game->session_ids[i]]
            );
            if(session->account_id < 0) {
                continue;
            }
            sgs_adb_set_data(
                &(sgs->account_db),
                player->account_id,
                &(player->data)
            );
        }
    }
    game->state = SGS_GS_DONE;

    int done = 1;
    for(int i = 0; i < game->player_count; ++i) {
        sgs_player_t* player = &(game->players[i]);

        // check if this player's session has already closed
        if(player->session_id < 0) {
            continue;
        }

        sgs_session_t* session = &(
            sgs->session_server.sessions[player->session_id]
        );

        int sfd = session->socket_fd;

        // if session is open with no connection, can't close session
        if(sfd < 0) {
            done = 0;
            continue;
        }

        sgs_connection_t* conneciton = &(sgs->connections[sfd]);

        if(player->account_id < 0) {
            player->account_id = sgs_adb_create_account(
                &(sgs->account_db)
            );
            sgs_adb_set_data(
                &(sgs->account_db),
                player->account_id,
                &(player->data)
            );
        }

        SGS_SEC_MESSAGE_T(sizeof(sgs_player_logout_data_t)) msg;
        sgs_player_logout_data_t* ldata =
            (sgs_player_logout_data_t*)msg.message;
        ldata->account_id = htonl(player->account_id);
        memcpy(
            &(ldata->data),
            &(player->data),
            SGS_PLAYER_DATA_SIZE
        );

        int err = sgs_send_encrypted(
            sfd,
            connection->key,
            (sgs_sec_msg_t*)&ldata,
            SGS_MT_LOGOUT,
            sizeof(sgs_player_logout_data_t)
        );

        // if the message couldn't send, don't close the session
        if(err < 0) {
            sgs_close_connection(sgs, sfd);
            done = 0;
            continue;
        }

        // remove game association from this player's session
        session->game_id = -1;
        session->player_id = -1; 

        // mark that this player's session has closed successfully
        player->session_id = -1;

        // close the associated connection and session
        sgs_close_connection(sgs, sfd);
    }

    if(done == 1) {
        sgs_gs_delete(&(sgs->game_server), game_id);
    }
}

void sgs_force_close_game(
    sgs_t* sgs,
    int game_id
) {
    sgs_game_t* game = &(sgs->game_server.games[game_id]);

    for(int i = 0; i < game->player_count; ++i) {
        sgs_player_t* player = &(game->players[i]);
        int session_id = player->session_id;

        if(session_id < 0) {
            continue;
        }

        sgs_session_t* session = &(
            sgs->session_server.sessions[session_id]
        );
        session->game_id = -1;
        session->player_id = -1; 
        player->session_id = -1;

        int sfd = session->socket_fd;
        if(sfd < 0) {
            sgs_disconnect_session(sgs, session_id);
            continue;
        }

        sgs_close_connection(sgs, sfd);
    }

    sgs_gs_delete(&(sgs->game_server), game_id);
}

int sgs_send_resume_token(
    sgs_t* sgs,
    int session_id,
    enum sgs_error_t* err
) {
    sgs_session_t* session = &(
        sgs->session_server.sessions[session_id]
    );
    sgs_connection_t* conneciton = &(sgs->connections[session->sfd]);

    SGS_SEC_MESSAGE_T(sizeof(sgs_resume_token_t)) msg;
    sgs_resume_token_t* rtoken = (sgs_resume_token_t*)msg.message;
    rtoken->session_id = session_id;
    rtoken->nonce = session->nonce;
    if(
        sgs_send_encrypted(
            session->sfd,
            connection->key,
            (sgs_sec_msg_t*)&msg,
            SGS_MT_RESUME,
            sizeof(sgs_resume_token_t)
        ) < 0
    ) {
        sgs_close_connection(sgs, session->sfd);
        *err = SGS_CONN_CLOSED;
        return -1;
    }
    
    return 0;
}

int sgs_handle_login(
    sgs_t* sgs,
    int sfd,
    sgs_msg_info_t* msg_info,
    enum sgs_error_t* err
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);

    sgs_player_login_data_t login_data;
    int err = sgs_read_message(
        (uint8_t*)&pdata,
        sizeof(sgs_player_logout_data_t),
        sfd,
        msg_info,
        conneciton->key,
        err
    );
    if(err < 0) {
        return -1;
    }

    int account_id = ntohl(login_data.account_id);
    err = sgs_adb_verify_data(
        &(sgs->account_db),
        account_id,
        &(login_data.data_extended)
    );
    if(err < 0) {
        *err = SGS_INV_PDATA;
        return -1;
    }

    int session_id = sgs_ss_create(
        &(sgs->session_server),
        sfd,
        account_id
    );
    if(session_id < 0) {
        *err = SGS_NO_ROOM_SS;
        return -1;
    }

    err = sgs_connect_session(sgs, sfd, session_id);
    if(err < 0) {
        *err = SGS_SESSION_IN_PROGRESS;
        return -1;
    }
    
    sgs_session_t* session = &(
        sgs->session_server.sessions[session_id]
    );
    if(session->game_id >= 0) {
        return 0;
    }

    err = sgs_create_player(sgs, session_id);
    if(err < 0) {
        *err = SGS_NO_ROOM_MM;
        return -1;
    }

    sgs_player_t* player = &(
        sgs->matchmaker.players[session->player_id]
    );
    memcpy(
        &(player->data),
        &(login_data.data_extended.data),
        SGS_PLAYER_DATA_SIZE
    );

    return sgs_send_resume_token(sgs, session_id, err);
}

int sgs_handle_resume(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
) {
    sgs_resume_token_t rtoken;
    if(
        sgs_read_message(
            sfd,
            (uint8_t*)&rtoken,
            sizeof(sgs_resume_token_t),
            err
        ) < 0
    ) {
        return -1;
    }
    if(!sgs_ss_is_open(&(sgs->session_server), rtoken.session_id)) {
        *err = SGS_INV_RTOKEN;
        return -1;
    }
    sgs_session_t* session = &(
        sgs->session_server.sessions[rtoken.session_id]
    );
    if(session->nonce != rtoken.nonce) {
        *err = SGS_INV_RTOKEN;
        return -1;
    }
    if(sgs_connect_session(sgs, sfd, rtoken.session_id) < 0) {
        *err = SGS_INV_RTOKEN;
        return -1;
    }

    return 0;
}

int sgs_handle_logout(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    sgs_session_t* session = &(
        sgs->session_server.sessions[connection->session_id]
    );
    if(session->game_id < 0) {
        SGS_MESSAGE_T(0) msg;
        sgs_send_plaintext(
            sgs,
            sfd,
            (sgs_sec_msg_t*)&msg,
            SGS_MT_SUCCESS,
            0
        );
        sgs_close_connection(sgs, sfd);
        return 0;
    }

    *err = SGS_GAME_IN_PROGRESS;
    return -1;
}

int sgs_handle_create_account(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
) {
    int session_id = sgs_ss_create(
        &(sgs->session_server),
        sfd,
        -1
    );
    if(session_id < 0) {
        *err = SGS_NO_ROOM_SS;
        return -1;
    }

    if(sgs_connect_session(sgs, sfd, session_id) < 0) {
        *err = SGS_SESSION_IN_PROGRESS;
        return -1;
    }

    if(sgs_create_player(sgs, session_id) < 0) {
        *err = SGS_NO_ROOM_MM;
        return -1;
    }

    return sgs_send_resume_token(sgs, session_id, err);
}

int sgs_handle_game_message(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    sgs_session_t* session = &(
        sgs->session_server.sessions[connection->session_id]
    );
    if(session->game_id < 0) {
        *err = SGS_INV_MSG_TYPE;
        return -1;
    }
    sgs_game_t* game = &(
        sgs->game_server.games[session->game_id]
    );
    if(
        sgs_read_message(
            sfd,
            (uint8_t*)&(game->messages[session->player_id]),
            SGS_GAME_MESSAGE_SIZE,
            err
        ) < 0
    ) {
        return -1;
    }

    return 0;
}

int sgs_handle_handshake(
    sgs_t* sgs,
    int sfd,
    sgs_msg_info_t* msg_info,
    enum sgs_error_t* err
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);

    if(msg_info.enctype != SGS_MET_PLAINTEXT) {
        *err = SGS_INV_MSG_ENCTYPE;
        return -1;
    }
    if(msg_info.type != SGS_MT_PUBKEY) {
        *err = SGS_INV_MSG_TYPE;
        return -1;
    }
    uint8_t pubkey[32];
    int err = sgs_read_message(
        pubkey, 32, sfd, &msg_info, (uint8_t*)0, err
    );
    if(err < 0) {
        return -1;
    }
    derive_key(connection->key, msg.message);
    connection->state = SGS_CS_SECURE;

    return 0;
}

int sgs_handle_ping(
    sgs_t* sgs,
    int sfd,
    sgs_msg_info_t* msg_info,
    enum sgs_error_t* err
) {
    if(msg_info.enctype != SGS_MET_PLAINTEXT) {
        *err = SGS_INV_MSG_ENCTYPE;
        return -1;
    }
    if(msg_info.length != 0) {
        *err = SGS_INV_MSG_LEN;
        return -1;
    }
    int err = sgs_send_ping(sgs, sfd);
    if(err < 0) {
        *err = SGS_CONN_CLOSED;
        return -1;
    }

    return 0;
}

int sgs_handle_message(
    sgs_t* sgs,
    int sfd,
    enum sgs_error_t* err
) {
    sgs_connection_t* connection = &(sgs->connections[sfd]);
    sgs_msg_info_t msg_info;
    int err;
    err = sgs_read_message_info(
        sfd, &msg_info, err
    );
    if(err < 0) {
        return -1;
    }

    if(msg_info.type == SGS_MET_PING) {
        return sgs_handle_ping(sgs, sfd, &msg_info, err);
    }

    if(connection->state != SGS_CS_SECURE) {
        return sgs_handle_handshake(sgs, sfd, &msg_info, err);
    }

    if(msg_info.enctype != SGS_MET_ENCRYPTED) {
        err = SGS_INV_MSG_ENCTYPE;
        return -1;
    }
    
    if(connection->session_id < 0) {
        switch(type) {
            case SGS_MT_LOGIN:
                return sgs_handle_login(sgs, sfd, &msg_info, err);
            case SGS_MT_CREATE:
                return sgs_handle_create_account(
                    sgs, sfd, &msg_info, err
                );
            case SGS_MT_RESUME:
                return sgs_handle_resume(sgs, sfd, &msg_info, err);
            default:
                *err = SGS_INV_MSG_TYPE;
                return -1;
        }
    }

    switch(type) {
        case SGS_MT_LOGOUT:
            return sgs_handle_logout(sgs, sfd, &msg_info, err);
        case SGS_MT_GAME:
            return sgs_handle_game_message(sgs, sfd, &msg_info, err);
        default:
            *err = SGS_INV_MSG_TYPE;
            return -1;
    }

    return 0;
}

void sgs_ping_all_connections(
    sgs_t* sgs
) {
    for(int i = 0; i < SGS_MAX_CONNECTIONS; ++i) {
        if(sgs->connections[i].state == SGS_CS_CLOSED) {
            continue;
        }
        sgs_send_ping(sgs, i);
    }
}

void sgs_process_events(
    sgs_t* sgs
) {
    return;
}

