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

#define MAX_EVENTS      40
#define MAX_CLIENTS     20

int port_num;
char socket_path[UNIX_PATH_MAX];
int inet_socket, unix_socket;
client_list clients;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;
void _init_(client_list* cl)
{
    cl->size = 0;
    cl->first = NULL;
    cl->counter = 0;
}

void _add_(client_list* cl, int fd, char* name)
{
    struct client_node* tmp, *p = cl->first;
    //empty list
    if (p == NULL)
    {
        p = malloc(sizeof(struct client_node));
        p->fd = fd;
        p->name = malloc((strlen(name)+1)*sizeof(char));
        sprintf(p->name, "%s", name);
        p->next = NULL;
        p->ping = 1;
        cl->first = p;
        cl->size = 1;
    } //list add to other clients
    else
    {
        while(p->next != NULL && p->next->fd < fd) p = p->next;
        tmp = p->next;
        p->next = malloc(sizeof(struct client_node));
        p->next->fd = fd;
        p->next->name = malloc((strlen(name)+1)*sizeof(char));
        sprintf(p->next->name, "%s", name);
        p->next->next = tmp;
        p->next->ping = 1;
        cl->size++;
    }
}

int _delete_(client_list *cl, int fd)
{
    struct client_node* tmp, *p = cl->first;
    if (p == NULL) return 0;
    if (p->fd == fd)
    {
        cl->first = p->next;
        cl->size--;
        free(p->name);
        free(p);
        return 1;
    }
    while(p->next != NULL && p->next->fd != fd) p = p->next;
    if (p->next == NULL) return 0;
    tmp = p->next->next;
    free(p->next->name);
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

int _next_fd_(client_list *cl)
{
    struct client_node* p = cl->first;
    int i;
    if (cl->size == 0) return 0;
    for (i = 0; i<(cl->counter)%(cl->size); i++) p = p->next;
    cl->counter++;
    return p->fd;
}

void reset_all_pings(client_list *cl)
{
    struct client_node* p;
    for (p = cl->first; p != NULL; p = p->next) p->ping = 0;
}

int confirm_ping(client_list *cl, int fd)
{
    struct client_node* p = cl->first;
    while(p != NULL && p->fd != fd) p = p->next;
    if (p == NULL) return 0;

    p->ping = 1;
    return 0;
}
void throw_err(const char* msg)
{
    perror(msg);
    exit(1);
}

int send_packet(int client_fd, msg_t msg_type, char* msg, int cnt)
{
    void *buff;
    packet_t pck;
    pck.msg_type = msg_type;
    pck.msg_length = strlen(msg) + 1;
    pck.c_count = cnt;

    buff = malloc(sizeof(packet_t) + pck.msg_length);
    memcpy(buff, &pck, sizeof(packet_t));
    memcpy(buff+sizeof(packet_t), msg, pck.msg_length);
    if (write(client_fd, buff,
        sizeof(packet_t) + pck.msg_length) < 0) return -1;
    return 0;
}


void* terminal(void *arg)
{
    char* expr = NULL;
    size_t n = 0;
    int cnt;
    printf("server $0> ");
    for (;;)
    {
        cnt = getline(&expr, &n, stdin);
        expr[cnt-1] = '\0';

        pthread_mutex_lock(&count_mutex);
        if (clients.size > 0) send_packet(_next_fd_(&clients), SERVER_CALC, expr, counter++);
        else printf("Calculating is impossible without clients!\n $%i> ", counter);
        pthread_mutex_unlock(&count_mutex);

        free(expr);
        n = 0;
    }

    return (void*) 0;
}


void close_socket(int socket_fd)
{
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
}

int accept_client(int server_fd)
{
    int client_fd;
    packet_t pck;
    char buff[MAX_MSG];

    if ((client_fd = accept(server_fd, NULL, NULL)) < 0) return 0;
    read(client_fd, &pck, sizeof(packet_t));
    read(client_fd, buff, pck.msg_length);

    pthread_mutex_lock(&count_mutex);

    if (clients.size >= MAX_CLIENTS)
    {
        pthread_mutex_unlock(&count_mutex);

        send_packet(client_fd, SERVER_REJC, "Server full", -1);
        close_socket(client_fd);
        return 0;
    }
    if (_is_present_(&clients, buff))
    {
        pthread_mutex_unlock(&count_mutex);

        send_packet(client_fd, SERVER_REJC, "Name already in use", -1);
        close_socket(client_fd);
        return 0;
    }

    _add_(&clients, client_fd, buff);

    pthread_mutex_unlock(&count_mutex);

    if(send_packet(client_fd, SERVER_ACCT, \
        "CALC cluster server: welcome!", -1)<0)
        return 0;
    return client_fd;
}

void accept_message(int client_fd)
{
    packet_t packet;
    char buff[MAX_MSG];
    int t;
    if ((t = read(client_fd, &packet, sizeof(packet_t))) <= 0) return;
    if ((t = read(client_fd, buff, packet.msg_length)) <= 0) return;
    switch(packet.msg_type)
    {
        case CLIENT_PONG:
            pthread_mutex_lock(&count_mutex);
            confirm_ping(&clients, client_fd);
            pthread_mutex_unlock(&count_mutex);
        break;

        case CLIENT_ANSW:
            printf("%i) res = %s\nserver $%i> ", packet.c_count, buff, counter);
            fflush(stdout);
        break;

        case CLIENT_ERRO:
            printf("%s\nserver $%i> ", buff, counter);
            fflush(stdout);
        break;

        case CLIENT_UNRG:
            pthread_mutex_lock(&count_mutex);
            close_socket(client_fd);
            _delete_(&clients, client_fd);
            pthread_mutex_unlock(&count_mutex);
        break;

        default:
            break;
    }
}

void init_socket(void)
{
    struct sockaddr_in addr_inet;
    struct sockaddr_un addr_unix;
    int one = 1;

    inet_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(inet_socket, SOL_SOCKET, SO_REUSEADDR,
        &one, sizeof(int)) < 0) throw_err("inet setsockopt");
    memset(&addr_inet, 0, sizeof(struct sockaddr_in));
    addr_inet.sin_family = AF_INET;
    addr_inet.sin_port = htons((uint16_t)port_num);
    addr_inet.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(inet_socket, (const struct sockaddr *) &addr_inet,
        sizeof(struct sockaddr_in)) < 0) throw_err("Inet socket bind");
    if (listen(inet_socket, MAX_CLIENTS) < 0) throw_err("Inet socket listen");

    unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    memset(&addr_unix, 0, sizeof(struct sockaddr_un));
    addr_unix.sun_family = AF_UNIX;
    strncpy(addr_unix.sun_path, socket_path, sizeof(addr_unix.sun_path) - 1);
    if (bind(unix_socket, (const struct sockaddr *) &addr_unix,
        sizeof(struct sockaddr_un)) < 0) throw_err("Unix socket bind");
    if (listen(unix_socket, MAX_CLIENTS) < 0) throw_err("Unix socket listen");

    _init_(&clients);
}

