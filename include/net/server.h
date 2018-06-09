#ifndef SERVER_H
#define SERVER_H

#include <sys/socket.h>

#define ERR_SERVER_CREATE_SOCKET 1
#define ERR_SERVER_BIND_SOCKET 2
#define ERR_SERVER_LISTEN 3
#define ERR_SERVER_SOCKET_CLOSED 4
#define ERR_SERVER_NO_SUCH_CLIENT 5

typedef struct server server;

int server_create(
        server **sv,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype);
int server_close(server *sv);

int server_awaitClients(server *sv, int n);

int server_send(server *sv, int clientid, const void *msg, size_t msglen);
int server_recv(server *sv, int clientid, void *buf, size_t bufsize, size_t *nread);

#endif
