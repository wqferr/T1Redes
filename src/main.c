#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ncurses.h>

#include "net/server.h"
#include "net/client.h"
#include "net/address.h"
#include "game/player.h"

#define DEFAULT_PORT 7277
#define DEFAULT_SERVER_IP "127.0.0.1"

#define BUF_SIZE 256

#define EXIT_INVALID_PORT 1
#define EXIT_COULDNT_CONNECT 2
#define EXIT_COULDNT_START_SERVER 3

int startClient(const char *serverip, int port);
int startServer(int port);
int game_over();

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
    int row, col, i = 0, j = 0, state = 0, selectx = 0, selecty = 0, now = 0, direction = 0;
    int boats[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};
    int **board;
    int *boats_x_coords, *boats_y_coords;

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

    fprintf(stderr, "Awaiting connection server message... ");
    fprintf(stderr, "status: %d\n", client_recv(cl, buf, BUF_SIZE, &msglen));
    fprintf(stderr, "Server said: \"%s\"\n", buf);
    memset(buf, 0, 255);

    fprintf(stderr, "Awaiting ship placement server message... ");
    fprintf(stderr, "status: %d\n", client_recv(cl, buf, BUF_SIZE, &msglen));
    // fprintf(stderr, "Server said: \"%s\"\n", buf);
    memset(buf, 0, 255);

    // initialize interface
    board = (int **) calloc(BOARDSIZEY, sizeof(int *));
    for (int k = 0; k < BOARDSIZEY; k++) {
        board[k] = (int *) calloc(BOARDSIZEX, sizeof(int));
    }

    boats_x_coords = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    boats_y_coords = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    memset(boats_x_coords, -1, sizeof(int)*NUMBERBOATS*5);
    memset(boats_y_coords, -1, sizeof(int)*NUMBERBOATS*5);

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);

    drawDivision();

    while(!game_over()) {
        
        int ch = getch();
        if (ch == 'q') {
            break;
        }
        else if (ch == KEY_DOWN) {
            direction = 0;
            
            if (state == 0) {
                if (i < BOARDSIZEY-1) {
                    i++;
                }   
            }
        }
        else if (ch == KEY_UP) {
            direction = 1;

            if (state == 0) {
                if (i > 0) {
                    i--;
                }   
            }
        }
        else if (ch == KEY_RIGHT) {
            direction = 2;

            if (state == 0) {
                if (j < BOARDSIZEX-1) {
                    j++;
                }   
            }
        }
        else if (ch == KEY_LEFT) {
            direction = 3;

            if (state == 0) {
                if (j > 0) {
                    j--;
                }   
            }
        }
        else if (ch == 10) {
            if (state == 0) {
                if (board[i][j] == 0) {
                    state = 1;
                    selectx = i;
                    selecty = j;
                }
            }
            else if (state == 1) {
                int invalid = 0;

                if (direction == 0) {
                    for (int k = 0; k < boats[now]; k++) {
                        if (board[i+k][j] != 0) {
                            invalid = 1;
                        }
                    }
                } else if (direction == 1) {
                    for (int k = 0; k < boats[now]; k++) {
                        if (board[i-k][j] != 0) {
                            invalid = 1;
                        }
                    }
                } else if (direction == 2) {
                    for (int k = 0; k < boats[now]; k++) {
                        if (board[i][j+k] != 0) {
                            invalid = 1;
                        }
                    }
                } else if (direction == 3) {
                    for (int k = 0; k < boats[now]; k++) {
                        if (board[i][j-k] != 0) {
                            invalid = 1;
                        }
                    }
                }

                int boat_x[5], boat_y[5] = {-1,-1,-1,-1,-1};

                if (invalid == 0) {
                    if (direction == 0) {
                        for (int k = 0; k < boats[now]; k++) {
                            board[i+k][j] = 1;
                            boat_x[k] = i+k;
                            boat_y[k] = j;
                        }
                    } else if (direction == 1) {
                        for (int k = 0; k < boats[now]; k++) {
                            board[i-k][j] = 1;
                            boat_x[k] = i-k;
                            boat_y[k] = j;
                        }
                    } else if (direction == 2) {
                        for (int k = 0; k < boats[now]; k++) {
                            board[i][j+k] = 1;
                            boat_x[k] = i;
                            boat_y[k] = j+k;
                        }
                    } else if (direction == 3) {
                        for (int k = 0; k < boats[now]; k++) {
                            board[i][j-k] = 1;
                            boat_x[k] = i;
                            boat_y[k] = j-k;
                        }
                    }

                    clear();
                    drawDivision();

                    for (int k = 0; k < boats[now]; k++)
                        boats_x_coords[now*5 + k] = boat_x[k];
                    for (int k = 0; k < boats[now]; k++)
                        boats_y_coords[now*5 + k] = boat_y[k];

                    state = 0;
                    now++;

                    if (now >= NUMBERBOATS) {
                        state = 3;
                        client_send(cl, boats_x_coords, sizeof(int) * NUMBERBOATS * 5);
                        client_send(cl, boats_y_coords, sizeof(int) * NUMBERBOATS * 5);
                    }
                }
            }
        }
        else if (ch == KEY_RESIZE) {
            clear();
            drawDivision();
        }

        getmaxyx(stdscr, row, col);

        drawBoard(board, row/2-BOARDSIZEY, col/2-2*BOARDSIZEX-2);
        drawBoard(board, row/2-BOARDSIZEY, col/2+2);

        if (state == 0) {
            drawSquare(row/2-BOARDSIZEY+2*i, col/2-2*BOARDSIZEX-2+2*j);
        }
        else if (state == 1) {
            if (direction == 0 && i+boats[now] < BOARDSIZEY) {
                for (int k = 0; k < boats[now]; k++) {
                    drawSquareSelected(row/2-BOARDSIZEY+2*(i+k), col/2-2*BOARDSIZEX-2+2*j);
                }
            } else if (direction == 1 && i-boats[now] >= 0) {
                for (int k = 0; k < boats[now]; k++) {
                    drawSquareSelected(row/2-BOARDSIZEY+2*(i-k), col/2-2*BOARDSIZEX-2+2*j);
                }
            } else if (direction == 2 && j+boats[now] < BOARDSIZEX) {
                for (int k = 0; k < boats[now]; k++) {
                    drawSquareSelected(row/2-BOARDSIZEY+2*i, col/2-2*BOARDSIZEX-2+2*(j+k));
                }
            } else if (direction == 3 && j-boats[now] >= 0) {
                for (int k = 0; k < boats[now]; k++) {
                    drawSquareSelected(row/2-BOARDSIZEY+2*i, col/2-2*BOARDSIZEX-2+2*(j-k));
                }
            }

            move(row/2+BOARDSIZEY+1, col/2-2*BOARDSIZEX-1);
            for (int k = 0; k < boats[now]; k++) {
                addch('o');
                addch(' ');
            }
        }
        else {
            drawSquare(row/2-BOARDSIZEY+2*i, col/2+2+2*j);
        }
    }
    
    for (int k = 0; k < BOARDSIZEY; k++)
        free(board[k]);
    free(board);

    client_close(cl);
    free(buf);

    endwin();

    return EXIT_SUCCESS;
}

