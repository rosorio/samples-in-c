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

#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "tcp-server.h"
#include "miniunit.h"

int tests_run = 0;

static char *test_invalid_command_message()
{
    struct peer_t peer;
    int res;
    bzero((void *)&peer, sizeof(peer));
    peer.fd = STDOUT_FILENO;

    res = process_command(&peer, "");
    mu_assert("error, test bad state", res == 502);

    res = process_command(&peer, "E");
    mu_assert("error, test bad state", res == 502);

    res = process_command(&peer, "EH");
    mu_assert("error, test bad state", res == 502);

    res = process_command(&peer, "ehlo");
    mu_assert("error, test bad state", res == 502);

    res = process_command(&peer, "ehlo Rob");
    mu_assert("error, test bad state", res == 502);

    return 0;
}

static char *test_command_message()
{
    struct peer_t peer;
    int res;
    bzero((void *)&peer, sizeof(peer));
    peer.fd = STDOUT_FILENO;

    res = process_command(&peer, "DATE");
    mu_assert("error, test bad state", res == 550);

    res = process_command(&peer, "EHLO Rob");
    mu_assert("error, ehlo Rob", res == 250);

    res = process_command(&peer, "DATE");
    mu_assert("error, date", res == 250);

    res = process_command(&peer, "QUIT");
    mu_assert("error, quit", res == 221);

    res = process_command(&peer, "EXIT");
    mu_assert("error, invalid message", res == 502);
    return 0;
}

static char *test_server()
{
    int res;
    struct server_t srv;
    bzero((char *)&srv, sizeof(srv));

    /* set the default values */
    srv.address.sin_family = AF_INET;
    srv.address.sin_port = htons(4242);
    srv.address.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.big_stop =1;

    res = start_server(&srv);

    mu_assert("error, running server on port 4242", res == 0);
    return 0;
}

static char * all_tests()
{
    mu_run_test(test_command_message);
    mu_run_test(test_server);
    mu_run_test(test_invalid_command_message);
    return 0;
}

int main (int argc, char *argv[])
{
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
