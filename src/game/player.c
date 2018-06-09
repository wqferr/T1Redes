
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#define BOARDSIZEX 10
#define BOARDSIZEY 10
#define NUMBERBOATS 10

void drawBoard(int **board, int y, int x) {
	move(y, x);
	addch(ACS_ULCORNER);
	for(int i = 0; i < BOARDSIZEX-1; i++) {
		addch(ACS_HLINE);
		addch(ACS_TTEE);
	}
	addch(ACS_HLINE);
	addch(ACS_URCORNER);

	for(int i = 0; i < BOARDSIZEY; i++) {
		move(y+2*(i+1), x);
		
		addch(ACS_LTEE);
		for(int j = 0; j < BOARDSIZEX-1; j++) {
			addch(ACS_HLINE);
			addch(ACS_PLUS);
		}
		addch(ACS_HLINE);
		addch(ACS_RTEE);

		move(y+2*i+1, x);
		for(int j = 0; j < BOARDSIZEX; j++) {
			addch(ACS_VLINE);
			if (board[i][j] == 0)
				addch(' ');
			else if (board[i][j] == 1)
				addch('o');
			else
				addch('X');
		}
		addch(ACS_VLINE);
	}

	move(y+2*BOARDSIZEY, x);
	addch(ACS_LLCORNER);
	for(int i = 0; i < BOARDSIZEX-1; i++) {
		addch(ACS_HLINE);
		addch(ACS_BTEE);
	}
	addch(ACS_HLINE);
	addch(ACS_LRCORNER);
}

void drawDivision() {
	int row, col;

	getmaxyx(stdscr, row, col);

	for(int i = 0; i < 2*BOARDSIZEY+3; i++) {
		move(row/2-BOARDSIZEY-1+i, col/2);
		addch(ACS_VLINE | COLOR_PAIR(2));
	}
}

void drawSquare(int y, int x) {
	mvaddch(y  , x+1, ACS_HLINE | A_BLINK | COLOR_PAIR(1));
	mvaddch(y+1, x  , ACS_VLINE | A_BLINK | COLOR_PAIR(1));
	mvaddch(y+1, x+2, ACS_VLINE | A_BLINK | COLOR_PAIR(1));
	mvaddch(y+2, x+1, ACS_HLINE | A_BLINK | COLOR_PAIR(1));
}

void drawSquareSelected(int y, int x) {
	mvaddch(y  , x+1, ACS_HLINE | COLOR_PAIR(2));
	mvaddch(y+1, x  , ACS_VLINE | COLOR_PAIR(2));
	mvaddch(y+1, x+2, ACS_VLINE | COLOR_PAIR(2));
	mvaddch(y+2, x+1, ACS_HLINE | COLOR_PAIR(2));
}

int main(int argc, char const *argv[]) {
	int row, col, i = 0, j = 0, state = 0, selectx = 0, selecty = 0, now = 0, direction = 0;
	int boats[] = {5, 4, 4, 3, 3, 3, 2, 2, 2, 2};
	int **board;

	board = (int **) calloc(BOARDSIZEY, sizeof(int *));
	for (int k = 0; k < BOARDSIZEY; k++) {
		board[k] = (int *) calloc(BOARDSIZEX, sizeof(int));
	}

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

				if (invalid == 0) {
					if (direction == 0) {
						for (int k = 0; k < boats[now]; k++) {
							board[i+k][j] = 1;
						}
					} else if (direction == 1) {
						for (int k = 0; k < boats[now]; k++) {
							board[i-k][j] = 1;
						}
					} else if (direction == 2) {
						for (int k = 0; k < boats[now]; k++) {
							board[i][j+k] = 1;
						}
					} else if (direction == 3) {
						for (int k = 0; k < boats[now]; k++) {
							board[i][j-k] = 1;
						}
					}

					clear();
					drawDivision();

					state = 0;
					now++;

					if (now >= NUMBERBOATS) {
						state = 3;
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

	endwin();

	for (int k = 0; k < BOARDSIZEY; k++)
		free(board[k]);
	free(board);

	return 0;
}