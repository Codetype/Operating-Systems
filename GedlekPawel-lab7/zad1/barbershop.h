#ifndef BARBERSHOP_SYSTEMV_H
#define BARBERSHOP_SYSTEMV_H

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
#include <bits/time.h>
#include <time.h>
#include <wait.h>

#define IPCGrKey 1
#define GrSize 3
#define BarberSemNum 0
#define QueueSemNum 1
#define ChairSemNum 2
#define MaxSize 0
#define ClientsCount 1
#define ClientOnChair 2
#define QueueBegin 3

int bqueue_empty(pid_t *bqueue);
int bqueue_full(pid_t *bqueue);
int bqueue_put(pid_t *queue, pid_t pid);
pid_t bqueue_get(pid_t *bqueue);
void set_actual_client(pid_t *bqueue, pid_t pid);
void print_bqueue(pid_t *bqueue);
void increase_semvar(int sem_group_id, unsigned short sem_number);
void decrease_semvar(int sem_group_id, unsigned short sem_number);
__time_t get_time();

#endif
