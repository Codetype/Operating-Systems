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

#define MAX_EVENTS      40
#define MAX_CLIENTS     20

int client_counter = 0;
client_list clients;
int inet_socket;
int unix_socket;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;
char socket_path[100];

void _init_(client_list* cl)
{
    cl->size = 0;
    cl->first = NULL;
    cl->counter = 0;
}

void _add_(client_list* cl, struct sockaddr* address, socklen_t addr_size,
    char* name, int id, int sock)
{
    struct client_node* tmp, *p = cl->first;
    if (p == NULL)
    {
        p = malloc(sizeof(struct client_node));
        p->address = malloc(addr_size);
        memcpy(p->address, address, addr_size);
        p->addr_size = addr_size;
        p->name = malloc((strlen(name)+1)*sizeof(char));
        sprintf(p->name, "%s", name);
        p->id = id;
        p->sock = sock;

        p->next = NULL;
        p->ping = 1;
        cl->first = p;
        cl->size = 1;
    }
    else
    {
        while(p->next != NULL && p->next->id < id) p = p->next;
        tmp = p->next;
        p->next = malloc(sizeof(struct client_node));
        p->next->address = malloc(addr_size);
        memcpy(p->next->address, address, addr_size);
        p->next->addr_size = addr_size;
        p->next->name = malloc((strlen(name)+1)*sizeof(char));
        sprintf(p->next->name, "%s", name);
        p->next->id = id;
        p->next->sock = sock;
        p->next->next = tmp;
        p->next->ping = 1;
        cl->size++;
    }
}

int _delete_(client_list *cl, int id){
    struct client_node* tmp, *p = cl->first;
    if (p == NULL) return 0;
    if (p->id == id)
    {
        cl->first = p->next;
        cl->size--;
        free(p->address);
        free(p->name);
        free(p);
        return 1;
    }
    while(p->next != NULL && p->next->id != id) p = p->next;
    if (p->next == NULL) return 0;
    tmp = p->next->next;
    free(p->next->name);
    free(p->next->address);
    free(p->next);
    p->next = tmp;
    cl->size--;
    return 1;
}

int _is_present_(client_list *cl, char* name)
{
    struct client_node* p = cl->first;
    while(p != NULL && strcmp(p->name, name) != 0) p = p->next;
    return p != NULL;
}

void _next_addr_(client_list *cl, struct sockaddr** address, socklen_t* addr_size, int* sock){
    struct client_node* p = cl->first;
    int i;
    if (cl->size == 0) return;
    for (i = 0; i<(cl->counter)%(cl->size); i++) p = p->next;
    cl->counter++;
    *address = p->address;
    *addr_size = p->addr_size;
    *sock = p->sock;
    return;
}

void reset_all_pings(client_list *cl)
{
    struct client_node* p;
    for (p = cl->first; p != NULL; p = p->next) p->ping = 0;
}

int confirm_ping(client_list *cl, int fd)
{
    struct client_node* p = cl->first;
    while(p != NULL && p->id != fd) p = p->next;
    if (p == NULL) return 0;

    p->ping = 1;
    return 0;
}


void throw_err(const char* msg)
{
  perror(msg);
  exit(1);
}

void send_packet(int socket_fd, struct sockaddr* client_addr, socklen_t addr_size,
    msg_t msg_type, char * msg, int cnt, int id)
{
    void *buff;
    packet_t pck;
    pck.msg_type = msg_type;
    pck.msg_length = strlen(msg) + 1;
    pck.c_count = cnt;
    pck.id = id;

    buff = malloc(sizeof(packet_t) + MAX_MSG);
    memcpy(buff, &pck, sizeof(packet_t));
    memcpy(buff+sizeof(packet_t), msg, pck.msg_length);

    if (sendto(socket_fd,buff,sizeof(packet_t) + pck.msg_length,0,
        client_addr,addr_size)<0) perror("sendto");

    free(buff);
}

void receive_packet(int socket_fd, struct sockaddr * client_addr, socklen_t* addr_size,
    char* msg, packet_t* pck)
{
    void *buff = malloc(sizeof(packet_t) + MAX_MSG);
    if (recvfrom(socket_fd,buff,sizeof(packet_t) + MAX_MSG, 0,
        client_addr, addr_size)<0) perror("recvfrom");

    memcpy(pck, buff, sizeof(packet_t));
    memcpy(msg, buff+sizeof(packet_t), pck->msg_length);

    free(buff);
}

