

#ifndef GAME_H_
#define GAME_H_

#define BOARD_SIZE 8
#define PIECE_RADIUS 25
#define SQUARE_SIZE 60
#define HALF_SQUARE_SIZE 30
#define GRID_SIZE 480
#define LINE_WIDTH 4
#define BOARD_COLOR ((uint32_t) 0xFF4C9572)

#include "stdio.h"

uint8_t mat[BOARD_SIZE][BOARD_SIZE];

void drawGrid(void);

void init_game(void);

//imprime el tablero
void printBoard();

void flip(uint8_t player, uint8_t i, uint8_t j);

void getAvailableMoves(uint8_t player, uint8_t availablePosition[], uint8_t *numAvailablePosition);

int insertMove(uint8_t i, uint8_t j, uint8_t player, uint8_t availablePosition[], uint8_t numAvailablePosition);//como necesito de saber las jugadas en el tablero, paso la matrix por parametros

#endif /* GAME_H_ */
