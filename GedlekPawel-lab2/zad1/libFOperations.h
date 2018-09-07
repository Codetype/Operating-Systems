/*
* Paweł Gędłek
* LAB2 - ZAD1
* libFOperations.h
*/
#ifndef LAB2_LIBFILEOPERATION
#define LAB2_LIBFILEOPERATION

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>


void generateData_LIB(char *fileName, int recordsNumber, int recordSize);

void copyDataF2F_LIB(char *fileName1, char *fileName2, int recordsNumber, int recordSize);

void sortData_LIB(char *fileName, int recordsNumber, int recordSize);

#endif
