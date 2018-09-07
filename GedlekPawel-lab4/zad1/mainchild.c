/*
*
* Gedlek Pawel
* lab4, zad1
*
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


// Stores the id of child process
int progLoop;
pid_t childPID;
const char *scriptname = "./timer.sh";

// Forks and executes passed script in child process
void execSH() {
    pid_t pid = fork();
    if(pid < 0)
        perror("Error during executing fork()");

    else if(pid > 0)
        childPID = pid;
    else {
        // Ignores both signals that would be sent to process from shell if user typed keyboard signal keys
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);
        if(execlp(scriptname, scriptname, NULL) == -1) {
            perror("Error during executing script");
            exit(1);
        }
    }
}


// Function used for handling SIGINT
void signIntHandler(int signo) {
    if(progLoop == 1)
        kill(childPID, SIGKILL);
    printf("Odebrano sygnal SIGINT\n");
    exit(0);
}


// Function used for handling SIGTSTP
void signTstpHandler(int signo) {
    // In case handler was called from previous call to this handler
    if(progLoop == 0) {
        progLoop = 1;
        return;
    }

    // In case handler was called while processing main loop
    kill(childPID, SIGKILL);
    progLoop = 0;
    fprintf(stdout, "Oczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\n");

    sigset_t sigmask;
    sigfillset(&sigmask);
    sigdelset(&sigmask, SIGINT);
    sigdelset(&sigmask, SIGTSTP);
    sigsuspend(&sigmask); // Pauses until SIGTSTP or SIGINT occurs
}

int main(int argc, char** argv) {
    childPID = -1;

    // Setting handler for SIGINT
    if(signal(SIGINT, signIntHandler) == SIG_ERR) {
        perror("Error during setting SIGINT handler");
        exit(1);
    }
    // struct sigaction setup
    struct sigaction acttstp;
    acttstp.sa_handler = signTstpHandler;
    sigemptyset(&acttstp.sa_mask);
    acttstp.sa_flags = 0;

    // Setting handler for SIGTSTP
    if(sigaction(SIGTSTP, &acttstp, NULL) == -1) {
        perror("Error during setting SIGTSTP handler");
        exit(1);
    }

    progLoop = 1;

    while(1) {
        execSH();
        pause();
    }
}
