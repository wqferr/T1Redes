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

#define FALSE 0
#define TRUE 1

#define DEFAULT_PORT 7277
#define DEFAULT_SERVER_IP "127.0.0.1"

#define BUF_SIZE 256

#define EXIT_INVALID_PORT 1
#define EXIT_COULDNT_CONNECT 2
#define EXIT_COULDNT_START_SERVER 3

int startClient(const char *serverip, int port);
int startServer(int port);
int check_hit(int pos[2], int *board_x, int *board_y, int *hitvec, int *which);
int game_over();
void addLog(char **log, char *message);
int is_down(int *hitvec, int boat);

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
	int **board, **boardenemy;
	int *boats_x_coords, *boats_y_coords;
	char **log, *auxlog;
	int is_over = FALSE, ship_down;

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
	boardenemy = (int **) calloc(BOARDSIZEY, sizeof(int *));
	for (int k = 0; k < BOARDSIZEY; k++) {
		board[k] = (int *) calloc(BOARDSIZEX, sizeof(int));
		boardenemy[k] = (int *) calloc(BOARDSIZEX, sizeof(int));
	}

	auxlog = (char *) calloc(50, sizeof(char));
	log = (char **) malloc(sizeof(char *) * LOGSIZE);
	for (int k = 0; k < LOGSIZE; k++) {
		log[k] = (char *) calloc(50, sizeof(char));
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

	while(TRUE) {
		int ch = getch();
		if (ch == 'q') {
			break;
		}
		else if (ch == KEY_DOWN) {
			direction = 0;
			
			if (state == 0 || state == 3) {
				if (i < BOARDSIZEY-1) {
					i++;
				}   
			}
		}
		else if (ch == KEY_UP) {
			direction = 1;

			if (state == 0 || state == 3) {
				if (i > 0) {
					i--;
				}   
			}
		}
		else if (ch == KEY_RIGHT) {
			direction = 2;

			if (state == 0 || state == 3) {
				if (j < BOARDSIZEX-1) {
					j++;
				}   
			}
		}
		else if (ch == KEY_LEFT) {
			direction = 3;

			if (state == 0 || state == 3) {
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
						state = 4;

						client_send(cl, boats_x_coords, sizeof(int) * NUMBERBOATS * 5);
						client_send(cl, boats_y_coords, sizeof(int) * NUMBERBOATS * 5);
					}
				}
			}
			else if (state == 3) {
				state = 4;
				int *attack_coords = (int *) malloc(sizeof(int) * 2);
				int hit;
				attack_coords[0] = i;
				attack_coords[1] = j;
				client_send(cl, attack_coords, sizeof(int) * 2);
				client_recv(cl, &hit, sizeof(int), &msglen);
				client_recv(cl, &ship_down, sizeof(int), &msglen);
				client_recv(cl, &is_over, sizeof(int), &msglen);

				if (hit) {
					boardenemy[i][j] = 2; // Acertou.

					sprintf(auxlog, "Hit at %d %d!", i, j);
					addLog(log, auxlog);
				}
				else {
					boardenemy[i][j] = 3; // Errou.

					sprintf(auxlog, "Missed at %d %d!", i, j);
					addLog(log, auxlog);
				}

				if(ship_down == 0) {
					sprintf(auxlog, "Enemy Carrier down!");
					addLog(log, auxlog);
				} 
				else if(ship_down == 1) {
					sprintf(auxlog, "Enemy Battleship down!");
					addLog(log, auxlog);
				}
				else if(ship_down == 2) {
					sprintf(auxlog, "Enemy Cruiser down!");
					addLog(log, auxlog);
				}
				else if(ship_down == 3) {
					sprintf(auxlog, "Enemy Destroyer down!");
					addLog(log, auxlog);
				}


				if(is_over) {
					sprintf(auxlog, "Game Over! YOU WIN!!!!!", i, j);
					addLog(log, auxlog);

					state = 999;
				}
			}
		}
		else if (ch == KEY_RESIZE) {
			clear();
			drawDivision();
		}

		if (state != 10000) {
			getmaxyx(stdscr, row, col);

			drawBoard(board, row/2-BOARDSIZEY, col/2-2*BOARDSIZEX-2);
			drawBoard(boardenemy, row/2-BOARDSIZEY, col/2+2);

			drawLog(log);
		}

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
		else if (state == 3) {
			drawSquare(row/2-BOARDSIZEY+2*i, col/2+2+2*j);
		}
		else if (state == 4) {
			wrefresh(stdscr);

			int turn_buf;
			client_recv(cl, &turn_buf, sizeof(int), &msglen);
						
			if(turn_buf == 1) {
				state = 3;
			}
			else {
				int hit, *where;
				where = (int *) malloc(sizeof(int) * 2);
				client_recv(cl, &hit, sizeof(int), &msglen);
				client_recv(cl, where, sizeof(int)*2, &msglen);
				client_recv(cl, &ship_down, sizeof(int), &msglen);
				client_recv(cl, &is_over, sizeof(int), &msglen);

				if(hit == 1) {
					board[where[0]][where[1]] = 2;

					sprintf(auxlog, "Enemy hit at %d %d!", where[0], where[1]);
					addLog(log, auxlog);
				} else {
					board[where[0]][where[1]] = 3;

					sprintf(auxlog, "Enemy missed at %d %d!", where[0], where[1]);
					addLog(log, auxlog);
				}

				if(ship_down == 0) {
					sprintf(auxlog, "Carrier down!");
					addLog(log, auxlog);
				} 
				else if(ship_down == 1) {
					sprintf(auxlog, "Battleship down!");
					addLog(log, auxlog);
				}
				else if(ship_down == 2) {
					sprintf(auxlog, "Cruiser down!");
					addLog(log, auxlog);
				}
				else if(ship_down == 3) {
					sprintf(auxlog, "Destroyer down!");
					addLog(log, auxlog);
				}

				if(is_over) {
					sprintf(auxlog, "Game over! YOU LOSE!!!", where[0], where[1]);
					addLog(log, auxlog);

					state = 9999;
				}
			}
		}
		else if (state == 999) {
			drawWinScreen();
			state = 10000;
		}
		else if (state == 9999) {
			drawLooseScreen();
			state = 10000;
		}
	}
	
	for (int k = 0; k < BOARDSIZEY; k++) {
		free(board[k]);
		free(boardenemy[k]);
	}
	free(board);
	free(boardenemy);

	client_close(cl);
	free(buf);

	endwin();

	return EXIT_SUCCESS;
}

