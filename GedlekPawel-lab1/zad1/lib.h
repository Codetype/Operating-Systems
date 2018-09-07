#ifndef zad1_lib
#define zad1_lib

typedef struct{
  char** charArray;
  int arraySize;
  int blockSize;
} libraryArray;

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

//functions to implement
void printLibraryArray(libraryArray* choosenArray);
int codeASCIIsum(char* charBlock);
int blockLength(char* charBlock);

libraryArray* createStaticArray(int sizeOfArray, int sizeOfBlock);
libraryArray* createDynamicArray(int sizeOfArray, int sizeOfBlock);
void deleteExistsArray(libraryArray* arrayToDelete);
void addNewBlock(libraryArray* choosenArray, int blockIndex, char* blockArray);
void deleteExistsBlock(libraryArray *choosenArray, int blockIndex);
char *findClosestBlock(libraryArray *choosenArray, int asciiValue);

#endif/* end of include guard:  */
