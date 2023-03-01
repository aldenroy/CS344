#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ALLOWED_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZ "

int main(int argc, char *argv[]) {
    int i;
    if (argc != 2) {
        fprintf(stderr, "Usage: keygen keylength\n");
        return 1;
    }

    int key_length = atoi(argv[1]);
    srand(time(NULL));

    for (i = 0; i < key_length; i++) {
        char random_char = ALLOWED_CHARS[rand() % 27];
        printf("%c", random_char);
    }

    printf("\n");
    return 0;
}
