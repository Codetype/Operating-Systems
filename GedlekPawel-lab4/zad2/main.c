#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define to_parent_signal SIGRTMAX
#define to_child_signal SIGRTMAX-1


int chldNumbers;
int chldRequests;
int chldFinished;
int isCreate=1;
int isRequest=1;
int isApprovement=1;
int isRT_Request=1;
int isExit=1;
int created_number;
pid_t *created_processes;
int request_number;
pid_t *requested_processes;
int approvment_number;
pid_t *approvement_processes;
int rt_request_number;
pid_t *rt_requested_processes;
int exited_number;
pid_t *exited_processes;

int a=0;
int request_approved;

void ch_sig_usr2(int signum){
    if(isApprovement==1)
        printf("Child PID: %d received parent's approval\n", getpid());
    request_approved=1;
}
void ch_sig_quit(int signum){
    exit(1);
}
void set_child_handlers(){
    struct sigaction act1;
    sigfillset(&act1.sa_mask);
    act1.sa_handler=ch_sig_usr2;
    if(sigaction(to_child_signal, &act1, NULL)==-1){
        perror("Error during setting SIGUSR2 handler for child\n");
        exit(1);
    }
    struct sigaction act2;
    sigfillset(&act2.sa_mask);
    act2.sa_handler=ch_sig_quit;
    if(sigaction(SIGQUIT, &act2, NULL)==-1){
        perror("Error during setting SIGQUIT handler for child\n");
        exit(1);
    }
}
void create_child(){
    pid_t id=fork();
    if(id<0){
        perror("Error during fork() executing\n");
        exit(1);
    }
    else if (id==0){
        request_approved=0;
        set_child_handlers();
        srand(getpid());
        int sleep_time=rand()%11;
        sleep(sleep_time);
        union sigval value;
        sigset_t tmp_mask, old_mask;
        sigfillset(&tmp_mask);
        if(sigprocmask(SIG_BLOCK, &tmp_mask, &old_mask)==-1){
            perror("Error during setting sigmask.\n");
            exit(1);
        }
        if(sigqueue(getppid(), to_parent_signal, value)==-1){
            perror("Error during sending request to parent\n");
            exit(1);
        }
        if(isRequest==1) printf("Child %d: sent request to parent process \n", getpid());
        while(request_approved==0) sigsuspend(&old_mask);
        int random_signal = SIGRTMIN + rand()%((SIGRTMAX-1)-SIGRTMIN);
        if(sigqueue(getppid(), random_signal, value)==-1){
            printf("Error during sending SIGRT to parent.\n");
            exit(1);
        }
        if(isRT_Request==1)
            printf("Child %d: sent SIGRT %d to parent process %d\n", getpid(), random_signal-32, getppid());

        exit(1);
    }

    created_processes[created_number]=id;
    created_number++;

    if(isCreate==1)
        printf("Spawned child process with PID: %d\n", id);
}
void sigINTHandler(int signum){
    printf("Received signal SIGINT\n");
    for(int i=0; i<created_number; i++){
        int local_flag=0;
        for(int j=0; j<exited_number; i++){
            if(exited_processes[j]==created_processes[i])
                local_flag=1;
        }
        if(local_flag==0)
            kill(created_processes[i], SIGQUIT);
    }
    chldFinished=2;
}

void sigCHLDHandler(int signum){
    int status;
    pid_t id;

    while((id=waitpid(-1, &status, WNOHANG))>0){
        int w_status=WEXITSTATUS(status);
        exited_processes[exited_number]=id;
        exited_number++;

        if(isExit==1)
            printf("Child PID %d has been terminated with exit code %d\n", id, w_status);
    }

    if(exited_number==chldNumbers)
        chldFinished=1;

}

