#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>


void usage(char * bin)
{
    printf("Usage: %s <key>\n", bin);
}

unsigned char convert(unsigned char c, unsigned char k)
{
    char ck = toupper(c) - k;
    if (ck < 0) ck = 26 + ck;
    return isupper(c) ? 'A' + ck  : 'a' + ck;
}

int main(int argc, char * argv[])
{
    unsigned char c;
    int i = 0;
    char *command = strdup(basename(argv[0]));

    if (argc != 2) {
        printf ("Error: The key is missing\n");
        usage(command);
        free(command);
        return -1;
    }

    char *key = strdup(argv[1]);
    int  klen = strlen(key);

    while (!feof(stdin)) {
        c = getc(stdin);
        if (isalpha(c)) {
            c = convert(c, toupper(key[i % klen]));
            i++;
        }
        printf("%c", c);
    }
}
