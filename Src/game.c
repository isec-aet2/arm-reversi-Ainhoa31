

#include "game.h"
#include "stm32f769i_discovery_lcd.h"

void drawGrid(void)
{
	uint8_t offset;

	// Draw the background
	BSP_LCD_SetTextColor(BOARD_COLOR);
	BSP_LCD_FillRect(0, 0, GRID_SIZE, GRID_SIZE);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

	for(uint8_t i=0; i<=BOARD_SIZE; i++)
	{

		// Para i=0 el offset seria -2
		if(i == 0)
		{
			offset = 0;
		}
		else
		{
			offset = LINE_WIDTH/2;
		}

		// Horizontal
		BSP_LCD_FillRect(0, (i * SQUARE_SIZE)-offset, GRID_SIZE, LINE_WIDTH);

		// Vertical
		BSP_LCD_FillRect((i * SQUARE_SIZE)-offset, 0, LINE_WIDTH, GRID_SIZE);
	}
}

void gameTitle(void)
{
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  BSP_LCD_FillRect(480, 0, 320, 50);

  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  BSP_LCD_DisplayStringAt(100, 10, (uint8_t *)"REVERSI", RIGHT_MODE); //función quiere uint8_t
}

void init_game(char *p1Name, char*p2Name)
{
	gameTitle();
	drawGrid();

    // Clear the board
    for(uint8_t i=0; i<BOARD_SIZE; i++)
    {
        for(uint8_t j=0; j<BOARD_SIZE; j++)
        {
             mat[i][j] = 0;
        }
    }

    mat[3][3] = 1;
    mat[4][4] = 1;

    mat[4][3] = 2;
    mat[3][4] = 2;

	printInfo(p1Name, p2Name);
    printBoard();
}

//imprime el tablero
void printBoard(void)
{
	uint8_t i, j;
	uint8_t value;

    for (i = 0; i < BOARD_SIZE; i++)
    {
        for (j = 0; j < BOARD_SIZE; j++)
        {
            value = mat[i][j]; //para leer que está almacenado en la matriz

            if (value == 1)
            {
            	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            	BSP_LCD_FillCircle((j*SQUARE_SIZE)+HALF_SQUARE_SIZE, (i*SQUARE_SIZE)+HALF_SQUARE_SIZE, PIECE_RADIUS);
            }
            else if (value == 2)
            {
        		 BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
             	 BSP_LCD_FillCircle((j*SQUARE_SIZE)+HALF_SQUARE_SIZE, (i*SQUARE_SIZE)+HALF_SQUARE_SIZE, PIECE_RADIUS);
            }
        }
    }
}

void printInfo(char * p1Name, char * p2Name)
{
	uint8_t player1Counter, player2Counter;
	char pieces[20];
	char information[20];

    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_FillRect(485, LINE(5), BSP_LCD_GetXSize()-485, 350);

    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_DrawHLine(480, LINE(5), 420);

    countPieces(&player1Counter, &player2Counter);

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

	sprintf(information, "GAME INFORMATION");
	BSP_LCD_DisplayStringAt(20, LINE(6), (uint8_t*) information, RIGHT_MODE);

	sprintf(pieces, "Pieces %s=%.2d", p1Name, player1Counter);
	BSP_LCD_DisplayStringAt(5, LINE(8), (uint8_t*) pieces, RIGHT_MODE);

	sprintf(pieces, "Pieces %s=%.2d", p2Name, player2Counter);
	BSP_LCD_DisplayStringAt(5, LINE(9), (uint8_t*) pieces, RIGHT_MODE);

}

//hace el recuento despues de cada movimiento de las fichas de cada jugador
void countPieces (uint8_t *player1Counter, uint8_t *player2Counter)
{
    *player1Counter=0;
    *player2Counter=0;

    for(uint8_t i=0; i<BOARD_SIZE; i++)
    {
        for(uint8_t j=0; j<BOARD_SIZE; j++)
        {
            if(mat[i][j] == 1)
            {
                (*player1Counter)++;
            }
            else if(mat[i][j] == 2)
            {
                (*player2Counter)++;
            }
        }
    }
}

