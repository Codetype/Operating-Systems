/*
* Paweł Gędłek
* LAB2 - ZAD1
* sysFOperations.h
*/
#ifndef LAB2_SYSTFILEOPERATION
#define LAB2_SYSTFILEOPERATION

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void generateData_SYS(char *filePath, int recordsNumber, int recordSize);

void copyDataF2F_SYS(char *sourceFileName, char *destFileName, int recordsNumber, int recordSize);

void sortData_SYS(char *filePath, int recordsNumber, int recordSize);

#endif
