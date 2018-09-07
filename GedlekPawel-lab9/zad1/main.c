/*
*
* Paweł Gędłek
* lab 9, zad 1
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

/*
* P - number of producers
* K - number of consumers
* N - size of buffer
* L - length of line
* srch_type: -1 - lesser than
              0 - equal
              1 - greater than
* prnt_type:  1 - print consumers and producers 
              0 - print searching lines
* nk:         >  0 - end of thread after nk seconds
              == 0 - end of thread after end of file
*/
int P, 
    K, 
    N, 
    L, 
    srch_type, 
    prnt_type, 
    nk;

FILE* file_buff;

char** buffer = NULL;
int write_ind = 0, 
    read_ind = 0, 
    count = 0;

//initiliaze mutex and conditions
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  empty_cond  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  full_cond   = PTHREAD_COND_INITIALIZER;

void sigexit(int sig){
    exit(1);
};

void exit_fun()
{
    if (buffer) free(buffer);
    if (file_buff) fclose(file_buff);
    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&empty_cond);
    pthread_cond_destroy(&full_cond);
}

void load_configuration(char* const config_file)
{
    FILE* conf;
    if ((conf = fopen(config_file, "r")) < 0){
        perror("opening file");
        exit(1);
    } 
    char buff[50];

    fscanf(conf, "%d", &P);
    fscanf(conf, "%d", &K);
    fscanf(conf, "%d", &N);

    fscanf(conf, "%s", buff);
    file_buff = fopen(buff, "r");
    if(file_buff == NULL){
        perror("open file error\n");
        exit(1);
    }
    fscanf(conf, "%d", &L);
    fscanf(conf, "%d", &srch_type);
    fscanf(conf, "%d", &prnt_type);
    fscanf(conf, "%d", &nk);

    if(fclose(conf) < 0) 
        perror("Configure file close");
}

void print_srch_line(char* buff, int ind)
{
    if (buff[strlen(buff)-1] == '\n') buff[strlen(buff)-1] = '\0';
    int str_len = strlen(buff);
    switch (srch_type)
    {
        case -1:    
            if (str_len < L) 
                printf("Index: %d, found string with length: %d < %d\t%s\n", ind, str_len, L, buff);  
        break;
        case 0:    
            if (str_len == L) 
                printf("Index: %d, found string with length: %d == %d\t%s\n", ind, str_len, L, buff);            
        break;
        case 1:  
            if (str_len > L) 
                printf("Index: %d, found string with length: %d > %d\t%s\n", ind, str_len, L, buff);  
        break;
    }
    
}

void* producer(void* arg)
{
    char* buff = NULL;
    size_t n = 0;
    while (1)
    {
        //waiting for free buffer
        pthread_mutex_lock(&count_mutex);

        while (count >= N){
            
            pthread_cond_wait(&full_cond, &count_mutex);
            
        }
        //add element to the buffer
        if (getline(&buff, &n, file_buff) <= 0)
        {            
            pthread_mutex_unlock(&count_mutex);
            break;
        }
        if (prnt_type == 1) 
            //printf("Producer no: %lu puts a line into %i \t%i/%i\n", pthread_self(), write_ind, count+1, N);
            printf("Producer puts a line into buff[%d] \tElements in buffer: %d\n", write_ind, count+1);
        buffer[write_ind] = buff;
        write_ind = (write_ind+1)%N;
        count++;

        //appear a new element
        pthread_cond_signal(&empty_cond);
        pthread_mutex_unlock(&count_mutex);

        n = 0;
        buff = NULL;
    }
    pthread_exit((void*) 0);
}

void* consumer(void* arg)
{
    char* buff;
    while (1)
    {
        //waiting for an element
        pthread_mutex_lock(&count_mutex);
        while(count <= 0){
            pthread_cond_wait(&empty_cond, &count_mutex);
        }
        buff = buffer[read_ind];
        buffer[read_ind] = NULL;

        if (prnt_type == 1) 
            //printf("consumer no: %lu reads a line from %d \t%d / %d\n", pthread_self(), read_ind, count-1, N);
            printf("Consumer reads a line from buff[%d] \tElements in buffer: %d\n", read_ind, count-1);
        else
            print_srch_line(buff, read_ind);
        //get element from buffer
        read_ind = (read_ind+1)%N;
        count--;

        //free place in buffer
        pthread_cond_signal(&full_cond);
        pthread_mutex_unlock(&count_mutex);
        
        free(buff);
    }
    
    pthread_exit((void*) 0);
}

int main(int argc, char *argv[])
{ 
    load_configuration("data.txt");
    atexit(exit_fun);
    signal(SIGINT, sigexit);

    pthread_t* prods = malloc(P * sizeof(pthread_t));
    pthread_t* cons = malloc(K * sizeof(pthread_t));
    int i;

    buffer = malloc(N * sizeof(char*));

    for (i = 0; i<P; i++){
        if (pthread_create(&prods[i], NULL, &producer, NULL)){
            perror("producer thread create");
            exit(1);
        }
    }
    for (i = 0; i<K; i++){
        if (pthread_create(&cons[i], NULL, &consumer, NULL)){
            perror("consumer thread create");
            exit(1);
        }
    }
    if (nk) alarm(nk);
    
    for (i = 0; i<P; i++) 
        if (pthread_join(prods[i], NULL)){ 
            perror("Thread prod join");
            exit(1);
        }
    
    //check is it okay
    while (1)
    {    
        pthread_mutex_lock(&count_mutex);
        if (count == 0) break;
        pthread_mutex_unlock(&count_mutex);
    }
    exit(0);
}
