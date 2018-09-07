//include my own libraries
#include "lib.h"
//include standard C libraries

const int staticSize = 100;

void printLibraryArray(libraryArray* choosenArray){
  for(int i=0; i<choosenArray->arraySize; i++){
    if(choosenArray->charArray[i] == NULL) printf("NULL\n");
    else{
      printf("el: %s\tASCII: %d\tLength: %d\n",choosenArray->charArray[i], codeASCIIsum(choosenArray->charArray[i]), blockLength(choosenArray->charArray[i]));
    }
  }
}

int codeASCIIsum(char* charBlock){
  int charsSum = 0;
  if(charBlock != NULL){
    for(int i=0; charBlock[i] != '\0'; i++){
      charsSum += (int)charBlock[i];
    }
  }
  return charsSum;
}

int blockLength(char* charBlock){
  int length = 0;
  for(int i=0; charBlock[i] != '\0'; i++){
    length += 1;
  }
  return length;
}

libraryArray* createStaticArray(int sizeOfArray, int sizeOfBlock){
  if(sizeOfArray <= 0){
    printf("Size of array must be greater than zero!\n");
    return NULL;
  }
  libraryArray* newArray = calloc(1, sizeof(libraryArray));
  char stArray[staticSize][staticSize];
  newArray->charArray = (char **) stArray;
  newArray->arraySize = sizeOfArray;
  newArray->blockSize = sizeOfBlock;

  return newArray;
}

libraryArray* createDynamicArray(int sizeOfArray, int sizeOfBlock){
  if(sizeOfArray <= 0){
    printf("Size of array must be greater than zero!\n");
    return NULL;
  }
  libraryArray* newArray = calloc(1, sizeof(libraryArray));
  newArray->charArray = calloc((size_t)sizeOfArray, sizeof(char *));
  newArray->arraySize = sizeOfArray;
  newArray->blockSize = sizeOfBlock;

  return newArray;
}

void deleteExistsArray(libraryArray* arrayToDelete){
  for(int i=0; i<arrayToDelete->arraySize; i++){
    free(arrayToDelete->charArray[i]);
  }
}

void addNewBlock(libraryArray* choosenArray, int blockIndex, char* blockArray){
  if(blockIndex >= choosenArray->arraySize){
    printf("Index out of bounds!\n");
  } else if(blockLength(blockArray) > choosenArray->blockSize){
    printf("String is too long for this block!\nThe limit is: %d\n", choosenArray->blockSize);
  } else{
    if(choosenArray->charArray[blockIndex] != NULL) deleteExistsBlock(choosenArray, blockIndex);
    choosenArray->charArray[blockIndex] = (char*) calloc(choosenArray->blockSize, sizeof(char));
    strcpy(choosenArray->charArray[blockIndex],blockArray);
  }
}

void deleteExistsBlock(libraryArray *choosenArray, int blockIndex){
  free(choosenArray->charArray[blockIndex]);
  choosenArray->charArray[blockIndex] = NULL;
}

char *findClosestBlock(libraryArray *choosenArray, int asciiValue){
  int minDifference = INT_MAX;
  char *result = NULL;
  for(int i=0; i < choosenArray->arraySize; i++){
    char *blk = choosenArray->charArray[i];
    if(blk != NULL){
      int tmp = codeASCIIsum(choosenArray->charArray[i]);
      if(abs(asciiValue-tmp) < minDifference){
        minDifference = abs(asciiValue-tmp);
        result = choosenArray->charArray[i];
      }
    }
  }
  return result;
}
