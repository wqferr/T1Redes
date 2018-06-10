#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <ncurses.h>

#define BOARDSIZEX 10
#define BOARDSIZEY 10
#define NUMBERBOATS 10

void drawBoard(int **board, int y, int x);
void drawDivision();
void drawSquare(int y, int x);
void drawSquareSelected(int y, int x);

#endif
