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
    int enable;
    
    server *sv = malloc(sizeof(*sv));
    sv->clients = NULL;
    sv->nclients = 0;

    sv->sockfd = socket(AF_INET, socktype, 0);
    if (sv->sockfd == -1) {
        // Couldn't create socket
        free(sv);
        return ERR_SERVER_CREATE_SOCKET;
    }

    enable = 1;
    setsockopt(sv->sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    if (bind(sv->sockfd, serveraddr, addrlen) < 0) {
        // Couldn't create server
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
        // Close connection to all clients
        close(sv->clients[i]);
    }
    free(sv->clients);
    free(sv);
    return 0;
}


int server_awaitClients(server *sv, int n) {
    int client;

    if (sv->clients != NULL || listen(sv->sockfd, n) < 0) {
        // Server is already listening, has clients connected
        // or listen returned an error
        return ERR_SERVER_LISTEN;
    }
    sv->nclients = 0;
    sv->clients = malloc(n * sizeof(*sv->clients));
    while (sv->nclients < n) {
        // Loop until n clients are successfully accepted
        client = accept(sv->sockfd, NULL, 0);
        if (client >= 0) {
            sv->clients[sv->nclients] = client;
            sv->nclients++;
        }
    }

    return 0;
}

int server_send(server *sv, int clientid, const void *msg, size_t msglen) {
    if (clientid < 0 || clientid >= sv->nclients) {
        // Client index out of bounds
        return ERR_SERVER_NO_SUCH_CLIENT;
    }
    if (write(sv->clients[clientid], msg, msglen) < 0) {
        // Error sending message
        return ERR_SERVER_SOCKET_CLOSED;
    }
    return 0;
}

int server_recv(server *sv, int clientid, void *buf, size_t bufsize, size_t *nread) {
    if (clientid < 0 || clientid >= sv->nclients) {
        // Client index out of bounds
        return ERR_SERVER_NO_SUCH_CLIENT;
    }
    *nread = read(sv->clients[clientid], buf, bufsize);
    if (*nread < 0) {
        // Error reading message
        return ERR_SERVER_SOCKET_CLOSED;
    }
    return 0;
}