int startServer(int port) {
	server *sv;
	char *buf;
	int *int_buf, *player1_hitvec, *player2_hitvec, *action;
	size_t msglen;
	struct sockaddr_in svaddr;
	const int p_turn = 1;
	const int p_not_turn = 0;
	int hit, p1, p2, over = FALSE;

	srand(time(NULL));
	p1 = rand() % 2;
	p2 = (p1+1) % 2;

	player1_hitvec = (int *) calloc(NUMBERBOATS*5, sizeof(int));
	player2_hitvec = (int *) calloc(NUMBERBOATS*5, sizeof(int));

	action = (int *) malloc(sizeof(int) * 2);
	
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
	fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 0, player1_board_y, sizeof(int)*NUMBERBOATS*5, &msglen));
	
	int *player2_board_x = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
	int *player2_board_y = (int *) malloc(sizeof(int)*NUMBERBOATS*5);
	fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 1, player2_board_x, sizeof(int)*NUMBERBOATS*5, &msglen));
	fprintf(stderr, "Awaiting ship placement... status: %d\n", server_recv(sv, 1, player2_board_y, sizeof(int)*NUMBERBOATS*5, &msglen));

	fprintf(stderr, "Selecting playing order...\n");

	fprintf(stderr, "Player %d was selected to go first\n", p1);
	int which = -1;
	while(!game_over(player1_hitvec, player2_hitvec)) {

		fprintf(stderr, "Sending player begin turn message... status: %d\n", server_send(sv, p1, &p_turn, sizeof(int)));
		fprintf(stderr, "Sending player wait turn message... status: %d\n", server_send(sv, p2, &p_not_turn, sizeof(int)));
		fprintf(stderr, "Awaiting player action response.. status: %d\n", server_recv(sv, p1, action, sizeof(int)*2, &msglen));
		
		if(p1 == 0)
			hit = check_hit(action, player2_board_x, player2_board_y, player2_hitvec, &which);
		else
			hit = check_hit(action, player1_board_x, player1_board_y, player1_hitvec, &which);
		fprintf(stderr, "%d\n", which);
		fprintf(stderr, "Sending hit info message... status: %d\n", server_send(sv, p1, &hit, sizeof(int)));
		fprintf(stderr, "Sending hit info message... status: %d\n", server_send(sv, p2, &hit, sizeof(int)));
		fprintf(stderr, "Sending hit location message... status: %d\n", server_send(sv, p2, action, sizeof(int)*2));
		fprintf(stderr, "Sending ship down message... status: %d\n", server_send(sv, p1, &which, sizeof(int)));
		fprintf(stderr, "Sending ship down message... status: %d\n", server_send(sv, p2, &which, sizeof(int)));

		if(game_over(player1_hitvec, player2_hitvec))
			over = TRUE;
		fprintf(stderr, "Sending game over status message... status: %d\n", server_send(sv, p1, &over, sizeof(int)));
		fprintf(stderr, "Sending game over status message... status: %d\n", server_send(sv, p2, &over, sizeof(int)));

		if(!game_over(player1_hitvec, player2_hitvec)) {
			fprintf(stderr, "Sending begin turn message... status: %d\n", server_send(sv, p2, &p_turn, sizeof(int)));
			fprintf(stderr, "Sending wait turn message... status: %d\n", server_send(sv, p1, &p_not_turn, sizeof(int)));
			fprintf(stderr, "Awaiting player action response.. status: %d\n", server_recv(sv, p2, action, sizeof(int)*2, &msglen));
			if(p2 == 0)
				hit = check_hit(action, player2_board_x, player2_board_y, player2_hitvec, &which);
			else
				hit = check_hit(action, player1_board_x, player1_board_y, player1_hitvec, &which);
			
			fprintf(stderr, "%d\n", which);
			fprintf(stderr, "Sending hit info message... status: %d\n", server_send(sv, p2, &hit, sizeof(int)));
			fprintf(stderr, "Sending hit info message... status: %d\n", server_send(sv, p1, &hit, sizeof(int)));
			fprintf(stderr, "Sending hit location message... status: %d\n", server_send(sv, p1, action, sizeof(int)*2));
			fprintf(stderr, "Sending ship down message... status: %d\n", server_send(sv, p1, &which, sizeof(int)));
			fprintf(stderr, "Sending ship down message... status: %d\n", server_send(sv, p2, &which, sizeof(int)));

			if(game_over(player1_hitvec, player2_hitvec))
				over = TRUE;
			fprintf(stderr, "Sending game over status message... status: %d\n", server_send(sv, p1, &over, sizeof(int)));
			fprintf(stderr, "Sending game over status message... status: %d\n", server_send(sv, p2, &over, sizeof(int)));
		}
	}

	server_close(sv);
	free(buf);

	return EXIT_SUCCESS;
}