void sigUSR1Handler(int signum, siginfo_t *info, void *context){
    requested_processes[request_number]=info->si_pid;
    if(isRequest==1){
        printf("Parent received request from child PID %d\n", info->si_pid);
    }
    request_number++;

    if(request_number==chldRequests){
        for(int i=0; i<request_number; i++){
            kill(requested_processes[i], to_child_signal);
            approvement_processes[approvment_number]=requested_processes[i];
            approvment_number++;

            if(isApprovement==1)
                printf("Parent approve request from child PID %d\n", requested_processes[i]);
        }

    }
    else if(request_number>chldRequests){
        kill(requested_processes[request_number-1], to_child_signal);
        approvement_processes[approvment_number]=requested_processes[request_number-1];
        approvment_number++;

        if(isApprovement==1)
        printf("Parent approve request from child PID %d\n", requested_processes[request_number-1]);
    }

}

void sigRTHandler(int signum, siginfo_t *info, void *context){
    rt_requested_processes[rt_request_number]=info->si_pid;
    rt_request_number++;

    if(isRT_Request==1)
        printf("Parent received RT signal %d from child PID: %d\n", signum-32, info->si_pid);
}


void setHandlers(){
    struct sigaction sigactINT;
    sigactINT.sa_handler = sigINTHandler;
    sigfillset(&sigactINT.sa_mask);
    if(sigaction(SIGINT, &sigactINT, NULL)==-1){
        perror("Error during setting SIGINT handler\n");
        exit(1);
      }
    struct sigaction sigactCHLD;
    sigactCHLD.sa_handler=sigCHLDHandler;
    sigfillset(&sigactCHLD.sa_mask);
    sigactCHLD.sa_flags=SA_NOCLDSTOP;
    if(sigaction(SIGCHLD, &sigactCHLD, NULL)==-1){
        perror("Error during setting SIGCHLD handler\n");
        exit(1);
    }
    struct sigaction sigactUSR1;
    sigactUSR1.sa_sigaction=sigUSR1Handler;
    sigfillset(&sigactUSR1.sa_mask);
    sigactUSR1.sa_flags=SA_SIGINFO;
    if(sigaction(to_parent_signal, &sigactUSR1, NULL)==-1){
        perror("Error during setting SIGUSR1 handler\n");
        exit(1);
    }
    struct sigaction sigactRT;
    sigactRT.sa_sigaction=sigRTHandler;
    sigfillset(&sigactRT.sa_mask);
    sigactRT.sa_flags=SA_SIGINFO;
    for(int i=SIGRTMIN; i<SIGRTMAX-1; i++){
        if(sigaction(i, &sigactRT, NULL)==-1){
            perror("Error during setting SIGRT handler\n");
            exit(1);
        }
    }
}

void var_initialization(){
    chldFinished=0;

    created_processes=malloc(sizeof(pid_t)*chldNumbers);
    created_number=0;

    requested_processes=malloc(sizeof(pid_t)*chldNumbers);
    request_number=0;

    approvement_processes=malloc(sizeof(pid_t)*chldNumbers);
    approvment_number=0;

    rt_requested_processes=malloc(sizeof(pid_t)*chldNumbers);
    rt_request_number=0;

    exited_processes=malloc(sizeof(pid_t)*chldNumbers);
    exited_number=0;
}


int main(int argc, char** argv){
    if(argc !=3){
        perror("Wrong number of arguments");
        exit(1);
    }
    chldNumbers=(int)strtol(argv[1], NULL, 10);
    chldRequests=(int)strtol(argv[2], NULL, 10);
    if(chldRequests>chldNumbers){
        perror("Wrong value of arguments. argv[1] must be higher than argv[2].\n");
        exit(1);
    }
    var_initialization();
    setHandlers();
    sigset_t new_mask, old_mask;
    sigemptyset(&new_mask);
    sigprocmask(SIG_BLOCK, &new_mask, &old_mask);
    for(int i=0; i<chldNumbers; i++) create_child();
    while(chldFinished==0) sigsuspend(&old_mask);
}
