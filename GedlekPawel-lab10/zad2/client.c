/*
*
* Paweł Gędłek
* lab10,zad2
*
*/
#define _BSD_SOURCE
#define _GNU_SOURCE
#define _SVID_SOURCE

#include "cluster.h"

int socket_fd;
int connection_mode;
char name[MAX_MSG];
int id;

void throw_err(const char* msg)
{
    perror(msg);
    exit(1);
}

int calculate(char* msg)
{
    char opr, tmp[10];
    int c = 0, n1, n2, res, tm, len = strlen(msg);

    //delim white space
    while (msg[c] == ' ' && c < len) c++;
    tm = c;

    //parse number1
    while (msg[c] > '0' && msg[c] <= '9' && c < len) c++;
    if (c == len || c == tm) return 0;
    sprintf(tmp, "%.*s",(c-tm > 10) ? c-tm : 10, msg+tm);
    n1 = atoi(tmp);

     //delim white spaces
    while (msg[c] == ' ' && c < len) c++;
    if (c == len) return 0;

    //parse operation
    opr = msg[c++];

    //delim white spaces
    while (msg[c] == ' ' && c < len) c++;

    //parse number2
    if (c == len || msg[c] < '0' || msg[c] > '9') return 0;
    sprintf(tmp, "%.*s", (len-c > 10) ? len-c : 10, msg + c);
    n2 = atoi(tmp);

    //check operation and calculate
    switch (opr)
    {
        case '+': res = n1 + n2; break;
        case '-': res = n1 - n2; break;
        case '/': if (n2) res = n1 / n2; else return 0; break;
        case '*': res = n1 * n2; break;
        default: return 0;
    }
    sprintf(msg, "%i", res);
    return 1;
}

int send_packet(msg_t msg_type, char* msg, int cnt)
{
    void *buffer;
    packet_t packet;
    packet.id = id;
    packet.c_count = cnt;
    packet.msg_type = msg_type;
    packet.msg_length = strlen(msg) + 1;

    buffer = malloc(sizeof(packet_t) + packet.msg_length);
    memcpy(buffer, &packet, sizeof(packet_t));
    memcpy(buffer+sizeof(packet_t), msg, packet.msg_length);
    //sending message/packet to server
    if (write(socket_fd, buffer, sizeof(packet_t) + packet.msg_length) < 0)
      return -1;

    return 0;
}
void exitfun(void){
    send_packet(CLIENT_UNRG, name, -1);
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}
void receive_packet(char* msg, packet_t* pck)
{
    void *buff = malloc(sizeof(packet_t) + MAX_MSG);
    if (recv(socket_fd,buff,sizeof(packet_t) + MAX_MSG, 0)<0)
        perror("receive");

    memcpy(pck, buff, sizeof(packet_t));
    memcpy(msg, buff+sizeof(packet_t), pck->msg_length);

    free(buff);
}

void* networking(void *arg)
{
    char buff[MAX_MSG];
    packet_t packet;

    for (;;)
    {
        receive_packet(buff, &packet);

        switch(packet.msg_type)
        {
            case SERVER_PING:
                send_packet(CLIENT_PONG, "Pong", -1);
                break;
            case SERVER_CALC:
                printf("%i) evaluating: %s\n", packet.c_count, buff);
                if(calculate(buff)) send_packet(CLIENT_ANSW, buff, packet.c_count);
                else
                {
                    sprintf(buff, "SYNTAX ERROR!");
                    send_packet(CLIENT_ERRO, buff, -1);
                }
                break;
            default:
                break;
        }
    }
    return (void*) 0;
}

void init_socket(int port, char* server_addr)
{
    if (connection_mode == 0)
    {
        if((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) throw_err("INET socket");

        struct sockaddr_in inet_addr;
        inet_addr.sin_family = AF_INET;
        inet_addr.sin_port = htons(port);
        if (inet_pton(AF_INET, server_addr, &(inet_addr.sin_addr)) < 0)
                    throw_err("Incorrect server address!");
        memset(inet_addr.sin_zero, '\0', sizeof(inet_addr.sin_zero));
        if (connect(socket_fd, (struct sockaddr*) &inet_addr,
            sizeof(struct sockaddr_in))<0) throw_err("INET connect");
    }
    else
    {
        if((socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) throw_err("UNIX socket");

        struct sockaddr_un unix_addr;
        memset(&unix_addr, 0, sizeof(struct sockaddr_un));
        unix_addr.sun_family = AF_UNIX;
        strncpy(unix_addr.sun_path, server_addr,  sizeof(unix_addr.sun_path));

        if (bind(socket_fd, (struct sockaddr*) &unix_addr,
            sizeof(sa_family_t))<0) throw_err("UNIX autobind");
        if (connect(socket_fd, (struct sockaddr*) &unix_addr,
            sizeof(struct sockaddr_un))<0) throw_err("UNIX connect");
    }

    packet_t pck;
    char buff[MAX_MSG];

    send_packet(CLIENT_REGR, name, -1);
    receive_packet(buff, &pck);

    if (pck.msg_type == SERVER_ACCT)
    {
        printf("%s\n", buff);
        id = pck.id;
    }
    else throw_err(buff);
}


int main(int argc, char const *argv[])
{
    char server_addr[100];
    int port;
    if (argc < 4) throw_err("Too few arguments!");
    sprintf(name, "%s", argv[1]);
    sprintf(server_addr, "%s", argv[3]);
    if (strcmp(argv[2], "WEB") == 0)
    {
        connection_mode = 0;
        port = atoi(argv[4]);
        if (port == 0 && argv[4][0] != '0') throw_err("Invalid TCP port!");
    }
    else if (strcmp(argv[2], "UNIX") == 0) connection_mode = 1;
    else throw_err("Invalid connection mode!");

    init_socket(port, server_addr);
    atexit(exitfun);

    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t pth;
    pthread_create(&pth, NULL, &networking, NULL);

    sigwait(&set, &sig);
    pthread_cancel(pth);
    exit(0);

    return 0;
}
