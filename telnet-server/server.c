/*-
 * Copyright (c) 2019 Rodrigo Osorio <rodrigo@osorio.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "tcp-server.h"

void
shutdown_properly(struct server_t * server)
{
    struct peer_t *p, *todel;
    p = server->peers;
    while (NULL != p) {
        todel = p;
        p = p->next;
        /* shutdown and free */
        shutdown(todel->fd, SHUT_RDWR);
        close(todel->fd);
        free(todel);
    }
    /* no more peers */
    server->peers = NULL;
}

void
shutdown_peer(struct peer_t * peer)
{
    /* proper tcp shutdown */
    shutdown(peer->fd, SHUT_RDWR);
    close(peer->fd);
    printf("[INFO] Peer %d: close connexion\n", peer->id);
}

int
build_fd_set(struct server_t * server, fd_set * readfds)
{
    struct peer_t *prev, *peer, *todel;
    int max_fd;

    max_fd = server->fd;
    FD_ZERO(readfds);
    FD_SET(server->fd, readfds);
    peer = server->peers;
    prev = NULL;
    while (peer != NULL) {
        if (peer->remove) {
            if (server->peers == peer){
                server->peers = peer->next;
            } else {
                prev->next = peer->next;
            }
            todel = peer;
            peer = peer->next;

            shutdown_peer(todel);
            free(todel);
            continue; 
        }

        if (peer->fd > max_fd) {
            max_fd = peer->fd;
        }
        FD_SET(peer->fd, readfds);

        prev = peer;
        peer = peer->next;
    }
    return max_fd;
}

int
write_peer(struct peer_t *peer, char * message) {
    write(peer->fd, message, strlen(message));
    write(peer->fd, "\n", 1);
    return 0;
}

void
add_new_peer(struct server_t *server, int peer_fd, struct sockaddr_in *peer_info)
{
    struct peer_t *newpeer;

    newpeer = malloc(sizeof(struct peer_t));
    if (newpeer == NULL) {
        printf("[ERR] malloc(): %s\n", strerror(errno));
        exit(-1);
    }

    bzero(newpeer, sizeof(struct peer_t));
    newpeer->fd = peer_fd;
    newpeer->id = server->peer_id++;
    newpeer->next = server->peers;
    server->peers = newpeer;

    printf("[INFO] Peer %d: Incomming connexion from %s:%d\n",
        newpeer->id,
        inet_ntoa(peer_info->sin_addr),
        ntohs(peer_info->sin_port));
    /* Send welcome message */
    write_peer(newpeer, "220 localhost");
}

int
read_from_peer(struct peer_t * peer) {
    char buf[BUFSIZ];
    int n, i;

    if ((n = read(peer->fd, buf, BUFSIZ)) > 0) {
        for (i=0; i < n; i++) {
            if ('\n' == buf[i]) {
                peer->buf[peer->len++] = '\0';
                /* process the comand */
                switch (process_command(peer, peer->buf)) {
                    case 502:
                        printf("[WARN] Peer %d: unknown command, shutdown connexion\n", peer->id);
                        return (-1); /* close the connexion */
                    case 221:
                        printf("[INFO] Peer %d: quit\n", peer->id);
                        return (-2);
                    default:
                        /* reset buffer counter */
                        peer->len = 0;
                }
            } else if ('\r' == buf[i]) {
                /* skip it*/
            } else {
                peer->buf[peer->len++] = buf[i];
            }

            if (BUFSIZ  <= (peer->len)) {
                /* close the session */
                printf("[WARN] Peer %d: command too long, shutdown connexion\n", peer->id);
                peer->len = 0;
                return (-1);
            }
        }
    } else if (n < 0) {
        return (-1);
    }

    return 0;
}

void
read_from_peers(struct server_t *server, fd_set *readfds) {
    struct peer_t * peer;
    peer = server->peers;
    while ( peer ) {
        if (FD_ISSET(peer->fd, readfds)) {
            if (read_from_peer(peer) < 0) {
                peer->remove = 1;
            }
        }
    	peer = peer->next;
    }
}

int
start_server(struct server_t * server) {

    fd_set readfds;
    int optval = 1; /* flag value for setsockopt */
    struct timeval tv;
    int activity;
    int peer_fd;
    struct sockaddr_in peer_info;
    socklen_t peer_len;
    int max_fd;
    

    printf("[INFO] Starting server on %s:%d\n",
        inet_ntoa(server->address.sin_addr),
        ntohs(server->address.sin_port));

    /* create the server socket */
    server->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->fd < 0) {
        printf("[ERR] socket(): %s\n", strerror(errno));
        return(1);
    }

    /* set the SO_REUSEADDR */
    setsockopt(server->fd, SOL_SOCKET, SO_REUSEADDR,
        (const void *)&optval , sizeof(int));

    /* bind server */
    if (bind(server->fd, (struct sockaddr *) &server->address, sizeof(server->address)) < 0 ){
        printf("[ERR] bind(): %s\n", strerror(errno));
        return(1);
    }

    if (listen(server->fd, 90) < 0) {
        printf("[ERR] listen(): %s\n", strerror(errno));
    }

    while (server->big_stop == 0) {
        bzero(&tv, sizeof(tv));
        tv.tv_sec = 1;
        max_fd = build_fd_set(server, &readfds);
        
        activity = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        switch (activity) {
        case -1:
            if (errno == EINTR) {
                continue;
            }
            printf("[ERR] select(): %s\n", strerror(errno));
            shutdown_properly(server);
            return(-1);
        case 0:
            /* Select timeout */
            break;
        default:
            if (FD_ISSET(server->fd, &readfds)) {
                /* New peer */
                peer_len = sizeof(peer_info);
                peer_fd = accept(server->fd, (struct sockaddr *)&peer_info, &peer_len);
                if (peer_fd >= 0) {
                    add_new_peer(server, peer_fd, &peer_info);
                }
            } 
            read_from_peers(server, &readfds);
            
        }
    }
    shutdown(server->fd, SHUT_RDWR);
    close(server->fd);
    return 0;
}
