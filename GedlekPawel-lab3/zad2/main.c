/*
*
* PAWEŁ GĘDŁEK
* LAB3, ZAD 2
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <memory.h>
#include <unistd.h>

#define maxArgs 10
#define maxLines 100

void executeCommands(char* filename){
  FILE* file = fopen(filename, "r");
  if(file == NULL){
    printf("Error during opening file with commands. :(");
    exit(0);
  }
  char lineBuffor[maxLines], *parametersBuffor[maxArgs];
  int argIter = 0;
  //fgets need: string buffer, snumber of elements and stdin
  //return NULL when gains eof
  while(fgets(lineBuffor, maxLines, file)){
    argIter = 0;
    //strtok take current string and go to the next pointer
    //return NULL, when it doesn't see a next string
    while((parametersBuffor[argIter] = strtok(argIter == 0 ? lineBuffor : NULL, " \n\t")) != NULL){
      //iterator goes through the file and check the numbers of it, because we set a limit
      argIter++;
      if(argIter >= maxArgs){
        printf("You set up too many arguments in file! Limit is: %d", maxArgs);
      }
    }
    printf("Current pid is: %d, parent pid: %d\n",getpid(),getppid());
    pid_t pid = fork();
    // >0 - parent; ==0 - child; <0 - error
    if(pid == 0){
      printf("Start new process! Its pid is: %d, and its parent pid: %d\n",getpid(), getppid());
      //function to get exec function and needed arguments
      execvp(parametersBuffor[0], parametersBuffor);
    }
    int status;
    //check children proces status, wait until all child proces will be ended
    wait(&status);
    //we have to report potential error
    if(status){
      printf("Error while running command nr: %s\n", parametersBuffor[0]);
    }
  }
  fclose(file);
}

int main(int argc, char *argv[]){
  if(argc < 2){
    printf("You have to set up 1 more argument!\nWe need name of file.\n");
    return 1;
  }
  executeCommands(argv[1]);
  return 0;
}
