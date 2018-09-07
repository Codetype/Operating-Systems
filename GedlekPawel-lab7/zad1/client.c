#include "barbershop.h"

int semaphore_set_id = -1;
int shmemory_id = -1;
void *memory_address;
int cut_counter = 0;
int cut_required = 2;

void sigint_handler(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Client received SIGINT, time to kill all clients\n");
    killpg(getgid(), SIGTERM);
    exit(0);
}

void close_func(){
  //to "delete" segment
  if(shmdt(memory_address) == -1){
    perror("shmdt close memory");
    exit(1);
  }
}
int trying_haircut() {
    decrease_semvar(semaphore_set_id, QueueSemNum);
    int in_bqueue;
    int barber_sem_val = semctl(semaphore_set_id, BarberSemNum, GETVAL);
    if(barber_sem_val == -1){
      perror("semctl trying haircut");
      exit(1);
    }
    if(barber_sem_val == 0) {
        printf("%zu, pid: %d\tTime to wake up barber.\n", get_time(), getpid());
        increase_semvar(semaphore_set_id, BarberSemNum);
        increase_semvar(semaphore_set_id, BarberSemNum);
        set_actual_client(memory_address, getpid());
        in_bqueue = 0;
    } else {
        if(bqueue_put(memory_address, getpid()) == -1) {
            printf("%zu, pid: %d\tQueue overflow, returning...\n", get_time(), getpid());
            in_bqueue = -1;
        } else {
            printf("%zu, pid %d\tBarber is busy now. You have to wait.\n", get_time(), getpid());
            in_bqueue = 0;
        }
    }

    increase_semvar(semaphore_set_id, QueueSemNum);
    return in_bqueue;
}
void get_haircut() {
    while(cut_counter < cut_required) {
        int in_bqueue = trying_haircut();

        if (in_bqueue == 0) {
            decrease_semvar(semaphore_set_id, ChairSemNum);
            ++cut_counter;
            printf("%zu\t Client %d is having hair cut.\n", get_time(), getpid());
        }
    }
}
void set_clients_queue(int clientsNumber) {
    int clientsCounter = 0, status;

    for(int i = 0; i < clientsNumber; i++) {
        if(fork() == 0) {
            get_haircut();
            _exit(0);
        }
    }

    while(clientsCounter < clientsNumber) {
        wait(&status);
        clientsCounter++;

        if(status == 0) {
           printf("Client's hair cutted succesfully :)\n");
        }
        else {
            perror("status error\n");
            exit(0);
        }
    }
}

pid_t *set_shared_memory(){
    //generate specyfic ipc_key
    key_t ipc_key = ftok(getenv("HOME"), IPCGrKey);
    //get exists set of semaphores
    semaphore_set_id = semget(ipc_key, 0, 0);
    if(semaphore_set_id == -1){
         perror("semget configure");
         exit(1);
    }
    //to get exists id shared memory
    shmemory_id = shmget(ipc_key, 0, 0);
    if(shmemory_id == -1){
        perror("shmget configure");
        exit(1);
    }
    //address, where add new memory segment, thanks to NULL, default momory address
    memory_address = shmat(shmemory_id, NULL, 0);
    if(memory_address == (void *) -1) {
        perror("shmat configure");
        exit(1);
    }

    pid_t *bqueue = (pid_t *) memory_address;
    return bqueue;
}

int main(){
    atexit(close_func);

    //catch SIGINT
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &sigint_handler;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &action, NULL) == -1)
        perror("sigaction SIGINT client");

    set_shared_memory();
    //set number of clients
    set_clients_queue(20);
    return 0;
}
