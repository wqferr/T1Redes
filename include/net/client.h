#ifndef CLIENT_H
#define CLIENT_H

#include <stddef.h>
#include <sys/socket.h>

#define ERR_CLIENT_CREATE_SOCKET 1
#define ERR_CLIENT_CONNECT_SOCKET 2
#define ERR_CLIENT_SOCKET_CLOSED 3


typedef struct client client;

int client_create(
        client **cl,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype);
int client_close(client *cl);

int client_send(client *cl, const void *msg, size_t msglen);
int client_recv(client *cl, void *buf, size_t bufsize, size_t *nread);

#endif