int check_hit(int pos[2], int *board_x, int *board_y, int *hitvec, int *which) {
	for (int i = 0; i < NUMBERBOATS*5; i++) {
		fprintf(stderr, "(%d - %d) ", board_x[i], board_y[i]);
	}
	fprintf(stderr, "\n");

	int boats[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};
	for(int i=0; i < NUMBERBOATS; i++) {
		for(int j=0; j < boats[i]; j++) {
			if(board_x[i*5 + j] == pos[0] && board_y[i*5 +j] == pos[1]) {
				hitvec[i*5 + j] = TRUE;
				if (i == 0) {
					(*which) = is_down(hitvec, i) ? 0 : -1;
				}
				else if (i <= 2) {
					(*which) = is_down(hitvec, i) ? 1 : -1;
				}
				else if (i <= 5) {
					(*which) = is_down(hitvec, i) ? 2 : -1;
				}
				else if (i <= 9) {
					(*which) = is_down(hitvec, i) ? 3 : -1;
				}

				return TRUE;
			}
		}
	}
	return FALSE;
}

int is_down(int *hitvec, int boat) {
	int boats[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};

	for (int j = 0; j < boats[boat]; j++) {
		if (hitvec[boat*5 + j] == FALSE) {
			return FALSE;
		}
	}
	return TRUE;
}

int game_over(int *hitvec1, int *hitvec2) {
	int p1_over = TRUE;
	int p2_over = TRUE;

	int boats[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};

	for(int i=0; i < NUMBERBOATS; i++) {
		for(int j=0; j < boats[i]; j++) {
			if(hitvec1[i*5 + j] == FALSE) {
				p1_over = FALSE;
			}
			if(hitvec2[i*5 + j] == FALSE) {
				p2_over = FALSE;
			}
		}
	}

	return (p1_over || p2_over);
}

void addLog(char **log, char *message) {
	for (int i = 0; i < LOGSIZE-1; i++) {
		strcpy(log[i], log[i+1]);
	}

	strcpy(log[LOGSIZE-1], message);
}