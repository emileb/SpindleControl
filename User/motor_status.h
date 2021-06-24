/*
 * motor_status.h
 *
 *  Created on: 22 Jun 2021
 *      Author: emile
 */

#ifndef MOTOR_STATUS_H_
#define MOTOR_STATUS_H_

#include <stdint.h>
#include <stdbool.h>

void motor_setSpeed(int16_t);
void motor_setTorque(int16_t);
void motor_setEnabled(bool);
void motor_setCommsErrors(int32_t errors);

int16_t motor_getSpeed();
int16_t motor_getTorque();
bool motor_getEnabled();
int32_t motor_getCommsErrors();

#endif /* MOTOR_STATUS_H_ */
