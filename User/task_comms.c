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
#include "user_input.h"
#include "motor_status.h"

#define TASK_DELAY 10 // Task delay time in milliseconds


static bool m_ready = false; // When ready is false we are not allowed to send a non-zero speed. Ready only goes true when we have successfully read the status register
                             // and the motor is disabled. This makes sure it can't automatically turn on when you plug it in.


extern UART_HandleTypeDef huart6;
extern DMA_HandleTypeDef hdma_usart6_rx;
extern DMA_HandleTypeDef hdma_usart6_tx;

static tUartDevice m_uartDevSpindle;
static tModBusDevice m_modbusSpindle;

static osThreadId_t m_thread_id;

static int32_t m_commsErrors = 0;

static const osThreadAttr_t m_attributes =
{ .name = "task_comms", .priority = (osPriority_t) osPriorityNormal, .stack_size = 4096, };

static void task_comms(void *argument);

void task_commsInit()
{
	m_thread_id = osThreadNew(task_comms, NULL, &m_attributes);
	//task_comms(0);
}

// Monitor registers
#define REG_MONITOR_START 0x170

#define REG_TORQUE   (REG_MONITOR_START + 2)
#define REG_SPEED    (REG_MONITOR_START + 8)
#define REG_IN_PORT  (REG_MONITOR_START + 14)


// Writable registers
#define REG_WRITE_SPEED 169


static int16_t readSingleRegister(uint16_t reg, bool *error)
{
	uint16_t ret = 0;

	if(error)
		*error = false;

	// Reset in case there is old data in there
	modbus_reset(&m_modbusSpindle); // Reset incase there is

	// Send read register
	modbus_sendMultiReadReg(&m_modbusSpindle, 1, reg, 1);

	// Wait for it to be received
	osDelay(5);

	if(modbus_checkReceive(&m_modbusSpindle))
	{
		ret = (m_modbusSpindle.receiveData[m_modbusSpindle.dataStart] << 8)
				| m_modbusSpindle.receiveData[m_modbusSpindle.dataStart + 1];
	}
	else
	{
		PRINTF("ERROR reading reg %d\r\n", reg);
		m_commsErrors++;
		if(error)
			*error = true;

		osDelay(5);
	}

	return ret;
}

static void task_comms(void *argument)
{

	PRINTF("task_comms STARTED\r\n");

	//uartInit(&m_uartDevSpindle, &huart6, &hdma_usart6_rx, &hdma_usart6_tx);
	uartInit(&m_uartDevSpindle, &huart6, &hdma_usart6_rx, NULL);
	uartStartRx(&m_uartDevSpindle);

	modbus_init(&m_modbusSpindle, &m_uartDevSpindle, 1);

	//speed = modbus_sendReadReg(&m_modbusSpindle, 1, REG_SPEED);

	//modbus_sendWriteReg(&m_modbusSpindle, 1, 169, 100);

	int oldRPM = 0;

	osDelay(1000);

	while (true)
	{

		PRINTF("tick comms\r\n");

		uint32_t start = osKernelGetTickCount();

		bool error = false;

		int16_t speed = readSingleRegister(REG_SPEED, &error);
		if(!error)
			motor_setSpeed(speed);

		int16_t torque = readSingleRegister(REG_TORQUE, &error);
		if(!error)
			motor_setTorque(torque);


		int16_t statusBits = readSingleRegister(REG_IN_PORT, &error);
		if(!error)
		{
			bool motorEnabled = !(statusBits & 0x1);

			if(!motorEnabled)
				m_ready = true;

			motor_setEnabled(motorEnabled); // The bottom input port bit is the enable signal (inverted)
		}


		// Update the motor speed if necessary
		int rpm = ui_getSetRPM();
		if(m_ready && (oldRPM != rpm))
		{
			modbus_sendWriteReg(&m_modbusSpindle, 1, REG_WRITE_SPEED, rpm);
			osDelay(5);
			if(!modbus_checkReceive(&m_modbusSpindle))
				m_commsErrors++;

			oldRPM = rpm;
		}

		motor_setCommsErrors(m_commsErrors);
	}
}
