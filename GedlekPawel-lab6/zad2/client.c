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
#include <mqueue.h>

#include "central.h"

mqd_t serverqueue;
mqd_t server;
int clientID = -1;
char queue_name[8];

void myexit(void){
    if (clientID != -1)
    {
        char msg[maxMsg];
        msg[0] = STOP;
        msg[1] = (char)clientID;
        sprintf(msg+2, "%i", clientID);
        if (mq_send(server, msg, maxMsg, 1) < 0) perror("Error with sending");
    }

    if (mq_close(server) < 0) perror("Error with close server");
    if (mq_close(serverqueue) < 0) perror("Error with close client");
    if (mq_unlink(queue_name) < 0) perror("Error with unlink client");
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
void setSigInt(){
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
void connectClientToServer(char* queue_name){
    char msg[maxMsg];

    msg[0] = INIT;
    msg[1] = -1;
    sprintf(msg+2, "%s", queue_name);
    if (mq_send(server, msg, maxMsg, 1) < 0) err_msg("Error with init");

    if (mq_receive(serverqueue, msg, maxMsg, NULL) < 0) err_msg("Error with received init");
    switch(msg[0])
    {
        case REPLY: clientID = atoi(msg+2); if (clientID < 0) err_msg("Error with registering"); break;
        case ERROR: err_msg(msg+2); break;
        default: break;
    }
}
void rcvReply(char* msg){
    if (msg[0] == END) return;
    if (mq_receive(serverqueue, msg, maxMsg, NULL) < 0) perror("Error with receiving message");
    printf("\nResult from server: %s\n", msg+2);
}

int main(int argc, char const *argv[])
{
    FILE *file;
    int intr = 1;
    if (argc > 1)
    {
        file = fopen(argv[1], "r");
        if (file == NULL) err_msg("Commands file");
        intr = 0;
    }
    else file = stdin;

    setSigInt();
    atexit(myexit);

    char *buf = NULL;
    char line[100];
    size_t n;
    char msg[maxMsg];
    int count, pos;
    char msg_id;

    sprintf(queue_name, "/%iq", getpid());

    if ((server = mq_open(SERVER_NAME, O_WRONLY)) < 0) err_msg("Error: Client->server queue");
    if ((serverqueue = mq_open(queue_name, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, NULL)) < 0) err_msg("Error: Server->client queue");

    connectClientToServer(queue_name);
    while (printf("server $ ") < 0 || (count = getline(&buf, &n, file)) > 1)
    {
        sprintf(line, "%.*s", (buf[count-1] == '\n') ? count-1 : count, buf);
        msg_id = cmdParser(line, count, &pos);

        if (msg_id == UNKNOWN)
        {
            printf("Command not recognized: %s\n", line);
            continue;
        }

        msg[0] = msg_id;
        sprintf(msg+2, "%s", line + pos);
        msg[1] = (char) clientID;
        if (mq_send(server, msg, maxMsg, 1) < 0)
        {
            if (errno == EBADF && intr)
            {
                printf("400 ERROR\nServer not found\n");
                continue;
            }
            else err_msg("Error: Client send");
        }

        rcvReply(msg);
    }

    printf("Exiting from client service\n");
    exit(0);
}
