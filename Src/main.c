/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_lcd.h"
#include "stm32f769i_discovery_ts.h"
#include "stdio.h"
#include "stm32f7xx_hal_adc.h"
#include "game.h"
#include "menu.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define VREF 3300
#define MAX_CONVERTED_VALUE 4095
#define VSENS_AT_AMBIENT_TEMP 760
#define AVG_SLOPE 25
#define AMBIENT_TEMP 25
#define BUTTON_INT PA0
#define x1Player 55
#define x2Players 550
#define xPlayerGame 250
#define y1Player 330
#define y2Players 330
#define yPlayerGame 180
#define width1Player 185
#define width2Players 185
#define widthPlayerGame 295
#define height1Player 65
#define height2Players 65
#define heightPlayerGame 70
#define backColor 0xFFAC7644
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA2D_HandleTypeDef hdma2d;
DSI_HandleTypeDef hdsi;
LTDC_HandleTypeDef hltdc;
SD_HandleTypeDef hsd2;
TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
SDRAM_HandleTypeDef hsdram1;

/* USER CODE BEGIN PV */
int ADC1value;
TS_StateTypeDef TS_State;
char auxStr[30];

uint8_t alreadyTouched=0;
uint8_t touchedX, touchedY;
uint8_t twoSecondsPass=0;
uint8_t numberPlayers = 2;
uint8_t counterTurn=20;
uint8_t passCounter1=0;
uint8_t passCounter2=0;
uint8_t counterMin = 0;

uint16_t touchedPosX, touchedPosY;
uint16_t counterGame=0;

char p1Name[30], p2Name [30];

// Fase 1 - main menu; fase 2 - jogo; fase 3 - pos juego
uint8_t programPhase = 1;
uint8_t resetPressed = 0;
uint8_t writeToFile = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_ADC1_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_SDMMC2_SD_Init(void);
/* USER CODE BEGIN PFP */

static void LCD_Config(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//Interrupción del ADC
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	ADC1value = HAL_ADC_GetValue(hadc);//Va a guardar el valor convertido de ADC, esta int va a ser llamada cuando el ADC termina la conversión
}

//Interrupción generada por el botón y para el touch screen
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_0)
	{
		resetPressed = 1;//flag que dice cargamos en el boton reset
		//programPhase = 1;//vuelve a la fase 1
		//touchedPosX = 0;
		//touchedPosY = 0;
	}

	if(GPIO_Pin == GPIO_PIN_13)
	{
		  BSP_TS_GetState(&TS_State);//funcion del sistema que va a obtener el estado del TS y lo va a guardar en el TS_State

		  if(TS_State.touchDetected >= 1 && alreadyTouched==0)
		  {
			  alreadyTouched=1;

			  // Lineas=Y; Columnas=X
			  // El tablero solo va estar entre 0 e 480 dividimos por 60 y va estar entre 0 e 7
			  if(TS_State.touchX[0] <= 480)
			  {
				  touchedX = TS_State.touchY[0]/60;
				  touchedY = TS_State.touchX[0]/60;
			  }
			  //guardamos la posición de 0 a 800(para el menú)
			  touchedPosX = TS_State.touchX[0];
			  touchedPosY = TS_State.touchY[0];

		  }
		  //protección toque continuo
		  else if(TS_State.touchDetected == 0)
		  {
			  alreadyTouched=0;
		  }
	}
}

//Interrupción de los timers
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)//utilizado para saber cuando pasan 2 segundos
	{
		twoSecondsPass = 1;
	}
	else if(htim->Instance == TIM7)//utilizado para cada segundo
	{
		counterTurn--;
		counterGame++;
	}
}

