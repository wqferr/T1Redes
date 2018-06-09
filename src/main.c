#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "net/server.h"
#include "net/client.h"
#include "net/address.h"

#define DEFAULT_PORT 7277
#define DEFAULT_SERVER_IP "127.0.0.1"

#define BUF_SIZE 256

#define EXIT_INVALID_PORT 1
#define EXIT_COULDNT_CONNECT 2
#define EXIT_COULDNT_START_SERVER 3

int startClient(const char *serverip, int port);
int startServer(int port);

int main(int argc, char *argv[]) {
    int c;
    enum {
        CLIENT,
        SERVER
    } hosttype = SERVER; // Start server by default
    int port = DEFAULT_PORT;
    int status;
    char *serverip = DEFAULT_SERVER_IP;

    while ((c = getopt(argc, argv, "sc:p:")) != -1) {
        switch (c) {
            case 's':
                hosttype = SERVER;
                break;
            case 'c':
                serverip = strdup(optarg);
                hosttype = CLIENT;
                break;
            case 'p':
                port = atoi(optarg);
                if (port <= 0) {
                    fputs("Invalid port", stderr);
                    exit(EXIT_INVALID_PORT);
                }
                break;
            case '?':
                if (optopt == 'p') {
                    exit(EXIT_INVALID_PORT);
                }
                break;
        }
    }

    if (hosttype == CLIENT) {
        status = startClient(serverip, port);
        free(serverip);
    } else {
        status = startServer(port);
    }
    return status;
}


int startClient(const char *serverip, int port) {
    client *cl;
    char *buf;
    size_t msglen;
    struct sockaddr_in svaddr;

    fprintf(stderr, "Starting client...\n");
    fprintf(stderr, "Connecting to server at %s:%d...\n", serverip, port);

    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = htonl(iptoint(serverip));
    svaddr.sin_port = htons(port);

    if (client_create(&cl, (struct sockaddr *) &svaddr, sizeof(svaddr), SOCK_STREAM) != 0) {
        fprintf(stderr, "Couldn't connect to server. Terminating.\n");
        return EXIT_COULDNT_CONNECT;
    }

    fprintf(stderr, "Connection successful.\n");
    buf = calloc(1, BUF_SIZE);
    
    strcpy(buf, "Hello, server, this is client!");
    fprintf(stderr, "Sending message... ");
    fprintf(stderr, "status: %d\n", client_send(cl, buf, strlen(buf)));

    fprintf(stderr, "Awaiting server message... ");
    fprintf(stderr, "status: %d\n", client_recv(cl, buf, BUF_SIZE, &msglen));
    printf("Server said: \"%s\"\n", buf);
    client_close(cl);
    free(buf);

    return EXIT_SUCCESS;
}


int startServer(int port) {
    server *sv;
    char *buf;
    size_t msglen;
    struct sockaddr_in svaddr;

    fprintf(stderr, "Starting server on port %d...\n", port);

    svaddr.sin_family = AF_INET;
    svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    svaddr.sin_port = htons(port);

    if (server_create(&sv, (struct sockaddr *) &svaddr, sizeof(svaddr), SOCK_STREAM) != 0) {
        fprintf(stderr, "Couldn't start server. Terminating.\n");
        return EXIT_COULDNT_START_SERVER;
    }
    fprintf(stderr, "Server started.\n");

    buf = calloc(1, BUF_SIZE);

    fprintf(stderr, "Awaiting clients... ");
    fprintf(stderr, "status: %d\n", server_awaitClients(sv, 1));
    fprintf(stderr, "Client found.\n");

    fprintf(stderr, "Awaiting message... ");
    fprintf(stderr, "status: %d\n", server_recv(sv, 0, buf, BUF_SIZE, &msglen));

    strcpy(buf, "Hello, client, this is server!");
    fprintf(stderr, "Sending message... ");
    fprintf(stderr, "status: %d\n", server_send(sv, 0, buf, strlen(buf)));
    printf("Client said: \"%s\"\n", buf);

    server_close(sv);
    free(buf);

    return EXIT_SUCCESS;
}