#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "net/server.h"
#include "net/client.h"

#define DEFAULT_PORT 7277
#define DEFAULT_SERVER_IP "127.0.0.1"

#define EXIT_INVALID_PORT 1

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
    fprintf(stderr, "starting client\n");
    fprintf(stderr, "connecting to server at %s:%d...\n", serverip, port);

    return EXIT_SUCCESS;
}


int startServer(int port) {
    fprintf(stderr, "starting server on port %d\n", port);
    return EXIT_SUCCESS;
}