void readPlayers(void)
{
	 HAL_ADC_Stop_IT(&hadc1);

	char string[100] = {'\0'};
	UINT nBytes;
	uint8_t i = 0;

	 if( f_mount (&SDFatFS, SDPath, 0)!=FR_OK)
		 Error_Handler();

	 if(f_open(&SDFile, "Read.txt", FA_READ)!=FR_OK)
		 Error_Handler();

	 if(f_read(&SDFile, string, sizeof(string), &nBytes) !=FR_OK)
		 Error_Handler();

	 // split strings: strtok no funciona como debe ser
	 // lo que esta a hacer es poner un \0 donde aparece el \r
	 strtok(string, "\r");//strtok deberia retornar un puntero para donde aparece el \r

	 strcpy(p1Name, string);

	 char *aux;
	 for(aux = string; *aux !='\0'; aux++);

	 // mover para despues del primer \0 y para despues del \n
	 aux++;
	 aux++;

	 strcpy(p2Name, aux);

	 f_close(&SDFile);

	  HAL_ADC_Start_IT(&hadc1);
}

//retorna 1 si tocamos dentro del rectangulo y 0 en caso contrario
uint8_t insideRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
	if(touchedPosX >= x && touchedPosX <= x+width)
	{
		if(touchedPosY >= y && touchedPosY <= y+height)
		{
			return 1;
		}
	}

	return 0;

}

//función del menú de inicio, en ella vemos si el utilizador cargó dentro de algún rectangulo
void mainMenu(void)
{
	char tempStr1[10];

	if(insideRectangle(xPlayerGame, yPlayerGame, widthPlayerGame, heightPlayerGame)==1)
	{
    	BSP_LCD_Clear(LCD_COLOR_WHITE);

		readPlayers();

		if(numberPlayers == 1)
		{
			strcpy(p2Name, "ARM");
		}

		init_game(p1Name, p2Name);
		programPhase=2;
		counterGame = 0;
		passCounter1 = 0;
		passCounter2 = 0;
	}
	else if (insideRectangle(x1Player, y1Player, width1Player, height1Player)==1)
	{
		numberPlayers=1;

		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		sprintf(tempStr1, "1 PLAYER");
		BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) tempStr1, LEFT_MODE);

		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		sprintf(tempStr1, "2 PLAYERS");
		BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) tempStr1, RIGHT_MODE);

	}
	else if (insideRectangle(x2Players, y2Players, width2Players, height2Players)==1)
	{
		numberPlayers=2;

		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		sprintf(tempStr1, "1 PLAYER");
		BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) tempStr1, LEFT_MODE);

		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		sprintf(tempStr1, "2 PLAYERS");
		BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) tempStr1, RIGHT_MODE);
	}
}

//imprime el menú principal
void printMainMenu(void)
{
	BSP_LCD_SetBackColor(backColor);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

    BSP_LCD_DrawBitmap(0, 0, image);

	BSP_LCD_DisplayStringAt(0, 10, (uint8_t *)"REVERSI", CENTER_MODE);//funcion quiere uint8_t

	////////////////////////////////////
	//JUGAR JUEGO

	sprintf(auxStr, "PLAY GAME");
	BSP_LCD_DisplayStringAt(0, 200, (uint8_t*) auxStr, CENTER_MODE);

	////////////////////////////////////
	//1 JUGADOR

	sprintf(auxStr, "1 PLAYER");
	BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) auxStr, LEFT_MODE);

	////////////////////////////////////
	//2 JUGADORES

	sprintf(auxStr, "2 PLAYERS");
	BSP_LCD_DisplayStringAt(80, 350, (uint8_t*) auxStr, RIGHT_MODE);
}

//función que imprime el tiempo de jugada y el tiempo total
void printTime(void)
{
	BSP_LCD_SetTextColor(LCD_COLOR_RED);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

	sprintf(auxStr, "Time remaining: %.2d", counterTurn);
	BSP_LCD_DisplayStringAt(3, LINE(18), (uint8_t*) auxStr, RIGHT_MODE);

	counterMin = counterGame/60;

	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

	sprintf(auxStr, "Total time: %.2d:%.2d", counterMin, counterGame %60);//modulo de los segundos, que es el resto de la division entera
	BSP_LCD_DisplayStringAt(10, LINE(11), (uint8_t*) auxStr, RIGHT_MODE);
}

