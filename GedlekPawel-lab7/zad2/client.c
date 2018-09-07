#include "barbershop.h"

int shmemory_id = -1;
void *memory_address;
int cut_counter = 0;
int cut_required = 3;
sem_t *barber_sem, *queue_sem, *chair_sem;
int shared_memory_desc;

pid_t *set_shared_memory(){
    barber_sem = sem_open(BarberSemName, O_RDWR);
    if(barber_sem == SEM_FAILED){
        printf("sem_open \n");
    }
    queue_sem = sem_open(QueueSemName, O_RDWR );

    chair_sem = sem_open(ChairSemName, O_RDWR );

    shared_memory_desc = shm_open(ShMemName, O_RDWR , 0666);

    memory_address = mmap(0,  sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_desc, 0);
    if(memory_address == MAP_FAILED) {
        perror("mmap's FAILED");
        exit(1);
    }

    if(memory_address == (void *) -1) {
        perror("mem address");
        exit(1);
    }

    pid_t *bqueue = (pid_t *) memory_address;
    return bqueue;
}

void sigint_handler(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Client received SIGINT, time to kill all clients\n");
    killpg(getgid(), SIGTERM);
    exit(0);
}

void close_func(){
    if(munmap(memory_address, queue_size * sizeof(pid_t) + 3) == -1){
      perror("munmap");
      exit(EXIT_FAILURE);
    }
}

int trying_haircut() {
    decrease_semvar(queue_sem);
    int in_bqueue;
    int barber_sem_val;
    if(sem_getvalue(barber_sem, &barber_sem_val) == -1){
      perror("sem_getvalue");
      exit(1);
    }
    if(barber_sem_val == -1) {
      perror("sem_val error");
      exit(1);
    }

    if(barber_sem_val == 0) {
        printf("%zu\t pid: %d\tTime to wake up.\n", get_time(), getpid());
        increase_semvar(barber_sem);
        increase_semvar(barber_sem);
        set_actual_client(memory_address, getpid());
        in_bqueue = 0;
    } else {
        if(bqueue_put(memory_address, getpid()) == -1) {
            printf("%zu\t pid: %d\tQueue overflow, returning...\n", get_time(), getpid());
            in_bqueue = -1;
        } else {
            printf("%zu\t pid %d\tBarber is busy. You have to wait.\n", get_time(), getpid());
            in_bqueue = 0;
        }
    }

    increase_semvar(queue_sem);
    return in_bqueue;
}

void get_haircut() {
    while(cut_counter < cut_required) {
        int in_bqueue = trying_haircut();
        if (in_bqueue == 0) {
            decrease_semvar(chair_sem);
            ++cut_counter;
            printf("%zu\t Client: %d is having haircut now.\n", get_time(), getpid());
        }
    }
}

void set_clients_queue(int client_count) {
    int clientsCounter = 0, status;

    for(int i = 0; i < client_count; i++) {
        if(fork() == 0) {
            get_haircut();
            _exit(0);
        }
    }

    while(clientsCounter < client_count) {
        wait(&status);
        clientsCounter++;

        if(status == EXIT_SUCCESS) {
            printf("Client's hair cutted succesfully\n");
        }
        else {
            printf("status error\n");
            exit(0);
        }
    }
}


int main(){
    atexit(close_func);

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &sigint_handler;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &action, NULL) == -1){
      perror("sigaction client");
      exit(0);
    }
    set_shared_memory();

    set_clients_queue(20);
    return 0;

}
