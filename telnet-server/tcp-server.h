#ifndef TCP_SERVER_H
#define TCP_SERVER_H

struct peer_t {
    int fd;
    int len;
    int remove;
    int hello;
    int id;
    char buf[BUFSIZ];
    struct peer_t *next;
} peer_t;

typedef struct server_t {
    struct peer_t *peers;
    struct sockaddr_in address;
    int big_stop;
    int fd;
    int peer_id;
} server_t;

int start_server(struct server_t *server);
int process_command(struct peer_t *peer, char *command);
int write_peer(struct peer_t *peer, char * message);

#endif
