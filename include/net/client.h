#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>

#define ERR_SOCKET_CREATE 1
#define ERR_SOCKET_CONNECT 2


typedef struct client client;

int client_create(
        client **cl,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype);
int client_close(client *cl);

int client_send(client *cl, void *msg, int msglen);

#endif
