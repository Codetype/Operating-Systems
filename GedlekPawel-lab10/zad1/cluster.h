#ifndef _CLUSTER_SERVER
#define _CLUSTER_SERVER

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#define UNIX_PATH_MAX   108
#define MAX_MSG         100
#define SERVER_ACCT     (char)0
#define SERVER_REJC     (char)1
#define SERVER_PING     (char)2
#define SERVER_CALC     (char)3
#define CLIENT_REGS     (char)4
#define CLIENT_PONG     (char)5
#define CLIENT_ANSW     (char)6
#define CLIENT_ERRO     (char)7
#define CLIENT_UNRG     (char)8

typedef char msg_t;
typedef struct {
    msg_t msg_type;
    int msg_length;
    int c_count;
} packet_t;

struct client_node {
    int fd;
    char* name;
    int ping;
    struct client_node* next;
} client_node;

typedef struct {
    int size;
    int counter;
    struct client_node* first;
} client_list;

#endif
