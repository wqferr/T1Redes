#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>

#define ERR_CLIENT_CREATE_SOCKET 1
#define ERR_CLIENT_CONNECT_SOCKET 2


typedef struct client client;

int client_create(
        client **cl,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype);
int client_close(client *cl);

int client_send(client *cl, void *msg, int msglen);

#endif
