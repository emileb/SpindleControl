/*
 * task_comms.c
 *
 *  Created on: Jan 18, 2021
 *      Author: emile
 */

#include <stdbool.h>
#include <stdint.h>

#include "cmsis_os.h"
#include "debug.h"
#include "modbus.h"

extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

static tUartDevice m_uartDevSpindle;
static tModBusDevice m_modbusSpindle;

static osThreadId_t m_thread_id;

static const osThreadAttr_t m_attributes =
{ .name = "task_comms", .priority = (osPriority_t) osPriorityNormal, .stack_size = 4096, };

static void task_comms(void *argument);

void task_commsInit()
{
	m_thread_id = osThreadNew(task_comms, NULL, &m_attributes);
	//task_comms(0);
}

int speed;

static void task_comms(void *argument)
{

	PRINTF("task_comms STARTED\r\n");

	uartInit(&m_uartDevSpindle, &huart6, &hdma_usart6_rx, &hdma_usart6_tx);
	uartStartRx(&m_uartDevSpindle);

	modbus_init(&m_modbusSpindle, &m_uartDevSpindle, 1);

	modbus_sendReadReg(&m_modbusSpindle, 1, 0x178);


	//modbus_sendWriteReg(&m_modbusSpindle, 169, 100);

	while (true)
	{
		if (modbus_checkReceive(&m_modbusSpindle))
		{
			speed = (m_modbusSpindle.receiveData[m_modbusSpindle.dataStart] << 8)
					| m_modbusSpindle.receiveData[m_modbusSpindle.dataStart + 1];

			PRINTF("Speed = %d\r\n", speed);

			modbus_sendReadReg(&m_modbusSpindle, 1, 0x172);
		}
		//PRINTF("tick\r\n");
		osDelay(10);
	}
}
