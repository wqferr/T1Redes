#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "game/player.h"

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
			else if (board[i][j] == 2)
				addch('x');
			else if (board[i][j] == 3)
				addch('.');
			else
				addch('*');
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

void drawLog(char **log) {
	for (int i = 0; i < LOGSIZE; i++) {
		move(i, 0);
		wprintw(stdscr, "%s", log[i]);
	}
}