void exitfun(void)
{
    close_socket(inet_socket);
    close_socket(unix_socket);

    struct client_node* p;
    for (p = clients.first; p != NULL; p = p->next) close_socket(p->fd);

    unlink(socket_path);

    pthread_mutex_destroy(&count_mutex);

    printf("\n");
}



void* networking(void *arg)
{
    struct epoll_event ev, events[MAX_EVENTS];
    int epoll_fd, nfds, i, connection_fd;

    if ((epoll_fd = epoll_create1(0))< 0) throw_err("Epoll creation");
    //input events
    ev.events = EPOLLIN;
    ev.data.fd = inet_socket;
    //register inet file decriptor to monitoring
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inet_socket, &ev) < 0)
        throw_err("Epoll server add");
    ev.data.fd = unix_socket;
    //register unix file decriptor to monitoring
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, unix_socket, &ev) < 0)
        throw_err("Epoll server add");
    ev.events = EPOLLIN | EPOLLET;

    while(1)
    {
        if ((nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1)) < 0)
            throw_err("Epoll wait");

        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == inet_socket
                || events[i].data.fd == unix_socket)
            {
                connection_fd = accept_client(events[i].data.fd);
                if (connection_fd)
                {
                    ev.data.fd = connection_fd;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection_fd, &ev)
                        < 0) printf("Epoll client error\n");
                }
            }
            else accept_message(events[i].data.fd);
        }
    }

    return (void*) 0;
}

void* clients_ping(void *arg)
{
    struct client_node* p;
    while(1)
    {
        pthread_mutex_lock(&count_mutex);

        for (p = clients.first; p != NULL; p = p->next)
        {
            if (p->ping == 0)
            {
                close_socket(p->fd);
                _delete_(&clients, p->fd);
            }
        }

        reset_all_pings(&clients);

        for (p = clients.first; p != NULL; p = p->next)
            send_packet(p->fd, SERVER_PING, "Ping", -1);

        pthread_mutex_unlock(&count_mutex);

        sleep(2);
    }

    return (void*) 0;
}

int main(int argc, char const *argv[])
{
    if (argc < 3) throw_err("Too few arguments!");
    port_num = atoi(argv[1]);
    if (port_num == 0 && argv[1][0] != '0') throw_err("Invalid TCP port!");
    sprintf(socket_path, "%s", argv[2]);

    init_socket();
    atexit(exitfun);

    sigset_t set;
    int sig, i;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    pthread_t pth[3];
    pthread_create(&pth[0], NULL, &terminal, NULL);
    pthread_create(&pth[1], NULL, &networking, NULL);
    pthread_create(&pth[2], NULL, &clients_ping, NULL);

    sigwait(&set, &sig);
    for (i = 0; i<3; i++) pthread_cancel(pth[i]);
    exit(0);
}
