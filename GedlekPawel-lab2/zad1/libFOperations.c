/*
* Paweł Gędłek
* LAB2 - ZAD1
* libFOperations.c
*/
#include "libFOperations.h"

void generateData_LIB(char *fileName, int recordsNumber, int recordSize) {
    FILE *file = fopen(fileName, "w+");
    FILE *randomVal = fopen("/dev/urandom", "r");
    char *buff = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {
        /*
        * fread  & fwrite return number of records, when we gain recordsize, we break
        */
        if (fread(buff, sizeof(char), (size_t) recordSize, randomVal) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }

        for (int j = 0; j < recordSize; ++j) {
            buff[j] = (char) (abs(buff[j]) % 25 + 97);
        }
        buff[recordSize-1] = 10;

        if (fwrite(buff, sizeof(char), (size_t) recordSize, file) != recordSize) {
            printf("Error during writing occurs!\n");
            return;
        }
    }
    fclose(file);
    fclose(randomVal);
    free(buff);
}

void copyDataF2F_LIB(char *fileName1, char *fileName2, int recordsNumber, int recordSize){
    FILE *file1 = fopen(fileName1, "r");
    FILE *file2 = fopen(fileName2, "w+");
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {

        if (fread(buffer, sizeof(char), (size_t) recordSize, file1) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }

        if (fwrite(buffer, sizeof(char), (size_t) recordSize, file2) != recordSize) {
            printf("Error during writing occurs!\n");
            return;
        }
    }
    free(buffer);
    fclose(file1);
    fclose(file2);
}
void sort_file(char *fileName, int recordsNumber, int recordSize){
    FILE* trigger = fopen(fileName,"r+");
    char* buff1 = calloc(recordSize,sizeof(char));
    char* buff2 = calloc(recordSize,sizeof(char));
    if(trigger){
        for(int i=1; i < recordsNumber; i++){
            fseek(trigger,i*recordSize,0);
            fread(buff1,sizeof(char),recordSize,trigger);
            int j=0;
            while(1){
                fseek(trigger,j*recordSize,0);
                fread(buff2,sizeof(char),recordSize,trigger);
                if(j>=i || buff1[0]<buff2[0]){
                    break;
                }
                j++;
            }

            fseek(trigger,-recordSize,1);
            fwrite(buff1,sizeof(char),recordSize,trigger);
            for(int k = j+1; k < i+1; k++){
                fseek(trigger,k*recordSize,0);
                fread(buff1,sizeof(char),recordSize,trigger);
                fseek(trigger,-recordSize,1);
                fwrite(buff2,sizeof(char),recordSize,trigger);
                strncpy(buff2,buff1,recordSize);
            }

        }
    }
    fclose(trigger);
}
void sortData_LIB(char *fileName, int recordsNumber, int recordSize){
    FILE *file = fopen(fileName, "r+");
    char* keyBuff = (char *) calloc((size_t) recordSize, sizeof(char));
    char* tmpBuff = (char *) calloc((size_t) recordSize, sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for (int i = 1; i < recordsNumber; ++i) {
        fseek(file, i*offset, 0);

        if (fread(keyBuff, sizeof(char), (size_t) recordSize, file) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }
        fseek(file, (-2) * offset, 1);
        if (fread(tmpBuff, sizeof(char), (size_t) recordSize, file) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }
        j = i ;
        while  ((int)keyBuff[0] < (int)tmpBuff[0] && j > 1 ){
            if (fwrite(tmpBuff, sizeof(char), (size_t) recordSize, file) != recordSize) {
                printf("Error during writing occurs!\n");
                return;
            }
            fseek(file, (-3) * offset, 1);
            if (fread(tmpBuff, sizeof(char), (size_t) recordSize, file) != recordSize){
                printf("Error during reading occurs!\n");
                return;
            }
            j--;
        }
        if (keyBuff[0] < tmpBuff[0] && j == 1 ){
            if (fwrite(tmpBuff, sizeof(char), (size_t) recordSize, file) != recordSize) {
                printf("Error during reading occurs!\n");
                return;
            }
            fseek(file, (-2) * offset, 1);
        }
        if (fwrite(keyBuff, sizeof(char), (size_t) recordSize, file) != recordSize) {
            printf("Error during reading occurs!\n");
            return;
        }
    }
    free(keyBuff);
    free(tmpBuff);
    fclose(file);
}
