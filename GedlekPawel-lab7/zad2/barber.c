#include "barbershop.h"

const char b_name[] = "somestring";
int shared_memory_desc;
void *shared_mem_address;
sem_t *barber_sem, *queue_sem, *chair_sem;

void sigint_handler(int sig_number, siginfo_t *siginfo, void *foo){
    printf("Barber received SIGINT\n");
    exit(0);
}

pid_t *bqueue_init(int size){

    barber_sem = sem_open(BarberSemName, O_RDWR | O_CREAT | O_EXCL , 0666, 0);
    if(barber_sem == SEM_FAILED){
        printf("barber sem_open\n");
    }
    queue_sem = sem_open(QueueSemName, O_RDWR | O_CREAT | O_EXCL , 0666, 1);
    if(queue_sem == SEM_FAILED){
        printf("queue sem_open\n");
    }
    chair_sem = sem_open(ChairSemName, O_RDWR | O_CREAT | O_EXCL, 0666, 0);
    shared_memory_desc = shm_open(ShMemName, O_RDWR | O_CREAT, 0666);

    if(ftruncate(shared_memory_desc, size * sizeof(pid_t) + 3) == -1){
      perror("ftruncate");
      exit(1);
    }

    shared_mem_address = mmap(0, sizeof(pid_t *), PROT_READ | PROT_WRITE, MAP_SHARED, shared_memory_desc, 0);

    if(shared_mem_address == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    queue_size = size;
    pid_t *bqueue = (pid_t *) shared_mem_address;
    bqueue[ClientsCount] = 0;
    bqueue[ClientOnChair] = -1;
    bqueue[MaxSize] = size;

    return bqueue;
}

void close_func(){
    if(sem_close(barber_sem) == -1){
      perror("sem close barber");
      exit(1);
    }
    if(sem_close(queue_sem) == -1){
      perror("sem close queue");
      exit(1);
    }
    if(sem_close(chair_sem) == -1){
      perror("sem close chair");
      exit(1);
    }
    sem_unlink(BarberSemName);
    sem_unlink(QueueSemName);
    sem_unlink(ChairSemName);
    munmap(shared_mem_address, queue_size * sizeof(pid_t) + 3);
    shm_unlink(ShMemName);
}

void cutting_customer(pid_t client_pid){
    printf("%zu\t Barber is making now haircut for client: %d\n", get_time(), client_pid);
    printf("%zu\t Barber has already finished client: %d\n", get_time(), client_pid);
    increase_semvar(chair_sem);
}

int main(){
    atexit(close_func);

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
    printf("%zu\t Barber is sleeping zzz\n", get_time());

    while(1){
        decrease_semvar(barber_sem);
        decrease_semvar(queue_sem);
        client_on_chair = bqueue[ClientOnChair];
        increase_semvar(queue_sem);

        cutting_customer(client_on_chair);

        while(1){
            decrease_semvar(queue_sem);
            print_bqueue(bqueue);
            next_client = bqueue_get(bqueue);

            if(next_client == -1){
                printf("Barber fall asleep zzz\n");
                decrease_semvar(barber_sem);
                increase_semvar(queue_sem);
                break;
            }
            else {
                set_actual_client(bqueue ,next_client);
                client_on_chair = next_client;
                cutting_customer(client_on_chair);
                increase_semvar(queue_sem);
            }
        }
    }
}
