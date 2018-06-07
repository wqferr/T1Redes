#include "net/server.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


struct server {
    int sockfd;
};

int server_create(
        server **svref,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype) {
    server *sv = malloc(sizeof(*sv));
    sv->sockfd = socket(AF_INET, socktype, 0);
    if (sv->sockfd == -1) {
        free(sv);
        return ERR_SERVER_CREATE_SOCKET;
    }
    if (bind(sv->sockfd, serveraddr, addrlen) < 0) {
        close(sv->sockfd);
        free(sv);
        return ERR_SERVER_BIND_SOCKET;
    }

    *svref = sv;
    return 0;
}


int server_close(server *sv) {
    close(sv->sockfd);
    free(sv);
    return 0;
}
