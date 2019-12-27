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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <netdb.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

#include "tcp-server.h"


struct server_t srv;

void
usage()
{
    fprintf(stderr, "Usage : tcpserver [host] [port]\n");
}

void
hander_sigint(int sig)
{
    srv.big_stop = 1;
    printf("[INFO] The server is shuting down, please wait\n");
}

int
main(int argc, char * argv[])
{
    int retcode;
    int port = 0;
    char *endp;
    struct hostent *host;
    struct sockaddr_in serveraddr;

    /* reset the structure */
    bzero((char *)&srv, sizeof(srv));
    /* set the default values */
    srv.address.sin_family = AF_INET;
    srv.address.sin_port = htons(4242);
    srv.address.sin_addr.s_addr = htonl(INADDR_ANY);

    switch (argc) {
        case 3:
            host = gethostbyname(argv[1]);
            if (host == NULL) {
                fprintf(stderr, "tcp-server: %s: %s\n", argv[1], strerror(errno));
                exit(1);
            }
            memcpy(&srv.address.sin_addr.s_addr, host->h_addr, host->h_length);
            argv++;
            /* continue */
        case 2:
            port = strtol(argv[1],&endp, 10);
            /* only accept valid port numbers */
            if (port <= 0 || port > 65535 || ((endp - argv[1]) != strlen(argv[1]))) {
                fprintf(stderr, "tcp-server: %s: Invalid port number\n", argv[1]);
            }
            srv.address.sin_port = htons(port);
            break;
        case 1:
            break;
        default:
            usage();
            exit(1);
    }

    /* ignore broken pipes */
    signal(SIGPIPE, SIG_IGN);
    /* handle sigint */
    signal(SIGINT, hander_sigint);
    retcode = start_server(&srv);

    printf("[INFO] Server stoped\n");

    exit(retcode);
}
