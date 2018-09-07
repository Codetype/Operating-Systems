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

mqd_t msgqueue;
mqd_t clients[maxClientsAmnt];
void err_msg(const char* msg){
    if (errno) perror(msg);
    else printf("%s\n", msg);
    exit(1);
}
void snd_err(const char* msgtext, mqd_t msgqid){
    char msg[maxMsg];
    msg[0] = ERROR;
    sprintf(msg+2, "%s", msgtext);
    if(mq_send(msgqid, msg, maxMsg, 1) < 0) perror("Error during sending message");
}
void sigIntHandler(int sig){
    if (sig == SIGINT)
    {
        printf("\nShutting down server service\n");
        exit(0);
    }
}
void setSigInt(){
    struct sigaction act;
    act.sa_handler = sigIntHandler;
    sigfillset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < -1) err_msg("Signal error");
}
void doINIT(char* msg){
    int i;
    char tmp_name[maxMsg-2];
    mqd_t tmp;
    sprintf(tmp_name, "%s", msg+2);
    if ((tmp = mq_open(tmp_name, O_WRONLY)) < 0) err_msg("Error: Server->client queue opening");
    for (i = 0; i < maxClientsAmnt && clients[i] != -1; i++) {;}
    if (i < maxClientsAmnt)
    {
        clients[i] = tmp;
        msg[0] = REPLY;
        sprintf(msg+2, "%i", i);
        if (mq_send(clients[i], msg, maxMsg, 1) < 0)
        {
            perror("Error: Connection init msg");
            clients[i] = (mqd_t) -1;
            return;
        }
        printf("Client no: %i registred\n", i);
    }
    else
    {
        msg[0] = ERROR;
        sprintf(msg+2, "Queue overflow");
        if (mq_send(tmp, msg, maxMsg, 1)) perror("Error init msg");
    }
}
void doSTOP(char* msg){
    if (msg[1] < 0 || msg[1] > maxClientsAmnt) return;
    int client_id = (int)msg[1];
    if (mq_close(clients[client_id]) < 0) perror("Error: Not remove client");
    printf("Client no: %i removed\n", client_id);
    clients[(int)msg[1]] = -1;
}
void doMIRROR(mqd_t queue_id, char* msg_text){
    char *str1 = msg_text;
    char *str2 = msg_text + strlen(msg_text) - 1;

    while (str1 < str2) {
      char tmp = *str1;
      *str1++ = *str2;
      *str2-- = tmp;
    }
    char msg[maxMsg];
    msg[0] = REPLY;
    sprintf(msg+2, "%s", msg_text);
    if (mq_send(queue_id, msg, maxMsg, 1) < 0) perror("Error: Server sending");
}
void doCALC(mqd_t clqid, char* msg, int len){
  char operator, tmp[10];
  int ind = 0, num1, num2, result;
  while (msg[ind] > '0' && msg[ind] <= '9' && ind < len) ind++;
  if (ind == 0) {
    snd_err("Incorrect syntax message", clqid);
    return;
  }
  sprintf(tmp, "%.*s", (ind > 10) ? ind : 10, msg);
  num1 = atoi(tmp);
    while (msg[ind] == ' ' && ind < len) ind++;
    if (ind == len) {
      snd_err("Incorrect syntax message", clqid);
      return;
    }
    operator = msg[ind++];
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
    char msgs[maxMsg];
    msgs[0] = REPLY;
    sprintf(msgs+2, "%i", result);
    if (mq_send(clqid, msgs, maxMsg, 1) < 0) perror("Error: Server sending");
}
void doTIME(int client_queue_id){
    char msg[maxMsg];
    msg[0] = REPLY;

    FILE *date = popen("date", "r");
    if (date == NULL || date < 0) perror("date");
    fgets(msg+2, maxMsg-2, date);
    pclose(date);

    sprintf(msg+2, "%.*s", (int)strlen(msg+2)-1, msg+2);
    if (mq_send(client_queue_id, msg, maxMsg, 1) < 0) perror("Error during sending message from server");
}
void myexit(void){
    int i;
    for (i = 0; i < maxClientsAmnt; i++)
        if (clients[i] != -1) if (mq_close(clients[i]) < 0) perror("Error during removing client");
    mq_close(msgqueue);
    mq_unlink(SERVER_NAME);
}


int main(int argc, char const *argv[]){
    const char *rqsts[4] = {"MIRROR", "CALC", "TIME", "END"};
    int i, endFlag = 0;
    char type, msg[maxMsg];
    for (i = 0; i < maxClientsAmnt; i++) clients[i] = (mqd_t)-1;

    setSigInt();
    atexit(myexit);

    if ((msgqueue = mq_open(SERVER_NAME, O_CREAT | O_EXCL | O_RDONLY, S_IRUSR | S_IWUSR, NULL)) < 0) err_msg("Error with server's queue opening.");

    while(1)
    {
        if (mq_receive(msgqueue, msg, maxMsg, NULL) < 0) err_msg("Error with server msg received");
        type = msg[0];
        if (type < 5) printf("Received %s from %i\n", rqsts[type-1], (int)msg[1]);
        switch (type)
        {
            case INIT: doINIT(msg); break;
            case STOP: doSTOP(msg); break;
            case MIRROR: doMIRROR((int)clients[(int)msg[1]], msg + 2); break;
            case CALC: doCALC((int)clients[(int)msg[1]], msg + 2, strlen(msg + 2)); break;
            case TIME: doTIME((int)clients[(int)msg[1]]); break;
            case END: endFlag = 1; break;
            default: printf("UNKNOWN request: %s\n", msg + 2); break;
        }
        if (endFlag) break;
    }
    printf("Exiting from server service.\n");
    exit(0);
}
