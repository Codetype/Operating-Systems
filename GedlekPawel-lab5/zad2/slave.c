#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

int main(int argc, char** argv) {
    if (argc < 3)  return 3;
    int pid = getpid();

    int count = strtol(argv[2], NULL, 10);



    for (int i = 0; i < count; i++) {
        char dateString[200];
        char printString[200];
        fgets(dateString, 200, popen("date", "r"));
        sprintf(printString, "My pid is %d and the current date is %s", pid, dateString);
        FILE* file = fopen(argv[1], "w");
        fputs(printString,file);
        fclose(file);
        srand(time(NULL));
        sleep(rand()%1+1);
    }
    return 0;
}
