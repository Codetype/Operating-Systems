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
int server;
int clientID = -1;
void myexit(){
    msgctl(serverqueue, IPC_RMID, NULL);
    if (clientID == -1) return;
    struct msgbuf msg;
    msg.mtype = STOP;
    msg.clientID = clientID;
    msg.clientPID = getpid();
    sprintf(msg.mtext, "%i", clientID);
    msgsnd(server, &msg, maxMsg, 0);
}
void err_msg(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(1);
}
void sigIntHandler(int signo){
    if (signo == SIGINT)
    {
        printf("\nShutting down connection with client service\n");
        exit(0);
    }
}
void setSigINT(){
    struct sigaction sigact;
    sigact.sa_handler = sigIntHandler;
    sigfillset(&sigact.sa_mask);
    sigact.sa_flags = 0;
    if (sigaction(SIGINT, &sigact, NULL) < -1) err_msg("Error connected with SIGINT signal.");
}
int cmdParser(char *line, int len, int* ind){
    int c = 0;
    int ret;
    char cmnd[6];

    while (line[c] != ' ' && c != len-1 && c < 6) c++;
    sprintf(cmnd, "%.*s", c, line);
    if (line[c] == ' ') while(line[c] == ' ') c++;
    *ind = c;

    if (strcmp(cmnd, "MIRROR") == 0) ret = MIRROR;
    else if (strcmp(cmnd, "CALC") == 0) ret = CALC;
    else if (strcmp(cmnd, "TIME") == 0) ret = TIME;
    else if (strcmp(cmnd, "END") == 0) ret = END;
    else if (strcmp(cmnd, "STOP") == 0) ret = STOP;
    else ret = UNKNOWN;

    return ret;
}
void connectClientToServer(){
    struct msgbuf msg;

    msg.mtype = INIT;
    msg.clientID = -1;
    msg.clientPID = getpid();
    sprintf(msg.mtext, "%i", serverqueue);
    if (msgsnd(server, &msg, maxMsg, 0) < 0) err_msg("Error with sending init message");

    if (msgrcv(serverqueue, &msg, maxMsg, 0, 0) < 0) err_msg("Error with receiving init message");
    switch(msg.mtype)
    {
        case REPLY: clientID = atoi(msg.mtext); if (clientID < 0) err_msg("Error with registering client."); break;
        case ERROR: err_msg(msg.mtext); break;
        default: break;
    }
}
void rcvReply(struct msgbuf* msg){
    if (msg->mtype == END) return;
    if (msgrcv(serverqueue, msg, maxMsg, 0, MSG_NOERROR) < 0) perror("Error with received message");
    printf("\nResult from server: %s\n", msg->mtext);
}
int main(int argc, char const *argv[])
{
    FILE *file;
    int intr = 1;
    if (argc > 1)
    {
        file = fopen(argv[1], "r");
        if (file == NULL) err_msg("Error connected with file containing commands.");
        intr = 0;
    }
    else file = stdin;

    setSigINT();
    atexit(myexit);

    char *buf = NULL;
    char line[100];
    size_t n;
    struct msgbuf msg;
    int count, pos, msg_id;

    if ((server = msgget(ftok(getenv("HOME"), 0), 0)) < 0) err_msg("Error with connection client->server");
    if ((serverqueue = msgget(IPC_PRIVATE, S_IRWXU | S_IRWXG | S_IRWXO)) < 0) err_msg("Error with connection server->client");

    connectClientToServer();
    while (printf("server $ ") < 0 || (count = getline(&buf, &n, file)) > 1)
    {
        sprintf(line, "%.*s", (buf[count-1] == '\n') ? count-1 : count, buf);
        msg_id = cmdParser(line, count, &pos);

        if (msg_id == UNKNOWN)
        {
            printf("Command UNKNOWN: %s\n", line);
            continue;
        }

        msg.mtype = msg_id;
        sprintf(msg.mtext, "%s", line + pos);
        msg.clientID = clientID;
        msg.clientPID = getpid();
        if (msgsnd(server, &msg, maxMsg, 0) < 0)
        {
            if (errno == EINVAL && intr)
            {
                printf("Error 404\nServer not found\n");
                continue;
            }
            else err_msg("Error with sending message by client.");
        }

        rcvReply(&msg);
    }

    printf("Exiting from client service.\n");
    exit(EXIT_SUCCESS);
}
