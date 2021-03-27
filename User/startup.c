/*
 * startup.c
 *
 *  Created on: Jan 18, 2021
 *      Author: emile
 */
#include "main.h"
#include "cmsis_os.h"
#include "st_uart.h"

#include "debug.h"
#include "tasks.h"

#include <string.h>


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart6_rx;

tUartDevice m_uartDevConsole;


void user_startup()
{
	// Initalise debug console
	uartInit(&m_uartDevConsole, &huart1, &hdma_usart1_rx, NULL);
	debug_init(&m_uartDevConsole);

	task_gui(NULL);

	// Start RTOS tasks
	task_commsInit();

	task_guiInit();

}
