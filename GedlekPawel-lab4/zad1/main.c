/*
*
* Gedlek Pawel
* lab4, zad1
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>


int signalno;


void signstpHandler(int signo){
  if(!signalno){
    signalno = 1;
  }
  else{
    signalno = 0;
    printf("Oczekuje na CTRL+Z - kontynuacja albo CTRL+C - zakończenie programu.\n");
  }
}

void signintHandler(int signo){
  printf("Odebrano sygnał SIGINT\n");
  exit(0);
}

int main(int argc, char *argv[]){
  if(signal(SIGINT, signintHandler) == SIG_ERR){
    printf("SIGINT error occurs!");
    exit(1);
  }
  struct sigaction sigact;
  sigact.sa_handler = signstpHandler;
  sigemptyset(&sigact.sa_mask); //help us specify signals blocked during executions of the handler
  sigact.sa_flags = 0; //to catch specific signal behaviour

  if(sigaction(SIGTSTP, &sigact, NULL) == -1){
    printf("SIGTSTP error occurs!");
    exit(1);
  }

  signalno = 1;

  while(1){
    if(signalno == 1){
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("Current Time : %s\n", time_str);

    sleep(1);
    } else{
        pause();
    }
  }

}
