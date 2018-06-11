#include "net/client.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


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
        // Couldn't create a socket
        free(cl);
        return ERR_CLIENT_CREATE_SOCKET;
    }
    if (connect(cl->sockfd, serveraddr, addrlen) < 0) {
        // Couldn't connect to server
        close(cl->sockfd);
        free(cl);
        return ERR_CLIENT_CONNECT_SOCKET;
    }

    *clref = cl;
    return 0;
}


int client_close(client *cl) {
    close(cl->sockfd);
    free(cl);
    return 0;
}


int client_send(client *cl, const void *msg, size_t msglen) {
    if (write(cl->sockfd, msg, msglen) < 0) {
        // Error sending message
        return ERR_CLIENT_SOCKET_CLOSED;
    }
    return 0;
}

int client_recv(client *cl, void *buf, size_t bufsize, size_t *nread) {
    *nread = read(cl->sockfd, buf, bufsize);
    if (*nread < 0) {
        // Error receiving
        return ERR_CLIENT_SOCKET_CLOSED;
    }
    return 0;
}
