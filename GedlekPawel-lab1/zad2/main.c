
#include "lib.h"

#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/resource.h>

double timeDifference(clock_t str, clock_t end){
  return (double)(end-str) / CLOCKS_PER_SEC;
}

double timeDiff(struct timeval time1, struct timeval time2){
  long tsecs = time2.tv_sec - time1.tv_sec;
  long tmsecs = time2.tv_usec - time1.tv_usec;
  return tsecs+tmsecs;
}

void timeTest1(libraryArray *Array, int numOfElems, int blkSize, bool isStatic){
  struct timeval startSysTime, startUserTime, endSysTime, endUserTime;
  struct rusage usage1st;
  getrusage(RUSAGE_SELF, &usage1st);

  clock_t startRealtime = clock();
  startSysTime = usage1st.ru_stime;
  startUserTime = usage1st.ru_utime;

  if(isStatic == true) Array = createStaticArray(numOfElems, blkSize);
  else Array = createDynamicArray(numOfElems, blkSize);

  fillRandomValues(Array, 0, numOfElems, blkSize);
  free(Array);

  getrusage(RUSAGE_SELF, &usage1st);
  clock_t endRealtime = clock();

  endSysTime = usage1st.ru_stime;
  endUserTime = usage1st.ru_utime;

  printf("Real time is: %lf\n", timeDifference(startRealtime, endRealtime));
  printf("System time is: %lf\n", timeDiff(startSysTime, endSysTime));
  printf("User time is: %lf\n", timeDiff(startUserTime, endUserTime));
}

void timeTest2(libraryArray *Array, int searchedSum){
  struct timeval startSysTime, startUserTime, endSysTime, endUserTime;
  struct rusage usage1st;
  getrusage(RUSAGE_SELF, &usage1st);

  clock_t startRealtime = clock();
  startSysTime = usage1st.ru_stime;
  startUserTime = usage1st.ru_utime;

  findClosestBlock(Array, searchedSum);

  getrusage(RUSAGE_SELF, &usage1st);
  clock_t endRealtime = clock();

  endSysTime = usage1st.ru_stime;
  endUserTime = usage1st.ru_utime;

  printf("Real time is: %lf\n", timeDifference(startRealtime, endRealtime));
  printf("System time is: %lf\n", timeDiff(startSysTime, endSysTime));
  printf("User time is: %lf\n", timeDiff(startUserTime, endUserTime));
}
void timeTest3(libraryArray *Array, int blockSize, int indx_s, int indx_e){
  struct timeval startSysTime, startUserTime, endSysTime, endUserTime;
  struct rusage usage1st;
  getrusage(RUSAGE_SELF, &usage1st);

  clock_t startRealtime = clock();
  startSysTime = usage1st.ru_stime;
  startUserTime = usage1st.ru_utime;

  for(int i=indx_s; i<indx_e; i++){
    deleteExistsBlock(Array, i);
  }
  fillRandomValues(Array, indx_s, indx_e, blockSize);

  getrusage(RUSAGE_SELF, &usage1st);
  clock_t endRealtime = clock();

  endSysTime = usage1st.ru_stime;
  endUserTime = usage1st.ru_utime;

  printf("Real time is: %lf\n", timeDifference(startRealtime, endRealtime));
  printf("System time is: %lf\n", timeDiff(startSysTime, endSysTime));
  printf("User time is: %lf\n", timeDiff(startUserTime, endUserTime));
}

void timeTest4(libraryArray *Array, int blockSize, int indx_s, int indx_e){
  struct timeval startSysTime, startUserTime, endSysTime, endUserTime;
  struct rusage usage1st;
  getrusage(RUSAGE_SELF, &usage1st);

  clock_t startRealtime = clock();
  startSysTime = usage1st.ru_stime;
  startUserTime = usage1st.ru_utime;

  for(int i=indx_s; i<indx_e; i++){
    deleteExistsBlock(Array,i);
    fillRandomValues(Array, i, i+1, blockSize);
  }

  getrusage(RUSAGE_SELF, &usage1st);
  clock_t endRealtime = clock();

  endSysTime = usage1st.ru_stime;
  endUserTime = usage1st.ru_utime;

  printf("Real time is: %lf\n", timeDifference(startRealtime, endRealtime));
  printf("System time is: %lf\n", timeDiff(startSysTime, endSysTime));
  printf("User time is: %lf\n", timeDiff(startUserTime, endUserTime));
}

int main(int argc, char* argv[]){
  srand(time(NULL));
  if(argc < 4){
    printf("Too few arguments!\nYou have to implement at least 3 arguments.\n");
    return 0;
  }
  //arguments on program start
  int numberOfElements = (int) strtol(argv[1],'\0',10);
  int blockSize = (int) strtol(argv[2],'\0',10);
  bool staticArray = (bool) strtol(argv[3],'\0',10);

  libraryArray *newArray = NULL;

  for(int i=4; i<argc; i++){
    if(strcmp(argv[i], "-crt") == 0){
      if(staticArray == true) newArray = createStaticArray(numberOfElements, blockSize);
      else newArray = createDynamicArray(numberOfElements, blockSize);
      fillRandomValues(newArray, 0, numberOfElements, blockSize);
    } else if(strcmp(argv[i], "-del") == 0) deleteExistsArray(newArray);
    else if(strcmp(argv[i], "-addblks") == 0){
      if(i+2 < argc){
          i++; int tmp_start = (int) strtol(argv[i],'\0',10);
          i++; int tmp_end = (int) strtol(argv[i],'\0',10);
          fillRandomValues(newArray, tmp_start, tmp_end, blockSize);
      } else printf("Function needs two arguments!\nThe start & end index to add element.\n");
    } else if(strcmp(argv[i], "-delblks") == 0){
      if(i+2 < argc){
          i++; int tmp_start = (int) strtol(argv[i],'\0',10);
          i++; int tmp_end = (int) strtol(argv[i],'\0',10);
          for(int i=tmp_start; i<tmp_end; i++){
            deleteExistsBlock(newArray, i);
          }
      } else if(strcmp(argv[i], "-fndblk") == 0){
        if(i+1 < argc){
        i++; int tmp = (int) strtol(argv[i],'\0',10);
        char *res = findClosestBlock(newArray, tmp);
        printf("%s", res);
      } else printf("Function needs one argument!\nThe searched ASCII value\n");
    }
    else printf("Avaliable methods:\n-crt = create new array,\n-del = delete exists array,\n"
    "-addblks start_ind end_ind = add blocks need two arg-s start & end index,\n"
    "-delblks start_ind end_ind = delete blocks need two arg-s start & end index,\n"
    "-fndblk value = find block with closest value of ascii to given,\n");
  }

  //time tests
  int choice = 0; libraryArray *tmpArray = NULL;
  switch(choice){
    case 1: timeTest1(tmpArray, numberOfElements, blockSize, staticArray); free(tmpArray); break;
    case 2: timeTest2(newArray, 1000); break;
    case 3: timeTest3(newArray, blockSize, 20, 320); break;
    case 4: timeTest4(newArray, blockSize, 20, 320); break;
    default: break;
  }
  free(newArray);
  //printLibraryArray(newArray);


  return 0;
}
