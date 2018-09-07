#include "barbershop.h"

int sem_set_id = -1;
int shared_mem_id = -1;
void *shmemory_address;

void sigint_handler(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Barber received SIGINT\n");
    exit(0);
}

pid_t *bqueue_init(int size){
    key_t ipc_key = ftok(getenv("HOME"), IPCGrKey);

    sem_set_id = semget(ipc_key, GrSize, IPC_CREAT | 0600);
    if(sem_set_id == 1){
      perror("semget queue init");
      exit(1);
    }
    if(semctl(sem_set_id, BarberSemNum, SETVAL, 0) == -1){
      perror("semctl barber semaphore");
      exit(1);
    }
    if(semctl(sem_set_id, QueueSemNum, SETVAL, 1) == -1){
      perror("semctl queue semaphore");
      exit(1);
    }
    if(semctl(sem_set_id, ChairSemNum, SETVAL, 0) == -1){
      perror("semctl chair semaphore");
      exit(1);
    }

    shared_mem_id = shmget(ipc_key, size * sizeof(pid_t), IPC_CREAT | 0666);
    if(shared_mem_id == -1){
      perror("shmget");
      exit(1);
    }
    shmemory_address = shmat(shared_mem_id, NULL, 0);
    if(shmemory_address == (void *) -1) {
        perror("shmat shared_mem_id");
        exit(1);
    }

    pid_t *bqueue = (pid_t *) shmemory_address;
    bqueue[ClientsCount] = 0;
    bqueue[ClientOnChair] = -1;
    bqueue[MaxSize] = size;

    return bqueue;
}

void close_func(){
    shmdt(shmemory_address);
    //IPC_RMID let us delete memory adress from system
    shmctl(shared_mem_id, IPC_RMID, NULL);
    semctl(sem_set_id, 0, IPC_RMID);
}

void cutting_customer(pid_t client_pid){
    printf("%zu, Barber is making now haircut for client: %d\n", get_time(), client_pid);
    printf("%zu, Barber has already finished client: %d\n", get_time(), client_pid);
    increase_semvar(sem_set_id ,ChairSemNum);
}

int main(){
    atexit(close_func);
    //size of queue/waiting room
    int size = 10;

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = &sigint_handler;
    action.sa_flags = SA_SIGINFO;
    if(sigaction(SIGINT, &action, NULL) == -1){
      perror("sigaction barber");
      exit(1);
    }

    pid_t *bqueue = bqueue_init(size);

    pid_t client_on_chair;
    pid_t next_client;
    printf("%zu\t Barber is sleepeing zzz\n", get_time());

    while(1){
        decrease_semvar(sem_set_id, BarberSemNum);
        decrease_semvar(sem_set_id, QueueSemNum);
        client_on_chair = bqueue[ClientOnChair];
        increase_semvar(sem_set_id, QueueSemNum);

        cutting_customer(client_on_chair);

        while(1){
            decrease_semvar(sem_set_id, QueueSemNum);
            print_bqueue(bqueue);
            next_client = bqueue_get(bqueue);

            if(next_client == -1){
                printf("Barber fall asleep zzz\n");
                decrease_semvar(sem_set_id, BarberSemNum);
                increase_semvar(sem_set_id, QueueSemNum);
                break;
            }
            else {
                set_actual_client(bqueue ,next_client);
                client_on_chair = next_client;
                cutting_customer(client_on_chair);
                increase_semvar(sem_set_id, QueueSemNum);
            }
        }
    }
}
