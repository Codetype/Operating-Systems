/*
*
* Gedlek Pawel
* Lab 5, zad 1
*
*/
#include <stdio.h>
#include <malloc.h>
#include <zconf.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/wait.h>

#define descrNum  10
int redHandler[descrNum];
int descriptors[descrNum][2];

void pullArg(char* args[128], int curr, int argNum){
    while (curr+1 < argNum){
        args[curr] = args[curr+1];
        curr++;
    }
    args[curr] = NULL;
}

int mergeArgs(char* args[128], int argNum, int pattern){
    int i = 0, merged = 0;
    while(i+1 < argNum-merged){
        if (args[i][0] == pattern){
            do {
                args[i][strlen(args[i])] = ' ';
                pullArg(args, i+1, argNum-merged);
                merged++;
            } while ((i < argNum-merged) && ((args[i][strlen(args[i])-1] != pattern ) ||
                                                 (args[i][strlen(args[i])-1] == pattern && args[i][strlen(args[i])-2] == '\\')));
            args[i][strlen(args[i])-1] = 0;
            args[i] = &args[i][1];
        }
        i ++;
    }
    return argNum - merged;
}

void createPipes(){
    int i;
    for(i=0; i<descrNum; i++){
        if(pipe(descriptors[i]) == -1){
            perror("Error occurs! Pipe is broken.");
            exit(1);
        }
    }
}

void closePipes(){
    int i;
    for(i=0; i<descrNum; i++){
        close(descriptors[i][0]);
        close(descriptors[i][1]);
        close(redHandler[i]);
    }
}


int main(int argc, char *argv[] ) {
    int i, argsNumber = 0, cmndNum = 0, redirected = 0;
    if (argc < 2){
        perror("Wrong number of arguments!\n");
        return (-1);
    }

    char *fileName = argv[1];
    FILE *fileHandler = fopen(fileName, "r");

    if (fileHandler == 0){
        perror("Error occurs during opening file");
        exit(0);
    }

    char *args[512], *cmnds[512], *redName[128];
    char line[1024], buff[4096];
    pid_t childProcess;

    createPipes();
    while(fgets(line, 1024, fileHandler)) {
        createPipes();

        if (strlen(line) == 1) continue;
        printf("Execute line: %s", line);
        cmndNum = 0;

        cmnds[cmndNum++] = strtok(line, "|\n");
        while ((cmnds[cmndNum++] = strtok(NULL, "|\n")) != NULL);

         cmndNum--;

        for (i = 0; i < cmndNum; ++i) {
            argsNumber = 0;

            if (strstr(cmnds[i], ">")){
                cmnds[i] = strtok(cmnds[i], ">");
                redName[i] = strtok(NULL, ">");
                redirected = 1;
                redHandler[i] = open(redName[i],  O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
            }

            args[argsNumber++] = strtok(cmnds[i], " \n");
            while ((args[argsNumber++] = strtok(NULL, " \n")) != NULL);

            argsNumber--;
            argsNumber = mergeArgs(args, argsNumber, '\"');
            argsNumber = mergeArgs(args, argsNumber, '\'');

            for (i=0; i<argsNumber; i++) printf("%s ", args[i]);
            printf("\n");

            if ((childProcess = fork()) == -1){
                perror("Error occurs during exceuting fork()");
                exit(1);
            }

            if (childProcess == 0) {
                if((i != cmndNum -1) || redirected)
                    dup2(descriptors[i+1][1],  STDOUT_FILENO);
                    dup2(descriptors[i][0], STDIN_FILENO);
                if (execvp(args[0], args) == -1) {
                    perror("Error occurs during exec filepath");
                    exit(-1);
                }
            }
            if (redirected){
                int length = (int) read(descriptors[i+1][0], buff, 4096);
                write(redHandler[i], buff, (size_t) length);
                write(descriptors[i+1][1], buff, (size_t) length);
            }
            close(descriptors[i+1][1]);
            redirected = 0;
        }

        printf("All comands: %d, waiting for them to exit\n", cmndNum);
        for (i = 0; i < cmndNum; ++i) {
            wait(NULL);
            printf("%d. thread is now exited\tIt was command: \n", i, cmnds[i]);
        }
        closePipes();
    }
    fclose(fileHandler);
    printf("\n");
    return 0;
}
