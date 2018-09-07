/*
*
* PAWEŁ GĘDŁEK
* LAB 3, ZAD 3
*
*/

#define _BSD_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <memory.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

#define maxArgs 10
#define maxLines 100

int softAndHardLimits(char *currTime, char *mem){
  long int tmLimit = strtol(currTime, NULL, 10);

  //struct to define soft and hard limits
  struct rlimit limitCPU;
  limitCPU.rlim_max = (rlim_t) tmLimit;
  limitCPU.rlim_cur = (rlim_t) tmLimit;

  //RLIMIT_CPU: limit in seconds which process can consume
  if(setrlimit(RLIMIT_CPU, &limitCPU) != 0){
    printf("Error during setting CPU limit");
    return -1;
  }

  long int memLimit = strtol(mem, NULL, 10);
  //struct to define soft and hard limits
  struct rlimit limitMem;
  limitMem.rlim_max = (rlim_t) memLimit*1024*1024;
  limitMem.rlim_cur = (rlim_t) memLimit*1024*1024;

  //RLIMIT_DATA: max size of the process's data segment
  if(setrlimit(RLIMIT_DATA, &limitMem) != 0){
    printf("Error during setting mem limit");
    return -1;
  }
  return 0;
}

void executeCommands(char* filename, char* tmLimit, char* mbLimit){
  FILE* file = fopen(filename, "r");
  if(file == NULL){
    printf("Error during opening file with comannds.\n");
    exit(0);
  }

  struct rusage usage1;
  //rusage children return resource usage statistics for all children
  //of the calling process
  getrusage(RUSAGE_CHILDREN, &usage1);
  char linesBuffor[maxLines], *parametersBuffor[maxArgs];
  int argIter = 0;
  //fgets need: string buffer, snumber of elements and stdin
  while(fgets(linesBuffor, maxLines, file)){
    argIter = 0;
    //strtok take current string and go to the next
    while((parametersBuffor[argIter] = strtok(argIter == 0 ? linesBuffor : NULL, " \n\t")) != NULL){
      argIter++;
      if(argIter >= maxArgs){
        printf("You set up too many arguments in file!\n");
      }
    }
    printf("Current pid is: %d, parent pid: %d\n",getpid(),getppid());
    pid_t pid = fork();
    if(pid == 0){
      printf("Current pid is: %d, parent pid: %d\n",getpid(),getppid()););
      softAndHardLimits(tmLimit, mbLimit);
      execvp(parametersBuffor[0], parametersBuffor);
      exit(1);
    }
    int status;
    wait(&status);
    if(status){
      printf("Error while running: %s\n", parametersBuffor[0]);
      if(strcmp(parametersBuffor[0],"./testMem") == 0) printf("Error conected with exceeded memory limit!\n");
      else if(strcmp(parametersBuffor[0],"./testT") == 0) printf("Error conected with exceeded time limit!\n");
    }
    struct rusage usage2;
    getrusage(RUSAGE_CHILDREN, &usage2);
    struct timeval ru_utime;
    struct timeval ru_stime;

    timersub(&usage2.ru_utime, &usage1.ru_utime, &ru_utime);
    timersub(&usage2.ru_stime, &usage1.ru_stime, &ru_stime);
    usage1 = usage2;
    /*for(int i=0; i<argIter; i++){
      printf("%s", parametersBuffor[i]);
    }*/
    printf("User CPU time used: %d.%d seconds, system CPU time used: %d.%d seconds,\n\n"
          ,(int) ru_utime.tv_sec, (int) ru_utime.tv_usec,(int) ru_stime.tv_sec,(int) ru_stime.tv_usec);
  }
  fclose(file);
}

int main(int argc, char *argv[]){
  if(argc < 4){
    printf("You have to set up 3 arguments!\nWe need name of file, time limit[seconds] and memory limit[MB].\n");
    return 1;
  }
  executeCommands(argv[1], argv[2], argv[3]);
  return 0;
}
