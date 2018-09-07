#ifndef CENTRAL_H_SYSTEMV
#define CENTRAL_H_SYSTEMV

#define MIRROR  1
#define CALC    2
#define TIME    3
#define END     4
#define INIT    5
#define STOP    6
#define REPLY   7
#define ERROR   8
#define UNKNOWN 9

#define maxMsgText     128
#define maxMsg         sizeof(struct msgbuf)-sizeof(long)
#define maxClientsAmnt 8

struct msgbuf {
    long mtype;
    char mtext[maxMsgText];
    pid_t clientPID;
    int clientID;
};

#endif
