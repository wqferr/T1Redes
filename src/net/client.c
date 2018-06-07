#include "net/client.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>


struct client {
    int sockfd;
};


int client_create(
        client **clref,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype) {
    client *cl = malloc(sizeof(*cl));
    cl->sockfd = socket(AF_INET, socktype, 0);
    if (cl->sockfd == -1) {
        free(cl);
        return ERR_CLIENT_CREATE_SOCKET;
    }
    if (connect(cl->sockfd, serveraddr, addrlen) < 0) {
        close(cl->sockfd);
        free(cl);
        return ERR_CLIENT_CONNECT_SOCKET;
    }

    *clref = cl;
    return 0;
}


int client_close(client *cl) {
    if (cl == NULL) {
        return ERR_CLIENT_INVALID_SOCKET;
    }
    close(cl->sockfd);
    free(cl);
    return 0;
}
