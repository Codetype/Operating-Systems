#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]){
  long int SIZE = strtol(argv[1], '\0', 10);
  srand(time(NULL));
  int *tmpArray = malloc(SIZE*sizeof(int));
  for(int i=0; i<SIZE; i++)
    tmpArray[i] = rand()%100;
  return 0;
}
