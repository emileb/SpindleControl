/*
 * motor_status.c
 *
 *  Created on: 22 Jun 2021
 *      Author: emile
 */

#include "motor_status.h"

static int16_t m_speedRPM = 0;
static int16_t m_torque = 0;
static bool m_enabled = false;
static int32_t m_commsErrors = 0;

void motor_setSpeed(int16_t speed)
{
	m_speedRPM = speed;
}

void motor_setTorque(int16_t torque)
{
	m_torque = torque;
}

void motor_setEnabled(bool enabled)
{
	m_enabled = enabled;
}

void motor_setCommsErrors(int32_t errors)
{
	m_commsErrors = errors;
}

int16_t motor_getSpeed()
{
	return m_speedRPM;
}

int16_t motor_getTorque()
{
	return m_torque;
}

bool motor_getEnabled()
{
	return m_enabled;
}

int32_t motor_getCommsErrors()
{
	return m_commsErrors;
}