int startServer(int port) {
    server *sv;
    char *buf;
    int *int_buf;
    size_t msglen;
    struct sockaddr_in svaddr;
    int last_client_id = 0;

    srand(time(NULL));

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
    int_buf = calloc(BUF_SIZE, sizeof(int));

    fprintf(stderr, "Awaiting players... ");
    fprintf(stderr, "status: %d\n", server_awaitClients(sv, 2));
    strcpy(buf, "You're connected. Please wait for another player.");
    fprintf(stderr, "Sending connection message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
    
    memset(buf, 0, 255);
    strcpy(buf, "You're connected to another player. Starting game.");
    fprintf(stderr, "Sending starting game message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));
    fprintf(stderr, "Player 2 connected. Starting game.\n");
    
    memset(buf, 0, 255);
    strcpy(buf,"Place your ships on the board!");
    fprintf(stderr, "Sending ship placement message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
    fprintf(stderr, "Sending ship placement message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));
    
    int *player1_board_x = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    int *player1_board_y = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 0, player1_board_x, sizeof(int)*NUMBERBOATS*5, &msglen));
    //memcpy(player1_board_x, int_buf, sizeof(int)*NUMBERBOATS*5);
    fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 0, player1_board_y, sizeof(int)*NUMBERBOATS*5, &msglen));
    // memcpy(player1_board_y, int_buf, sizeof(int)*NUMBERBOATS*5);
    
    for(int i=0; i<NUMBERBOATS; i++) {
        for(int j=0; j<5; j++) {
            fprintf(stderr, "p1: (%d - ", player1_board_x[i*5 + j]);
            fprintf(stderr, "%d)\n", player1_board_y[i*5 + j]);
        }
    }

    int *player2_board_x = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    int *player2_board_y = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
    fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 1, player2_board_x, sizeof(int)*NUMBERBOATS*5, &msglen));
    // memcpy(player2_board_x, int_buf, sizeof(int)*NUMBERBOATS*5);
    fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 1, player2_board_y, sizeof(int)*NUMBERBOATS*5, &msglen));
    // memcpy(player2_board_y, int_buf, sizeof(int)*NUMBERBOATS*5);

    for(int i=0; i<NUMBERBOATS; i++) {
        for(int j=0; j<5; j++) {
            fprintf(stderr, "p2: (%d - ", player2_board_x[i*5 + j]);
            fprintf(stderr, "%d)\n", player2_board_y[i*5 + j]);
        }
    }

    // memset(buf, 0, 255);
    // strcpy(buf, "Randomly selecting first player.");
    // fprintf(stderr, "Sending player selection message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
    // fprintf(stderr, "Sending player selection message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));
    int r = rand() % 2;
    // memset(buf, 0, 255);
    // sprintf(buf, "Player %d was selected to go first", r);
    // fprintf(stderr, "Sending player selected message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
    // fprintf(stderr, "Sending player selected message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));

    int i = 0;
    while(TRUE) {
        // // at this point, client reads if it is its turn
        // sprintf(buf, "%d", r);
        // fprintf(stderr, "Sending message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
        // fprintf(stderr, "Sending message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));
        // fprintf(stderr, "Awaiting player action.. status: %d\n", server_recv(sv, r, buf, BUF_SIZE, &msglen));
        // // TODO: get response and act on it    

        // sprintf(buf, "%d", (r+1)%2);
        // fprintf(stderr, "Sending message... status: %d\n", server_send(sv, 0, buf, strlen(buf)));
        // fprintf(stderr, "Sending message... status: %d\n", server_send(sv, 1, buf, strlen(buf)));
        // fprintf(stderr, "Awaiting player response.. status: %d\n", server_recv(sv, (r+1)%2, buf, BUF_SIZE, &msglen));
        // // TODO: get response and act on it    
        // i++;
    }

    server_close(sv);
    free(buf);

    return EXIT_SUCCESS;
}

int game_over() {
    return 0;
}