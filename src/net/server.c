#include "net/server.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>


struct server {
    int sockfd;
    int *clients;
    int nclients;
};

int server_create(
        server **svref,
        struct sockaddr *serveraddr,
        socklen_t addrlen,
        int socktype) {
    server *sv = malloc(sizeof(*sv));
    sv->clients = NULL;
    sv->nclients = 0;

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
    int i;
    close(sv->sockfd);
    for (i = 0; i < sv->nclients; i++) {
        close(sv->clients[i]);
    }
    free(sv->clients);
    free(sv);
    return 0;
}


int server_awaitClients(server *sv, int n) {
    int client;

    if (sv->clients != NULL || listen(sv->sockfd, n) < 0) {
        return ERR_SERVER_LISTEN;
    }
    sv->nclients = 0;
    sv->clients = malloc(n * sizeof(*sv->clients));
    while (sv->nclients < n) {
        client = accept(sv->sockfd, NULL, 0);
        if (client >= 0) {
            sv->nclients++;
        }
    }

    return 0;
}
