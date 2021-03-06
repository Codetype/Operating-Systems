#include <stdio.h>
#include <zconf.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <memory.h>

pid_t childProc;
int signalsReceivedFromMother = 0;
int signalsReceivedFromChild = 0;
int L = 10;
int confirmed = 0;
int type = 1;

static void usr2Action(int sigNum, siginfo_t* info, void* vp){
    char buffer[512];
    sprintf(buffer, "\nChild process received SIGUSR2. Time to exit\n");
    write(1, buffer, strlen(buffer));
    exit(0);
}

static void usr1Action1(int sigNum, siginfo_t* info, void* vp){
    char buffer[512];

    if (childProc == 0){
        signalsReceivedFromMother ++;
        sprintf(buffer, "Child process: %d\tReceived signal no.: %d\tNumber of received signal from parent: %d\n", getpid(), sigNum, signalsReceivedFromMother);
        write(1, buffer, strlen(buffer));

    } else {
        signalsReceivedFromChild ++;
        sprintf(buffer, "Main process received signal no.: %d\tFrom PID: %d\tNumber of received signals from child: %d\n", sigNum, info->si_pid, signalsReceivedFromChild);
        write(1, buffer, strlen(buffer));
    }
}

static void usr1Action2(int sigNum, siginfo_t* info, void* vp){
    char buffer[512];


    if (childProc == 0){
        signalsReceivedFromMother ++;
        sprintf(buffer, "Child PID: %d\tReceived signal no.: %d\tNumber of received signals from parent: %d\n", getpid(), sigNum, signalsReceivedFromMother);
        write(1, buffer, strlen(buffer));
        kill(info->si_pid, SIGUSR1);

    } else if (signalsReceivedFromMother < L) {
        signalsReceivedFromChild ++;
        sprintf(buffer, "Parent process confirmed receiving!\t\tNumber of received signals from child: %d\n", signalsReceivedFromChild);
        write(1, buffer, strlen(buffer));
        confirmed = 1;
    } else{
        sprintf(buffer, "Parent process received signal no: %d\tFrom PID: %d\n", sigNum, info->si_pid);
        write(1, buffer, strlen(buffer));
    }
}

static void usr1Action3(int sigNum, siginfo_t* info, void* vp){
    char buffer[512];

    if (childProc == 0){
        signalsReceivedFromMother ++;
        sprintf(buffer, "Child PID: %d\tReceived signal no.:%d\tNumber of received signals from parent: %d\n", getpid(), sigNum, signalsReceivedFromMother);
        write(1, buffer, strlen(buffer));

    } else {
        sprintf(buffer, "Parent process received signal no.: %d\tFrom PID: %d\n", sigNum, info->si_pid);
        write(1, buffer, strlen(buffer));
    }
}

static void intAction(int sigNum, siginfo_t* info, void* vp){
    if (childProc != 0){
        kill(childProc, SIGINT);
    }
    exit(0);
}



int main(int argc, char **argv) {
  int i;
    char buffer[512];
    if (argc != 3){
        printf("Wrong number of arguments!\n");
        exit(1);
    }

    L = (int) strtol(argv[1], NULL, 10);
    type = (int) strtol(argv[2], NULL, 10);

    struct sigaction sigAction;
    sigfillset(&sigAction.sa_mask);
    sigAction.sa_flags = SA_SIGINFO;

    sigAction.sa_sigaction = &intAction;
    sigaction(SIGINT, &sigAction, NULL);

    int status;

    if (type == 1){

        sigAction.sa_sigaction = &usr1Action1;
        sigaction(SIGUSR1, &sigAction, NULL);

        sigAction.sa_sigaction = &usr2Action;
        sigaction(SIGUSR2, &sigAction, NULL);


        childProc = fork();

        if (childProc != 0) {
            for (i = 0; i < L; ++i) {
                kill(childProc, SIGUSR1);
                sprintf(buffer, "Send %d'th signal SIGUSR1 to child PID: %d\n",i+1 ,childProc);
                write(1, buffer, strlen(buffer));
            }

            kill(childProc, SIGUSR2);
            wait(&status);
            printf("Child proces exited with status:%d\n", status);
            exit(0);
        }

        if (childProc == 0) {
            while (signalsReceivedFromMother < L) pause();
            for (i = 0; i < L; ++i) {
                kill(getppid(), SIGUSR1);
                sprintf(buffer, "Send %d'th signal SIGUSR1 to parent.\n", i+1);
                write(1, buffer, strlen(buffer));
            }
            while (1) pause();
        }
    }


    if (type == 2) {
        sigAction.sa_sigaction = &usr1Action2;
        sigaction(SIGUSR1, &sigAction, NULL);

        sigAction.sa_sigaction = &usr2Action;
        sigaction(SIGUSR2, &sigAction, NULL);

        childProc = fork();

    if (childProc != 0) {                                                                // TYPE == 2
        for (i = 0; i < L; ++i) {
            kill(childProc, SIGUSR1);
            sprintf(buffer, "Send %d'th signal SIGUSR1 to child PID: %d\n",i+1 ,childProc);
            write(1, buffer, strlen(buffer));
            while (confirmed == 0 && signalsReceivedFromChild < L) pause();
            confirmed = 0;
        }

        kill(childProc, SIGUSR2);
        wait(&status);
        printf("Child proces exited with status: %d\n", status);
        exit(EXIT_SUCCESS);
    }

    if (childProc == 0){
        while (1) pause();
    }

    }


    if (type == 3) {

        sigAction.sa_sigaction = &usr1Action3;
        sigaction(SIGRTMIN + 1, &sigAction, NULL);

        sigAction.sa_sigaction = &usr2Action;
        sigaction(SIGRTMIN + 2, &sigAction, NULL);

        childProc = fork();

        if (childProc != 0) {                                                                // TYPE == 3
            for (i = 0; i < L; ++i) {
                kill(childProc, SIGRTMIN + 1);
                sprintf(buffer, "Send %d'th signal SIGUSR1 to child PID: %d\n",i+1 ,childProc);
                write(1, buffer, strlen(buffer));
            }

            kill(childProc, SIGRTMIN + 2);
            wait(&status);
            printf("Child exit with status: %d\n", status);
            exit(0);
        }

        if (childProc == 0) {
            while (signalsReceivedFromMother < L) pause();
            for (i = 0; i < L; ++i) {
                kill(getppid(), SIGRTMIN + 1);
                sprintf(buffer, "Send %d'th signal SIGUSR1 to parent\n", i+1);
                write(1, buffer, strlen(buffer));
            }
            while (1) pause();
        }
    }


}
