#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <errno.h>
char *filepath;

int main(int argv, char *argc[]) {
    if (argv != 2) return 3;
    filepath = argc[1];
    if(mkfifo(filepath,  S_IRUSR | S_IWUSR)){
        printf("Oh dear, something went wrong with read()! %s\n", strerror(errno));
        return 4;
    }
    FILE *file = fopen(filepath, "r");
    if (file == NULL) return 2;

    char *buffer = NULL;
    size_t size = 0;
    while(true) {
        while ((size = getline(&buffer, &size, file)) != -1) {
            printf("%s\n", buffer);
        }
    }
    fclose(file);
    return 0;
}