//cambia la pieza del jugador opuesto
//asumese que i e j ya son contados desde 0 hasta 7 en lugar de 1 a 8
void flip(uint8_t player, uint8_t i, uint8_t j)
{
    uint8_t oppositePlayer;

    if(player==1)
    {
        oppositePlayer=2;
    }
    else
    {
        oppositePlayer=1;
    }

    /* TL   T   TR
     *  L (i,j) R
     * BL   B   BR
     */

    // top left
    if(i > 0 && j > 0)
    {
        if(mat[i-1][j-1] == oppositePlayer)
        {
            // posicao adjacent
        	int8_t startI = i - 1;
            int8_t startJ = j - 1;

            while (startI >= 0 && startJ >= 0)//para no salir fuera de la raiz
            {
                if(mat [startI][startJ]==player)
                {
                    while(startI != i && startJ!=j)
                    {
                        mat[startI][startJ] = player;//Aqui muda las fichas

                        startI++;
                        startJ++;
                    }
                    break; // sale del while porque ya sabemos que esta posicion es valida
                }

                startI--;
                startJ--;
            }
        }
    }

    // top
    if(i>0)
    {
        if(mat[i-1][j] == oppositePlayer)
        {
        	int8_t startI = i - 1;
            int8_t startJ = j;

            while (startI >= 0)
            {
                if(mat [startI][startJ]==player)
                {
                    while(startI != i)
                    {
                        mat[startI][startJ] = player;

                        startI++;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startI--;
            }
        }
    }

    // top right
    if(i > 0 && j <7)
    {
        if(mat[i-1][j+1] == oppositePlayer)
        {
        	int8_t startI = i - 1;
            int8_t startJ = j + 1;

            while (startI >= 0 && startJ <= 7)
            {
                if(mat [startI][startJ]==player)
                {
                    while(startI != i && startJ!=j)
                    {
                        mat[startI][startJ] = player;

                        startI++;
                        startJ--;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startI--;
                startJ++;
            }
        }
    }

    // right
    if(j < 7)
    {
        if(mat[i][j+1] == oppositePlayer)
        {
        	int8_t startI = i;
            int8_t startJ = j + 1;

            while (startJ <= 7)
            {
                if(mat [startI][startJ]==player)
                {
                    while(startJ!=j)
                    {
                        mat[startI][startJ] = player;

                        startJ--;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startJ++;
            }
        }
    }

    // bottom right
    if (i < 7 && j < 7)
    {
        if(mat[i+1][j+1] == oppositePlayer)
        {
        	int8_t startI = i + 1;
            int8_t startJ = j + 1;

            while (startI <= 7 && startJ <= 7)
            {
                if(mat [startI][startJ]==player)
                {
                     while(startI != i && startJ!=j)
                    {
                        mat[startI][startJ] = player;

                        startI--;
                        startJ--;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startI++;
                startJ++;
            }
        }
    }

    // bottom
    if(i < 7)
    {
            if(mat[i+1][j] == oppositePlayer)
            {
            	int8_t startI = i + 1;
                int8_t startJ = j;

                while (startI <=7)
                {
                    if(mat [startI][startJ]==player)
                    {
                        while(startI != i)
                        {
                            mat[startI][startJ] = player;

                            startI--;
                         }
                         break; // sale del while porque ya sabemos que esta posicion es valida
                    }
                    startI++;
                }
            }
    }

    // bottom left
    if(i < 7 && j > 0)
    {
        if(mat[i+1][j-1] == oppositePlayer)
        {
        	int8_t startI = i + 1;
            int8_t startJ = j - 1;

            while (startI <=7  && startJ >= 0)
            {
                if(mat [startI][startJ]==player)
                {
                     while(startI != i && startJ!=j)
                    {
                        mat[startI][startJ] = player;

                        startI--;
                        startJ++;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startI++;
                startJ--;
            }
        }
    }


    // left
    if(j > 0)
    {
        if(mat[i][j-1] == oppositePlayer)
        {
        	int8_t startI = i;
        	int8_t startJ = j - 1;

            while ( startJ >= 0)
            {
                if(mat [startI][startJ]==player)
                {
                     while(startJ!=j)
                    {
                        mat[startI][startJ] = player;

                        startJ++;
                    }
                     break; // sale del while porque ya sabemos que esta posicion es valida
                }
                startJ--;
            }
        }
    }
}

// Busca en el tablero entero los movimientos validos
void getAvailableMoves(uint8_t player, uint8_t availablePosition[], uint8_t *numAvailablePosition)
{
	uint8_t oppositePlayer;

    if(player==1)
    {
        oppositePlayer=2;
    }
    else
    {
        oppositePlayer=1;
    }


    *numAvailablePosition = 0;//todavia no sabemos cuantas hay por eso la igualamos a  0

    for(int i=0; i<8; i++)
    {
        for(int j=0; j<8; j++)
        {
            //Si encontramos en una posicion i,j algo diferente sabemos que no esta disponible y que tiene que pasar a la prox posicion
            if(mat[i][j] != 0)
                continue;

            /* TL   T   TR
             *  L (i,j) R
             * BL   B   BR
             */

            uint8_t positionValid = 0;//al inicio declaramos que la posición en invalida

            // top left
            if(i > 0 && j > 0) // Esta condición sirve para no acceder a posiciones fuera de la tabla
            {
                if(mat[i-1][j-1] == oppositePlayer)
                {
                    // posicao adjacent
                	int8_t startI = i - 1;//comienza en el mismo sitio de la ficha opuesta
                	int8_t startJ = j - 1;

                    while (startI >= 0 && startJ >= 0)//hasta los limites de la tabla
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI--;
                        startJ--;
                    }
                }
            }

            // top
            if(i>0)
            {
                if(mat[i-1][j] == oppositePlayer)
                {
                    int8_t startI = i - 1;
                    int8_t startJ = j;

                    while (startI >= 0)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI--;
                    }
                }
            }

            // top right
            if(i > 0 && j <7)
            {
                if(mat[i-1][j+1] == oppositePlayer)
                {
                	int8_t startI = i - 1;
                	int8_t startJ = j + 1;

                    while (startI >= 0 && startJ <= 7)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI--;
                        startJ++;
                    }
                }
            }

            // right
            if(j < 7)
            {
                if(mat[i][j+1] == oppositePlayer)
                {
                	int8_t startI = i;
                	int8_t startJ = j + 1;

                    while (startJ <= 7)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startJ++;
                    }
                }
            }

            // bottom right
            if (i < 7 && j < 7)
            {
                if(mat[i+1][j+1] == oppositePlayer)
                {
                	int8_t startI = i + 1;
                	int8_t startJ = j + 1;

                    while (startI <= 7 && startJ <= 7)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI++;
                        startJ++;
                    }
                }
            }

            // bottom
            if(i < 7)
            {
                if(mat[i+1][j] == oppositePlayer)
                {
                	int8_t startI = i + 1;
                	int8_t startJ = j;

                    while (startI <=7)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI++;
                    }
                }
            }

            // bottom left
            if(i < 7 && j > 0)
            {
                if(mat[i+1][j-1] == oppositePlayer)
                {
                	int8_t startI = i + 1;
                	int8_t startJ = j - 1;

                    while (startI <=7  && startJ >= 0)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startI++;
                        startJ--;
                    }
                }
            }

            // left
            if(j > 0)
            {
                if(mat[i][j-1] == oppositePlayer)
                {
                	int8_t startI = i;
                	int8_t startJ = j - 1;

                    while ( startJ >= 0)
                    {
                        if(mat [startI][startJ]==player)
                        {
                             positionValid = 1;
                             break; // sale del while porque ya sabemos que esta posicion es valida
                        }
                        startJ--;
                    }
                }
            }

            if (positionValid == 1)
            {
                *availablePosition=(i+1)*10+j+1;//almacenar esta posicion que esta disponible, podria utilizar otro puntero para j
                availablePosition++;//para que la proxima vez que encontraras una posición disponible la almacenaras en el enderezo de memoria

                (*numAvailablePosition)++;
            }
        }
    }
}


//inserta una posición en el tablero para el jugador, pasada por parámetro(player) si la posición fuera valida
//retorna 0 cuando la posición es invalida, retorna 1 cuando es valida
int insertMove(uint8_t i, uint8_t j, uint8_t player, uint8_t availablePosition[], uint8_t numAvailablePosition)//como necesito de saber las jugadas en el tablero, paso la matrix por parametros
{
	uint8_t n = (i+1)*10+(j+1);

    for(uint8_t k=0; k<numAvailablePosition; k++)
    {
        if(availablePosition[k]==n)//quiero saber si el valor que la persona escribio esta en las posiciones disponibles
        {
            mat[i][j] = player;
            flip(player, i, j);
            return 1;
        }
    }

    return 0;
}
