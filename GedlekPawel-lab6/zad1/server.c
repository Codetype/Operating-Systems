#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>

#include "central.h"

int serverqueue;
void err_msg(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(1);
}
void snd_err(const char* msgtext, int msgqid){
    struct msgbuf msg;
    msg.mtype = ERROR;
    sprintf(msg.mtext, "%s", msgtext);
    msgsnd(msgqid, &msg, maxMsg, 0);
}
void myexit(void){msgctl(serverqueue, IPC_RMID, NULL);}
void sigIntHandler(int signo){
    if (signo == SIGINT)
    {
        printf("\nShutting down connection with server service\n");
        exit(0);
    }
}
void setSigINT(){
    struct sigaction sigact;
    sigact.sa_handler = sigIntHandler;
    sigfillset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL) < -1) err_msg("Error connected with SIGINT signal");
}
void doINIT(int *clients, struct msgbuf* msg){
    int i, tmp;
    tmp = atoi(msg->mtext);
    for (i = 0; i < maxClientsAmnt && clients[i] != -1; i++) {;}
    if (i < maxClientsAmnt)
    {
        clients[i] = tmp;
        msg->mtype = REPLY;
        sprintf(msg->mtext, "%i", i);
        if (msgsnd(clients[i], msg, maxMsg, 0) < 0)
        {
            perror("Error with sending INIT message.");
            clients[i] = -1;
            return;
        }
        printf("Client no: %i registred, PID: %i\n", i, msg->clientPID);
    }
    else
    {
        msg->mtype = ERROR;
        sprintf(msg->mtext, "Queue is overflow.");
        if (msgsnd(tmp, msg, maxMsg, 0)) perror("Error with sending INIT message.");
    }
}
void doSTOP(int *clients, struct msgbuf* msg){
    if (msg->clientID < 0 || msg->clientID > maxClientsAmnt) return;
    printf("Client no: %i removed, PID: %i\n", msg->clientID, msg->clientPID);
    clients[msg->clientID] = -1;
}
void doMIRROR(int client_queue_id, char* msg_text){
    char *str1 = msg_text;
    char *str2 = msg_text + strlen(msg_text) - 1;

    while (str1 < str2) {
        char tmp = *str1;
        *str1++ = *str2;
        *str2-- = tmp;
    }
    struct msgbuf msg;
    msg.mtype = REPLY;
    sprintf(msg.mtext, "%s", msg_text);
    if (msgsnd(client_queue_id, &msg, maxMsg, 0) < 0) perror("Error with sending message to server.");
};
void doCALC(int clqid, char* msg, int len){
    char operator, tmp[10];
    int ind = 0, num1, num2, result;
    //get first number
    while (msg[ind] > '0' && msg[ind] <= '9' && ind < len) ind++;
    if (ind == 0) {
      snd_err("Incorrect syntax message", clqid);
      return;
    }
    sprintf(tmp, "%.*s", (ind > 10) ? ind : 10, msg);
    num1 = atoi(tmp);
    //get operator
    while (msg[ind] == ' ' && ind < len) ind++;
    if (ind == len) {
      snd_err("Incorrect syntax message", clqid);
      return;
    }
    operator = msg[ind++];
    //get second number
    while (msg[ind] == ' ' && ind < len) ind++;
    if (ind == len || msg[ind] < '0' || msg[ind] > '9') {
      snd_err("Incorrect syntax message", clqid);
      return;
    }
    sprintf(tmp, "%.*s", (len-ind > 10) ? len-ind : 10, msg + ind);
    num2 = atoi(tmp);
    switch (operator)
    {
        case '+': result = num1 + num2; break;
        case '-': result = num1 - num2; break;
        case '*': result = num1 * num2; break;
        case '/':
          if (num2) result = num1 / num2;
          else {
            snd_err("Dividing by zero!", clqid);
            return;
          } break;
        default: snd_err("Incorrect syntax message", clqid); return;
    }
    struct msgbuf msgs;
    msgs.mtype = REPLY;
    sprintf(msgs.mtext, "%i", result);
    if (msgsnd(clqid, &msgs, maxMsg, 0) < 0) perror("Server send");
}
void doTIME(int client_queue_id){
    struct msgbuf msg;
    msg.mtype = REPLY;

    FILE *date = popen("date", "r");
    if (date == NULL || date < 0) perror("date");
    fgets(msg.mtext, maxMsgText, date);
    pclose(date);
    sprintf(msg.mtext, "%.*s", (int)strlen(msg.mtext)-1, msg.mtext);
    if (msgsnd(client_queue_id, &msg, maxMsg, 0) < 0) perror("Server send");
}


int main(int argc, char const *argv[]){
    const char *rqsts[4] = {"MIRROR", "CALC", "TIME", "END"};
    int i, break_flag = 0;
    struct msgbuf msg;
    int clients[maxClientsAmnt];
    for (i = 0; i < maxClientsAmnt; i++) clients[i] = -1;

    setSigINT();
    atexit(myexit);

    if ((serverqueue = msgget(ftok(getenv("HOME"), 0), IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO)) < 0) err_msg("Error with server's queue opening.");

    while(1)
    {
        if (msgrcv(serverqueue, &msg, maxMsg, 0, MSG_NOERROR) < 0) err_msg("Error with server msg received");
        if (msg.mtype < 5) printf("Received %s from #%i\n", rqsts[msg.mtype-1], msg.clientID);
        switch (msg.mtype){
            case INIT: doINIT(clients, &msg); break;
            case STOP: doSTOP(clients, &msg); break;
            case MIRROR: doMIRROR(clients[msg.clientID], msg.mtext); break;
            case CALC: doCALC(clients[msg.clientID], msg.mtext, strlen(msg.mtext)); break;
            case TIME: doTIME(clients[msg.clientID]); break;
            case END: break_flag = 1; break;
            default: printf("UNKNOWN request: %s\n", msg.mtext); break;
        }
        if (break_flag) break;
    }

    printf("Exiting from server service.\n");
    exit(0);
}