//función que imprime la temperatura en la pantalla actualizada cada dos segundos
void printTemperature(void)
{
	int temperature;

	if(twoSecondsPass == 1)
    {
	    temperature = ((((ADC1value * VREF)/MAX_CONVERTED_VALUE) - VSENS_AT_AMBIENT_TEMP) * 10 / AVG_SLOPE) + AMBIENT_TEMP;//convierte el valor de ADC para temperatura y lo guarda

	    // Display temperature on the lcd

	    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

    	sprintf(auxStr, "Temperature %d C", temperature);
	    BSP_LCD_DisplayStringAt(20, LINE(3), (uint8_t*) auxStr, RIGHT_MODE);

	    twoSecondsPass = 0;
    }
}

//función importante del programa donde pasa todo el juego
uint8_t mainCycle(void)
{
    uint8_t playeri, playerj;
    uint8_t availablePosition[8*8], numAvailablePosition;

	////////////////////////////
	// Player 1

	uint8_t validPosition = 0;
	getAvailableMoves(1, availablePosition, &numAvailablePosition);//recibe el jugador, las posiciones disponibles y el numero de pos. disponibles

	if(numAvailablePosition == 0)
	{
		return 0;// 0 significa que no hay posiciones disponibles
	}

	sprintf(auxStr, "%s Turn", p1Name);
	BSP_LCD_DisplayStringAt(45, LINE(17), (uint8_t*) auxStr, RIGHT_MODE);
	counterTurn = 20;

	while (validPosition == 0)//mientras la posicion del usuario no fuera valida permanece en el while
	{
		printTime();
		  if(insideRectangle(x2Players, y2Players, width2Players, height2Players)==1)
		  {
				counterTurn = 20;
		  }
		printTemperature();

		if(counterTurn == 0)
		{
			passCounter1++;

			if(passCounter1 == 3)
			{
				return 1;//para ser diferente a 0, sale debido a que ya pasó 3 turnos
			}

			break;//cuando pasa un turno sale del ciclo while para continuar para el jugador 2
		}

		if(resetPressed == 1)//en caso de presionar el boton del reset sale de la función
		{
			  resetPressed = 0;
			  break;
		}

		// VER POSICION EN LA TOUCH SCREEN
		playeri = touchedX;//ahora son lineas, cambiamos arriba
		playerj = touchedY;//columnas

		validPosition = insertMove(playeri, playerj, 1, availablePosition, numAvailablePosition);//si consigue insertar validPosition es 1, en caso contrario es 0
	}

	printInfo(p1Name, p2Name);
	printBoard();//vuelve a imprimir el tablero


	////////////////////////////
	// Player 2

	validPosition = 0;
	counterTurn = 20;
	getAvailableMoves(2, availablePosition, &numAvailablePosition);

	if(numAvailablePosition == 0)
	{
		return 0;
	}

	sprintf(auxStr, "%s Turn", p2Name);
	BSP_LCD_DisplayStringAt(45, LINE(17), (uint8_t*) auxStr, RIGHT_MODE);

	while (validPosition == 0)
	{
		printTime();
		  if(insideRectangle(x2Players, y2Players, width2Players, height2Players)==1)
		  {
				counterTurn = 20;
		  }
		printTemperature();

		if(counterTurn == 0)
		{
			passCounter2++;

			if(passCounter2 == 3)
			{
				return 1;
			}

			break;
		}

		if(resetPressed == 1)
		{
			  resetPressed = 0;
			  break;
		}

		if(numberPlayers == 2)
		{
			playeri = touchedX;
			playerj = touchedY;
		}
		else // AI player
		{
			HAL_Delay(1000);
			uint8_t selectedPosition = availablePosition[0];

			//convierte selectedPosition en dos posiciones, linea y columna (de 0 a 7)
			playeri = (selectedPosition / 10) - 1;//la función insertMove requiere los parametros separados
			playerj = (selectedPosition % 10) - 1;

		}

		validPosition = insertMove(playeri, playerj, 2, availablePosition, numAvailablePosition);
	}

	printInfo(p1Name, p2Name);
	printBoard();//vuelve a imprimir el tablero

	return numAvailablePosition;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t player1Counter, player2Counter;
  uint8_t winner;
  uint nBytes;

  /* USER CODE END 1 */
  

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA2D_Init();
  MX_FMC_Init();
  MX_LTDC_Init();
  MX_ADC1_Init();
  MX_DSIHOST_DSI_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_SDMMC2_SD_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  LCD_Config();
  HAL_ADC_Start_IT(&hadc1);
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  BSP_TS_ITConfig();
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_TIM_Base_Start_IT(&htim7);
 // BSP_PB_Init(BUTTON_INT, BUTTON_MODE_EXTI);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  //if(programPhase == 1)
	 //{


		  mainMenu();//llamamos a esta función para ver si la persona carga en algún sitio
	  //}
	  if(programPhase == 2)
	  {
		  ////////////////////////////////////
		  // TOUCH SCREEN


		  if(mainCycle() == 0)//No más movimientos
		  {

			  BSP_LCD_SetTextColor(LCD_COLOR_RED);
			  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);

			  sprintf(auxStr, "GAME OVER!");
			  BSP_LCD_DisplayStringAt(65, LINE(14), (uint8_t*) auxStr, RIGHT_MODE);

			  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);

			  countPieces(&player1Counter, &player2Counter);
			  if(player1Counter > player2Counter)
			  {
				 winner = 1;
				 sprintf(auxStr, "Winner = %s", p1Name);

			  }
			  else if(player1Counter < player2Counter)
			  {
				 winner= 2;
				 sprintf(auxStr,  "Winner = %s", p2Name);
			  }
			  else if(player1Counter == player2Counter)
			  {
				  winner=0;
				  sprintf(auxStr, "It's a tie");
			  }

			  BSP_LCD_DisplayStringAt(10, LINE(15), (uint8_t*) auxStr, RIGHT_MODE);

			  programPhase=3;
			  writeToFile = 1;
		  }

		  if(passCounter1 == 3)
		  {
			  sprintf(auxStr, "GAME OVER!");
			  BSP_LCD_DisplayStringAt(65, LINE(14), (uint8_t*) auxStr, RIGHT_MODE);
			  sprintf(auxStr,  "Winner = %s", p2Name);
			  BSP_LCD_DisplayStringAt(10, LINE(15), (uint8_t*) auxStr, RIGHT_MODE);

			  winner = 2;
			  programPhase = 3;
			  writeToFile = 1;
		  }
		  else if(passCounter2 == 3)
		  {
			  sprintf(auxStr, "GAME OVER!");
			  BSP_LCD_DisplayStringAt(65, LINE(14), (uint8_t*) auxStr, RIGHT_MODE);
			  sprintf(auxStr,  "Winner = %s", p1Name);
			  BSP_LCD_DisplayStringAt(10, LINE(15), (uint8_t*) auxStr, RIGHT_MODE);

			  winner = 1;
			  programPhase =3;
			  writeToFile = 1;
		  }
	   }

	  if(programPhase==3)
	  {
		  printTemperature();

		  if(writeToFile == 1)//para no estar siempre escribiendo en el fichero
		  {
			 writeToFile = 0;

			 // Desacctivación de la interrupción ADC para poder escribir en el SD card
			  HAL_ADC_Stop_IT(&hadc1);

			 char auxStr2[150];

			 FRESULT res;

			  res = f_mount (&SDFatFS, SDPath, 0);
		      if(res != FR_OK)
			  {
				sprintf(auxStr, "Error in fmount: %d", res);
				Error_Handler();
				break;
			  }

		      res = f_open(&SDFile, "Rev.txt", FA_CREATE_ALWAYS | FA_WRITE);
			  if(res != FR_OK)
			  {
				sprintf(auxStr, "Error in fopen: %d", res);
				Error_Handler();
				break;
			  }

			  if(winner == 1)
			  {
				  sprintf(auxStr2, "Winner = %s\nPieces %s = %.2d\nPieces %s = %.2d\nTotal time: %.2d:%.2d\n", p1Name, p1Name, player1Counter, p2Name, player2Counter, counterMin, counterGame %60);
			  }
			  else if( winner == 2)
			  {
				  sprintf(auxStr2, "Winner = %s\nPieces %s = %.2d\nPieces %s = %.2d\nTotal time: %.2d:%.2d\n", p2Name, p1Name, player1Counter, p2Name, player2Counter, counterMin, counterGame %60);
			  }
			  else if(winner == 0)
			  {
				  sprintf(auxStr2, "It's a tie\nPieces %s = %.2d\nPieces %s = %.2d\nTotal time: %.2d:%.2d\n",  p1Name, player1Counter, p2Name, player2Counter, counterMin, counterGame %60);
		      }

			  res = f_write(&SDFile, auxStr2, strlen(auxStr2), &nBytes);
			  if(res != FR_OK)
			  {
				sprintf(auxStr, "Error in fwrite: %d", res);
				Error_Handler();
				break;
			  }

			  res = f_close(&SDFile);
			  if(res != FR_OK)
			  {
				sprintf(auxStr, "Error in fclose: %d", res);
				Error_Handler();
				break;
			  }

			  // Activación interrupción ADC
			 HAL_ADC_Start_IT(&hadc1);
		  }
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Activate the Over-Drive mode 
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_SDMMC2
                              |RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV2;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
  PeriphClkInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{

  /* USER CODE BEGIN DSIHOST_Init 0 */

  /* USER CODE END DSIHOST_Init 0 */

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_HOST_TimeoutTypeDef HostTimeouts = {0};
  DSI_PHY_TimerTypeDef PhyTimings = {0};
  DSI_LPCmdTypeDef LPCmd = {0};
  DSI_CmdCfgTypeDef CmdCfg = {0};

  /* USER CODE BEGIN DSIHOST_Init 1 */

  /* USER CODE END DSIHOST_Init 1 */
  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_ONE_DATA_LANE;
  PLLInit.PLLNDIV = 20;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV1;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;
  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }
  HostTimeouts.TimeoutCkdiv = 1;
  HostTimeouts.HighSpeedTransmissionTimeout = 0;
  HostTimeouts.LowPowerReceptionTimeout = 0;
  HostTimeouts.HighSpeedReadTimeout = 0;
  HostTimeouts.LowPowerReadTimeout = 0;
  HostTimeouts.HighSpeedWriteTimeout = 0;
  HostTimeouts.HighSpeedWritePrespMode = DSI_HS_PM_DISABLE;
  HostTimeouts.LowPowerWriteTimeout = 0;
  HostTimeouts.BTATimeout = 0;
  if (HAL_DSI_ConfigHostTimeouts(&hdsi, &HostTimeouts) != HAL_OK)
  {
    Error_Handler();
  }
  PhyTimings.ClockLaneHS2LPTime = 28;
  PhyTimings.ClockLaneLP2HSTime = 33;
  PhyTimings.DataLaneHS2LPTime = 15;
  PhyTimings.DataLaneLP2HSTime = 25;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 0;
  if (HAL_DSI_ConfigPhyTimer(&hdsi, &PhyTimings) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigFlowControl(&hdsi, DSI_FLOW_CONTROL_BTA) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetLowPowerRXFilter(&hdsi, 10000) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_ConfigErrorMonitor(&hdsi, HAL_DSI_ERROR_NONE) != HAL_OK)
  {
    Error_Handler();
  }
  LPCmd.LPGenShortWriteNoP = DSI_LP_GSW0P_DISABLE;
  LPCmd.LPGenShortWriteOneP = DSI_LP_GSW1P_DISABLE;
  LPCmd.LPGenShortWriteTwoP = DSI_LP_GSW2P_DISABLE;
  LPCmd.LPGenShortReadNoP = DSI_LP_GSR0P_DISABLE;
  LPCmd.LPGenShortReadOneP = DSI_LP_GSR1P_DISABLE;
  LPCmd.LPGenShortReadTwoP = DSI_LP_GSR2P_DISABLE;
  LPCmd.LPGenLongWrite = DSI_LP_GLW_DISABLE;
  LPCmd.LPDcsShortWriteNoP = DSI_LP_DSW0P_DISABLE;
  LPCmd.LPDcsShortWriteOneP = DSI_LP_DSW1P_DISABLE;
  LPCmd.LPDcsShortReadNoP = DSI_LP_DSR0P_DISABLE;
  LPCmd.LPDcsLongWrite = DSI_LP_DLW_DISABLE;
  LPCmd.LPMaxReadPacket = DSI_LP_MRDP_DISABLE;
  LPCmd.AcknowledgeRequest = DSI_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigCommand(&hdsi, &LPCmd) != HAL_OK)
  {
    Error_Handler();
  }
  CmdCfg.VirtualChannelID = 0;
  CmdCfg.ColorCoding = DSI_RGB888;
  CmdCfg.CommandSize = 640;
  CmdCfg.TearingEffectSource = DSI_TE_EXTERNAL;
  CmdCfg.TearingEffectPolarity = DSI_TE_RISING_EDGE;
  CmdCfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;
  CmdCfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;
  CmdCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  CmdCfg.VSyncPol = DSI_VSYNC_FALLING;
  CmdCfg.AutomaticRefresh = DSI_AR_ENABLE;
  CmdCfg.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_DISABLE;
  if (HAL_DSI_ConfigAdaptedCommandMode(&hdsi, &CmdCfg) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DSI_SetGenericVCID(&hdsi, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DSIHOST_Init 2 */

  /* USER CODE END DSIHOST_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};
  LTDC_LayerCfgTypeDef pLayerCfg1 = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 7;
  hltdc.Init.VerticalSync = 3;
  hltdc.Init.AccumulatedHBP = 14;
  hltdc.Init.AccumulatedVBP = 5;
  hltdc.Init.AccumulatedActiveW = 654;
  hltdc.Init.AccumulatedActiveH = 485;
  hltdc.Init.TotalWidth = 660;
  hltdc.Init.TotalHeigh = 487;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 0;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 0;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg.Alpha = 0;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 0;
  pLayerCfg.ImageHeight = 0;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg1.WindowX0 = 0;
  pLayerCfg1.WindowX1 = 0;
  pLayerCfg1.WindowY0 = 0;
  pLayerCfg1.WindowY1 = 0;
  pLayerCfg1.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  pLayerCfg1.Alpha = 0;
  pLayerCfg1.Alpha0 = 0;
  pLayerCfg1.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg1.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg1.FBStartAdress = 0;
  pLayerCfg1.ImageWidth = 0;
  pLayerCfg1.ImageHeight = 0;
  pLayerCfg1.Backcolor.Blue = 0;
  pLayerCfg1.Backcolor.Green = 0;
  pLayerCfg1.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg1, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SDMMC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SDMMC2_SD_Init(void)
{

  /* USER CODE BEGIN SDMMC2_Init 0 */

  /* USER CODE END SDMMC2_Init 0 */

  /* USER CODE BEGIN SDMMC2_Init 1 */

  /* USER CODE END SDMMC2_Init 1 */
  hsd2.Instance = SDMMC2;
  hsd2.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd2.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd2.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd2.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd2.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd2.Init.ClockDiv = 0;
  /* USER CODE BEGIN SDMMC2_Init 2 */

  /* USER CODE END SDMMC2_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 19999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 9999;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 9999;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank = FMC_SDRAM_BANK2;
  hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_13;
  hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_1;
  hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_DISABLE;
  hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_DISABLE;
  hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
  /* SdramTiming */
  SdramTiming.LoadToActiveDelay = 16;
  SdramTiming.ExitSelfRefreshDelay = 16;
  SdramTiming.SelfRefreshTime = 16;
  SdramTiming.RowCycleDelay = 16;
  SdramTiming.WriteRecoveryTime = 16;
  SdramTiming.RPDelay = 16;
  SdramTiming.RCDDelay = 16;

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK)
  {
    Error_Handler( );
  }

  /* USER CODE BEGIN FMC_Init 2 */

  /* USER CODE END FMC_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();

  /*Configure GPIO pin : PI13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PI15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */
static void LCD_Config(void)
{
  uint32_t  lcd_status;

  /* Initialize the LCD */
  lcd_status = BSP_LCD_Init();
  while(lcd_status != LCD_OK);//si LCD_init no fuera ok estaría en ciclo infinito

  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);

  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  printMainMenu();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	BSP_LCD_DisplayStringAt(10, LINE(15), (uint8_t*) auxStr, CENTER_MODE);

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
