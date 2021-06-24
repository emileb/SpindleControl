/*
 * user_input.c
 *
 *  Created on: Mar 27, 2021
 *      Authorrpm: emile
 */

#include "user_input.h"
#include "debug.h"


extern ADC_HandleTypeDef hadc3;


#define ADC_MAX_COUNTS 4096
#define RPM_MAX 3000
#define RPM_STEP_SIZE 10

static float adcFilt = 0;


uint16_t ui_getSetRPM()
{
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3, 10);

	uint16_t adc = HAL_ADC_GetValue(&hadc3);

	adcFilt = (0.9 * adcFilt) + (0.1 * adc);

	int32_t rpm = adcFilt;

	rpm = (rpm * (RPM_MAX / RPM_STEP_SIZE)) / ADC_MAX_COUNTS;

	rpm = rpm * RPM_STEP_SIZE;

	return rpm;
}
