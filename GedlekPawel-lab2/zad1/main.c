/*
* Paweł Gędłek
* LAB2 - ZAD1
* main.c
*/
#include "libFOperations.h"
#include "sysFOperations.h"

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <stdbool.h>

/*structure with pair repres. timeval*/
struct timeValues{
  struct timeval begin;
  struct timeval finish;
};

/*times we need in our tests*/
struct searchedTimes{
  struct timeValues userTime;
  struct timeValues sysTime;
  struct timeValues realTime;
  struct rusage newusage;
};

struct searchedTimes measureTime;
/*functions to measure time*/
void startMeasure(){
  getrusage(RUSAGE_SELF, &measureTime.newusage);
  measureTime.sysTime.begin = measureTime.newusage.ru_stime;
  measureTime.userTime.begin = measureTime.newusage.ru_utime;
  gettimeofday(&measureTime.realTime.begin,NULL);
}
void finishMeasure(){
  getrusage(RUSAGE_SELF, &measureTime.newusage);
  measureTime.sysTime.finish = measureTime.newusage.ru_stime;
  measureTime.userTime.finish = measureTime.newusage.ru_utime;
  gettimeofday(&measureTime.realTime.finish,NULL);
}
void printTime(){
printf("System time: %lf\nUser time: %lf\nReal time: %lf\n"
,(double)(measureTime.sysTime.finish.tv_sec)-(measureTime.sysTime.begin.tv_sec)+(double)(measureTime.sysTime.finish.tv_usec)*pow(10,-6)-(measureTime.sysTime.begin.tv_usec)*pow(10,-6)
,(double)(measureTime.userTime.finish.tv_sec)-(measureTime.userTime.begin.tv_sec)+(double)(measureTime.userTime.finish.tv_usec)*pow(10,-6)-(measureTime.userTime.begin.tv_usec)*pow(10,-6)
,(double)(measureTime.realTime.finish.tv_sec)-(measureTime.realTime.begin.tv_sec)+(double)(measureTime.realTime.finish.tv_usec)*pow(10,-6)-(measureTime.realTime.begin.tv_usec)*pow(10,-6)
);
}

/*
*
*Main function
*
*/

int main(int argc, char* argv[]){
  srand(time(NULL));
  if(argc < 4){
    printf("Too few arguments in command line!\nYou have to implement ./<program name> and at least 4 arguments\n"); return 1;
  }
  int numberOfRecords = 0, recordSize = 0;
  char* fnm1;
  char* fnm2;
  if(strcmp(argv[1], "generate") == 0){
      fnm1 = argv[2];
      numberOfRecords = (int) strtol(argv[3],'\0',10);
      recordSize = (int) strtol(argv[4],'\0',10);

      if(argc > 5){

        if(strcmp(argv[5], "sys") == 0){
          printf("System functions times:\n");
          startMeasure();

          generateData_SYS(fnm1, numberOfRecords, recordSize);

          finishMeasure();
          printTime();
        }
      }
      else{
        printf("Library functions times:\n");
        startMeasure();

        generateData_LIB(fnm1, numberOfRecords, recordSize);

        finishMeasure();
        printTime();
      }
  }
  else if(strcmp(argv[1], "sort") == 0){
    fnm1 = argv[2];
    numberOfRecords = (int) strtol(argv[3],'\0',10);
    recordSize = (int) strtol(argv[4],'\0',10);

    if(argc > 5){

      if(strcmp(argv[5], "sys") == 0){
        printf("System functions times:\n");
        startMeasure();

        sortData_SYS(fnm1, numberOfRecords, recordSize);

        finishMeasure();
        printTime();
      }
    }
    else{
      printf("Library functions times:\n");
      startMeasure();

      sortData_LIB(fnm1, numberOfRecords, recordSize);

      finishMeasure();
      printTime();
    }
  }
  else if(strcmp(argv[1], "copy") == 0){
    fnm1 = argv[2];
    fnm2 = argv[3];
    numberOfRecords = (int) strtol(argv[4],'\0',10);
    recordSize = (int) strtol(argv[5],'\0',10);

    if(argc > 6){
      if(strcmp(argv[6], "sys") == 0){
        printf("System functions times:\n");
        startMeasure();

        copyDataF2F_SYS(fnm1, fnm2, numberOfRecords, recordSize);

        finishMeasure();
        printTime();
      }
    }
    else{
      printf("Library functions times:\n");
      startMeasure();

      copyDataF2F_LIB(fnm1, fnm2, numberOfRecords, recordSize);

      finishMeasure();
      printTime();
    }
  }
  else{
      printf("Incorrect method. Available methods is: generate, sort, copy");
  }

  return 0;
}
