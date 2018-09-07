#include "barbershop.h"

void *shared_mem_address;

int bqueue_empty(pid_t *bqueue) {
    return (bqueue[ClientsCount] == 0) ? 1 : 0;
}

int bqueue_full(pid_t *bqueue) {
    return (bqueue[ClientsCount] == bqueue[MaxSize]) ? 1 : 0;
}

int bqueue_put(pid_t *bqueue, pid_t pid) {
    if (bqueue_full(bqueue)) {
        return -1;
    }

    bqueue[bqueue[ClientsCount] + QueueBegin] = pid;
    bqueue[ClientsCount] ++;
    return 0;
}

pid_t bqueue_get(pid_t *bqueue) {
    if (bqueue_empty(bqueue) == 1) {
        return -1;
    }

    pid_t next_client = bqueue[QueueBegin];

    for (int i=QueueBegin; i<bqueue[ClientsCount] + QueueBegin; i++) {
        bqueue[i] = bqueue[i+1];
    }

    bqueue[ClientsCount] -= 1;
    return next_client;
}

void set_actual_client(pid_t *bqueue, pid_t pid) {
    bqueue[ClientOnChair] = pid;
}

void print_bqueue(pid_t *bqueue){
    if(bqueue_empty(bqueue)){
        printf("Queue is empty\n");
        return;
    }
    printf("Actual client: %d\n", bqueue[ClientOnChair]);

    printf("Queue to haircut:\n");
    for (int i = QueueBegin; i < bqueue[ClientsCount] + QueueBegin; ++i) {
        printf("%d. %d\n",i - QueueBegin + 1, bqueue[i]);
    }

    printf("Queue max size - %d\tQueue current size - %d\n", bqueue[MaxSize], bqueue[ClientsCount]);

}

void increase_semvar(int sem_group_id, unsigned short sem_number) {
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_number;
    sem_buffer.sem_op = 1;
    sem_buffer.sem_flg = 0;
    if(semop(sem_group_id, &sem_buffer, 1) == -1){
      perror("semop sem_val increase");
      exit(1);
    }
}

void decrease_semvar(int sem_group_id, unsigned short sem_number) {
    struct sembuf sem_buffer;
    sem_buffer.sem_num = sem_number;
    sem_buffer.sem_op = -1;
    sem_buffer.sem_flg = 0;
    if(semop(sem_group_id, &sem_buffer, 1) == -1){
      perror("semop sem_val decrease");
      exit(1);
    }
}

__time_t get_time() {
    struct timespec time;
    if(clock_gettime(CLOCK_MONOTONIC, &time) == -1)
      perror("gettime");

    return time.tv_nsec / 1000;
}
