#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "game/player.h"

void drawBoard(int **board, int y, int x) {
	// Draw board first line
	move(y, x);
	addch(ACS_ULCORNER);
	for(int i = 0; i < BOARDSIZEX-1; i++) {
		addch(ACS_HLINE);
		addch(ACS_TTEE);
	}
	addch(ACS_HLINE);
	addch(ACS_URCORNER);

	// Draw board middle lines
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
			if (board[i][j] == 0) // If square empty
				addch(' ');
			else if (board[i][j] == 1) // If square has ship
				addch('o');
			else if (board[i][j] == 2) // If already hit
				addch('x');
			else if (board[i][j] == 3) // If missed
				addch('.');
			else // Error
				addch('*');
		}
		addch(ACS_VLINE);
	}

	// Draw board last line
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
	// For each log draw line
	for (int i = 0; i < LOGSIZE; i++) {
		move(i, 0);
		wprintw(stdscr, "                                                  ");
		move(i, 0);
		wprintw(stdscr, "%s", log[i]);
	}
}

void drawWinScreen() {
	int row, col;

	clear();

	// Draw blinking outter border
	// box(stdscr, 0, 0);
	wborder(stdscr, ACS_VLINE | A_BLINK | COLOR_PAIR(1), ACS_VLINE | A_BLINK | COLOR_PAIR(1), ACS_HLINE | A_BLINK | COLOR_PAIR(1), ACS_HLINE | A_BLINK | COLOR_PAIR(1), ACS_ULCORNER | A_BLINK | COLOR_PAIR(1), ACS_URCORNER | A_BLINK | COLOR_PAIR(1), ACS_LLCORNER | A_BLINK | COLOR_PAIR(1), ACS_LRCORNER | A_BLINK | COLOR_PAIR(1));

	// Draw text
	getmaxyx(stdscr, row, col);

	move(row/2-6, col/2-23/2);
	wprintw(stdscr, " __     __             ");
	move(row/2-5, col/2-23/2);
	wprintw(stdscr, " \\ \\   / /             ");
	move(row/2-4, col/2-23/2);
	wprintw(stdscr, "  \\ \\_/ /__  _   _     ");
	move(row/2-3, col/2-23/2);
	wprintw(stdscr, "   \\   / _ \\| | | |    ");
	move(row/2-2, col/2-23/2);
	wprintw(stdscr, "    | | (_) | |_| |    ");
	move(row/2-1, col/2-23/2);
	wprintw(stdscr, " __ |_|\\___/ \\__,_|    ");
	move(row/2, col/2-23/2);
	wprintw(stdscr, " \\ \\        / (_)      ");
	move(row/2+1, col/2-23/2);
	wprintw(stdscr, "  \\ \\  /\\  / / _ _ __  ");
	move(row/2+2, col/2-23/2);
	wprintw(stdscr, "   \\ \\/  \\/ / | | '_ \\ ");
	move(row/2+3, col/2-23/2);
	wprintw(stdscr, "    \\  /\\  /  | | | | |");
	move(row/2+4, col/2-23/2);
	wprintw(stdscr, "     \\/  \\/   |_|_| |_|");
	move(row/2+5, col/2-16/2);
	wprintw(stdscr, "Press Q to exit.");
}

void drawLooseScreen() {
	int row, col;

	clear();

	// Draw blinking outter border
	// box(stdscr, 0, 0);
	wborder(stdscr, ACS_VLINE | A_BLINK | COLOR_PAIR(1), ACS_VLINE | A_BLINK | COLOR_PAIR(1), ACS_HLINE | A_BLINK | COLOR_PAIR(1), ACS_HLINE | A_BLINK | COLOR_PAIR(1), ACS_ULCORNER | A_BLINK | COLOR_PAIR(1), ACS_URCORNER | A_BLINK | COLOR_PAIR(1), ACS_LLCORNER | A_BLINK | COLOR_PAIR(1), ACS_LRCORNER | A_BLINK | COLOR_PAIR(1));

	// Draw text
	getmaxyx(stdscr, row, col);

	move(row/2-6, col/2-29/2);
	wprintw(stdscr, " __     __                   ");
	move(row/2-5, col/2-29/2);
	wprintw(stdscr, " \\ \\   / /                   ");
	move(row/2-4, col/2-29/2);
	wprintw(stdscr, "  \\ \\_/ /__  _   _           ");
	move(row/2-3, col/2-29/2);
	wprintw(stdscr, "   \\   / _ \\| | | |          ");
	move(row/2-2, col/2-29/2);
	wprintw(stdscr, "  _ | | (_) | |_| |          ");
	move(row/2-1, col/2-29/2);
	wprintw(stdscr, " | ||_|\\___/ \\__,_|          ");
	move(row/2, col/2-29/2);
	wprintw(stdscr, " | |     ___   ___  ___  ___ ");
	move(row/2+1, col/2-29/2);
	wprintw(stdscr, " | |    / _ \\ / _ \\/ __|/ _ \\");
	move(row/2+2, col/2-29/2);
	wprintw(stdscr, " | |___| (_) | (_) \\__ \\  __/");
	move(row/2+3, col/2-29/2);
	wprintw(stdscr, " |______\\___/ \\___/|___/\\___|");
	move(row/2+5, col/2-16/2);
	wprintw(stdscr, "Press Q to exit.");
}