void accept_message(int socket_fd)
{
    socklen_t addr_size = (socket_fd == inet_socket) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_un);
    struct sockaddr* client_address = malloc(addr_size);

    char buff[MAX_MSG];
    packet_t pck;

    receive_packet(socket_fd, client_address, &addr_size, buff, &pck);

    switch(pck.msg_type)
    {
        case CLIENT_REGR:
            pthread_mutex_lock(&count_mutex);

            if (clients.size >= MAX_CLIENTS)
            {
                pthread_mutex_unlock(&count_mutex);

                send_packet(socket_fd, client_address, addr_size,
                    SERVER_REJC, "Server full", -1, -1);
                return;
            }
            if (_is_present_(&clients, buff))
            {
                pthread_mutex_unlock(&count_mutex);

                send_packet(socket_fd, client_address, addr_size,
                    SERVER_REJC, "Name already in use", -1, -1);
                return;
            }
            send_packet(socket_fd, client_address, addr_size,
                SERVER_ACCT, "CALC cluster server: welcome!", -1, client_counter);

            _add_(&clients, client_address, addr_size, buff,
                client_counter++, socket_fd);

            pthread_mutex_unlock(&count_mutex);
            break;
        case CLIENT_PONG:
            pthread_mutex_lock(&count_mutex);

            confirm_ping(&clients, pck.id);

            pthread_mutex_unlock(&count_mutex);
            break;
        case CLIENT_ANSW:
            printf("%i) res = %s\nserver $%i> ", pck.c_count, buff, counter);
            fflush(stdout);
            break;

        case CLIENT_ERRO:
            printf("%s\nserver $%i> ", buff, counter);
            fflush(stdout);
            break;
        case CLIENT_UNRG:
            pthread_mutex_lock(&count_mutex);

            _delete_(&clients, pck.id);

            pthread_mutex_unlock(&count_mutex);
            break;
        default:
            break;
    }
    free(client_address);
}

void* network(void *arg)
{
    struct epoll_event ev, events[MAX_EVENTS];
    int epoll_fd, nfds, i;

    if ((epoll_fd = epoll_create1(0))< 0) throw_err("Epoll creation");
    ev.events = EPOLLIN;
    ev.data.fd = inet_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inet_socket, &ev) < 0)
        throw_err("Epoll server add");
    ev.data.fd = unix_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, unix_socket, &ev) < 0)
        throw_err("Epoll server add");

    for (;;)
    {
        if ((nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) < 0)
            throw_err("Epoll wait");

        for (i = 0; i < nfds; i++) accept_message(events[i].data.fd);
    }

    return (void*) 0;
}

void* clients_ping(void *arg)
{
    struct client_node* p;
    struct sockaddr* address;
    socklen_t addr_size;
    for (;;)
    {
        pthread_mutex_lock(&count_mutex);

        for (p = clients.first; p != NULL; p = p->next)
            if (p->ping == 0) _delete_(&clients, p->id);

        reset_all_pings(&clients);

        for (p = clients.first; p != NULL; p = p->next)
        {
            address = p->address;
            addr_size = p->addr_size;
            send_packet(p->sock, address, addr_size,
                SERVER_PING, "Ping", -1, -1);
        }

        pthread_mutex_unlock(&count_mutex);

        sleep(2);
    }

    return (void*) 0;
}

void* server_thread(void *arg)
{
    struct sockaddr* address;
    socklen_t addr_size;
    char* line = NULL;
    size_t n = 0;
    int count, sock;
    printf("0> ");
    for (;;)
    {
        count = getline(&line, &n, stdin);
        line[count-1] = '\0';

        pthread_mutex_lock(&count_mutex);
        if (clients.size > 0)
        {
           _next_addr_(&clients, &address, &addr_size, &sock);
            send_packet(sock, address, addr_size, SERVER_CALC, line, counter++, -1);
        }
        else printf("No clients connected!\n%i> ", counter);
        pthread_mutex_unlock(&count_mutex);

        free(line);
        n = 0;
    }

    return (void*) 0;
}

void init_socket(int port, char* socket_path)
{
    struct sockaddr_in addr_inet;
    struct sockaddr_un addr_unix;

    if ((inet_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) throw_err("Inet socket");
    addr_inet.sin_family = AF_INET;
    addr_inet.sin_port = htons(port);
    addr_inet.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(addr_inet.sin_zero, '\0', sizeof addr_inet.sin_zero);
    if(bind(inet_socket, (struct sockaddr *) &addr_inet, sizeof(addr_unix)) < 0)
        throw_err("Inet bind");

    unlink(socket_path);
    if ((unix_socket = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) throw_err("Unix socket");
    memset(&addr_unix, 0, sizeof(struct sockaddr_un));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, socket_path, sizeof(addr_unix.sun_path));
    if(bind(unix_socket, (struct sockaddr *) &addr_unix, sizeof(addr_unix)) < 0)
        throw_err("Unix bind");

    _init_(&clients);
}

void exitfun(void)
{
    shutdown(inet_socket, SHUT_RDWR);
    shutdown(unix_socket, SHUT_RDWR);

    close(inet_socket);
    close(unix_socket);

    unlink(socket_path);

    pthread_mutex_destroy(&count_mutex);

    printf("\n");
}

int main(int argc, char const *argv[])
{
    int port;
    if (argc < 2) throw_err("Too few arguments!");
    port = atoi(argv[1]);
    if (port == 0 && argv[1][0] != '0') throw_err("Invalid TCP port!");
    sprintf(socket_path, "%s", argv[2]);

    atexit(exitfun);
    init_socket(port, socket_path);

    sigset_t set;
    int sig, i;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t pth[3];
    pthread_create(&pth[0], NULL, &network, NULL);
    pthread_create(&pth[1], NULL, &clients_ping, NULL);
    pthread_create(&pth[2], NULL, &server_thread, NULL);

    sigwait(&set, &sig);
    for (i = 0; i<3; i++) pthread_cancel(pth[i]);
    exit(0);
}
