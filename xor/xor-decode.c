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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFERSIZE 1024
int decode_message (char * key, FILE * f)
{
    char buffer[BUFFERSIZE];
    int wc, kc, kl, cnt;
    char clear;

    kc = 0;
    kl = strlen(key);
    while((cnt = fread(buffer, 1, BUFFERSIZE, f)) > 0)
    {
        for (wc=0; wc < cnt; wc++) {
            
            clear = (char)(buffer[wc] ^ key[kc % kl]);
            printf("%c", clear);
            kc++;
        }
    }

    if (!feof(f)) {
        fprintf(stderr, "\nxor-decode: %s\n", strerror(errno));
        return 1;
    }

    return 0;
}

void usage()
{
    fprintf(stderr, "Usage : xor-decode -k key [-f file]\n");
}

int main(int argc, char * argv[])
{
    int ch, vflag, retcode;
    FILE *fd = NULL;
    char *key = NULL, *file = NULL;

    while ((ch = getopt(argc, argv, "k:f:hv")) != -1)
    {
        switch (ch) {
        case 'k':
            key = strdup(optarg);
            if (key == NULL) {
                fprintf(stderr, "xor-decode: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }
            break;
        case 'f':
            file = strdup(optarg);
            if (file == NULL) {
                fprintf(stderr, "xor-decode: %s: %s\n", optarg, strerror(errno));
                exit(1);
            }

            break;
        case 'v':
            vflag = 1;
            break;
        case 'h':
            /* Call usage and exit */
            usage();
            exit (0);
            
         }
    }

    if (key == NULL) {
        fprintf(stderr, "xor-decode: You must provide a key in order to decode the message\n");
        usage();
        exit(1);
    }

    if (file != NULL) {
        if ((fd = fopen(file, "rb")) == NULL) {
            fprintf(stderr, "xor-decode: %s: %s\n", file, strerror(errno));
            exit(1);
        }
    }

    retcode = decode_message(key, (fd != NULL) ? fd : stdin);

    if (fd != NULL) {
        fclose(fd);
    }

   exit(retcode);
}

