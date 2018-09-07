/*
* Paweł Gędłek
* LAB2 - ZAD1
* sysFOperations.c
*/
#include "sysFOperations.h"



void generateData_SYS(char *fileName, int recordsNumber, int recordSize) {
    int file = open(fileName, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    int randVal = open("/dev/urandom", O_RDONLY);
    char *buff = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {
        if (read(randVal, buff, (size_t) recordSize) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }
        for (int j = 0; j < recordSize; ++j) {
            buff[j] = (char) (abs(buff[j]) % 25 + 97);
        }
        buff[recordSize-1] = 10;
        if (write(file, buff, (size_t) recordSize) != recordSize) {
            printf("Error during writing occurs!");
            return;
        }
    }
    close(file);
    close(randVal);
    free(buff);
}

void copyDataF2F_SYS(char *fileName1, char *fileName2, int recordsNumber, int recordSize){
    int file2 = open(fileName2, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
    int file1 = open(fileName1, O_RDONLY);
    char *buffer = (char *) calloc((size_t) recordSize, sizeof(char));

    for (int i = 0; i < recordsNumber; ++i) {
        if (read(file1, buffer, (size_t) recordSize) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }
        if (write(file2, buffer, (size_t) recordSize) != recordSize) {
            printf("Error during writing occurs!");
            return;
        }
    }
    close(file1);
    close(file2);
}
void sort_file_sys(char *fileName, int recordsNumber, int recordSize){
    int trigger = open(fileName, O_RDWR);
    char* buff1 = calloc(recordSize,sizeof(char));
    char* buff2 = calloc(recordSize,sizeof(char));
    if(trigger){
        for(int i=1; i < recordsNumber; i++){
            lseek(trigger,i*recordSize,SEEK_SET);
            read(trigger,buff1,recordSize);
            int j=0;
            while(1){
                lseek(trigger,j*recordSize,SEEK_SET);
                read(trigger,buff2,recordSize);
                if(j>=i || buff1[0]<buff2[0]){
                    break;
                }
                j++;
            }

            lseek(trigger,-recordSize,SEEK_CUR);
            write(trigger,buff1,recordSize);
            for(int k = j+1; k < i+1; k++){
                lseek(trigger,k*recordSize,SEEK_SET);
                read(trigger,buff1,recordSize);
                lseek(trigger,-recordSize,SEEK_CUR);
                write(trigger,buff2,recordSize);
                strncpy(buff2,buff1,recordSize);
            }

        }
    }
    close(trigger);
}
void sortData_SYS(char *fileName, int recordsNumber, int recordSize){
    int fileDesc = open(fileName, O_RDWR, O_TRUNC);
    char* keyBuff = (char *) calloc((size_t) recordSize, sizeof(char));
    char* tmpBuff = (char *) calloc((size_t) recordSize, sizeof(char));
    long int offset = (long int) (recordSize * sizeof(char));
    int j;

    for (int i = 1; i < recordsNumber; ++i) {

        lseek(fileDesc, i*offset, SEEK_SET);

        if (read(fileDesc, keyBuff, (size_t) recordSize ) != recordSize){
            printf("Error during reading occurs!\n");
            return;
        }
        lseek(fileDesc, (-2) * offset, SEEK_CUR);
        if (read(fileDesc, tmpBuff, (size_t) recordSize ) != recordSize) {
            printf("Error during reading occurs!\n");
            return;
        }

        j = i ;

        while  ((int)keyBuff[0] < (int)tmpBuff[0] && j > 1 ){
            if (write(fileDesc, tmpBuff, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Error during writing occurs!");
                return;
            }
            lseek(fileDesc, (-3) * offset, SEEK_CUR);
            if (read(fileDesc, tmpBuff, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Error during writing occurs!");
                return;
            }
            j--;
        }

        if (keyBuff[0] < tmpBuff[0] && j == 1 ){
            if (write(fileDesc, tmpBuff, (size_t) recordSize * sizeof(char)) != recordSize) {
                printf("Error during writing occurs!");
                return;
            }
            lseek(fileDesc, (-2) * offset, SEEK_CUR);
        }

        if (write(fileDesc, keyBuff, (size_t) recordSize * sizeof(char)) != recordSize) {
            printf("Error during writing occurs!");
            return;
        }

    }
    close(fileDesc);
    free(tmpBuff);
    free(keyBuff);

}
