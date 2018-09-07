/*
*
* Paweł Gędłek
* lab10,zad1
*
*/
#define _BSD_SOURCE
#define _GNU_SOURCE
#define _SVID_SOURCE

#include "cluster.h"

int server_fd;
int connection_mode;
char name[MAX_MSG];
struct sockaddr* server_address = NULL;


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
    packet.c_count = cnt;
    packet.msg_type = msg_type;
    packet.msg_length = strlen(msg) + 1;


    buffer = malloc(sizeof(packet_t) + packet.msg_length);
    memcpy(buffer, &packet, sizeof(packet_t));
    memcpy(buffer+sizeof(packet_t), msg, packet.msg_length);
    //sending message/packet to server
    if (write(server_fd, buffer, sizeof(packet_t) + packet.msg_length) < 0)
      return -1;

    return 0;
}
void exitfun(void)
{
    //logout client
    send_packet(CLIENT_UNRG, name, -1);
    //close and shutdown server
    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);
    if (server_address != NULL)
      free(server_address);
}
void* client_server_network(void *arg)
{
    char buff[MAX_MSG];
    packet_t packet;

    for (;;)
    {
        //reading packets from server
        read(server_fd, &packet, sizeof(packet_t));
        read(server_fd, buff, packet.msg_length);

        switch(packet.msg_type)
        {
            case SERVER_PING:
                send_packet(CLIENT_PONG, "Pong", -1);
            break;
            case SERVER_CALC:
                //making and sending calculations
                printf("%i) evaluating: %s\n", packet.c_count, buff);
                if(calculate(buff)) send_packet(CLIENT_ANSW, buff, packet.c_count);
                else
                {
                    sprintf(buff, "SYNTAX ERROR");
                    send_packet(CLIENT_ERRO, buff, -1);
                }
                break;
            default:
                break;
        }
    }

    return (void*) 0;
}

void init_client_server_connection(void)
{
    char buff[MAX_MSG];
    packet_t packet;
    //web connection_mode
    if (connection_mode == 0)
    {
        //creating INET socket, get server_fd
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            throw_err("INET socket");
        //connecting to server from client
        if (connect(server_fd, server_address, sizeof(struct sockaddr_in)) < 0)
            throw_err("INET connection client-server");
    } //unix connection_mode
    else
    {
        //creating unix socket, get server_fd
        if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
            throw_err("UNIX socket");
        //connecting to server from client
        if (connect(server_fd, server_address, sizeof(struct sockaddr_un)) < 0)
            throw_err("UNIX connection client-server");
    }
    //registretion packet
    send_packet(CLIENT_REGS, name, -1);

    //waiting for server aproving
    read(server_fd, &packet, sizeof(packet_t));
    read(server_fd, buff, packet.msg_length);

    if (packet.msg_type == SERVER_ACCT) printf("%s\n", buff);
    else throw_err(buff);
}
int main(int argc, char const *argv[])
{
  if (argc < 4) throw_err("Too few arguments!");
  //printt("%s", argv[1]);
  sprintf(name, "%s", argv[1]);

  if (strcmp(argv[2], "WEB") == 0)
  {
      //printt("%s", argv[2]);
      struct sockaddr_in* tmp;
      connection_mode = 0;
      tmp = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
      memset(tmp, 0, sizeof(struct sockaddr_in));
      tmp->sin_family = AF_INET;
      //printt("%s", argv[4]);
      tmp->sin_port = htons((uint16_t)atoi(argv[4]));
      //printt("%s", argv[3]);
      if (inet_pton(AF_INET, argv[3], &(tmp->sin_addr)) < 0)
          throw_err("Incorrect server address!");
      server_address = (struct sockaddr*)tmp;
  }
  else if (strcmp(argv[2], "UNIX") == 0)
  {
      struct sockaddr_un* tmp;
      connection_mode = 1;
      tmp = (struct sockaddr_un*) malloc(sizeof(struct sockaddr_un));
      memset(tmp, 0, sizeof(struct sockaddr_un));
      tmp->sun_family = AF_UNIX;
      strncpy(tmp->sun_path, argv[3], sizeof(tmp->sun_path) - 1);
      server_address = (struct sockaddr*)tmp;
  }
  else throw_err("Invalid connection connection_mode!");

    sigset_t set;
    int sig;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    atexit(exitfun);
    init_client_server_connection();

    pthread_t pth;
    pthread_create(&pth, NULL, &client_server_network, NULL);

    sigwait(&set, &sig);
    pthread_cancel(pth);
    exit(0);
}
