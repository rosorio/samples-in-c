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

