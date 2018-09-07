#ifndef BARBERSHOP_POSIX_H
#define BARBERSHOP_POSIX_H

#include <signal.h>
#include <zconf.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/sem.h>
#include <memory.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <wait.h>
#include <time.h>

#define MaxSize 0
#define ClientsCount 1
#define ClientOnChair 2
#define QueueBegin 3
#define BarberSemName "/barber_sem"
#define QueueSemName "/queue_sem"
#define ChairSemName "/chair_sem"
#define ShMemName "/bqueue_shared"

int bqueue_empty(pid_t *bqueue);
int bqueue_full(pid_t *bqueue);
int bqueue_put(pid_t *queue, pid_t pid);
pid_t bqueue_get(pid_t *bqueue);
void set_actual_client(pid_t *bqueue, pid_t pid);
void print_bqueue(pid_t *bqueue);
void increase_semvar(sem_t* sem);
void decrease_semvar(sem_t* sem);
__time_t get_time();
int queue_size;

#endif
