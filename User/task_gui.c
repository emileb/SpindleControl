/*
 * task_gui.c
 *
 *  Created on: Jan 19, 2021
 *      Author: emile
 */

#include "cmsis_os.h"
#include <stdbool.h>
#include <stdint.h>
#include "debug.h"
#include "user_input.h"
#include "motor_status.h"

#include "STemWin/STemWin.h"
#include "DIALOG.h"
#include <stdlib.h>
#include <string.h>
#include "GRAPH.h"
#include "TEXT.h"

#define SPINDLE_GEAR_RATIO (60.0 / 22.0) // We have a 60 tooth in the motor and 22 tooth on the spindle

#define GRAPH_WIDTH  480
#define GRAPH_HEIGHT 200

#define TEXT_TOP (GRAPH_HEIGHT + 15)

#define SPEED_MAX_GRAPH 9000  // Maximm value to be displayed on the graph



static osThreadId_t m_thread_id;

static const osThreadAttr_t m_attributes =
{ .name = "task_gui", .priority = (osPriority_t) osPriorityNormal, .stack_size = 1024 * 16, };

static GRAPH_Handle m_hGraph;
static GRAPH_DATA_Handle m_SpeedGraphData;
static GRAPH_DATA_Handle m_TorqueGraphData;

static TEXT_Handle m_statusText;
static TEXT_Handle m_speedText;
static TEXT_Handle m_torqueText;

static TEXT_Handle m_commsErrText;

void task_gui(void *argument);

static void createGui()
{
	WM_SetDesktopColor(GUI_BLACK);
	WM_SetCreateFlags(WM_CF_MEMDEV);

	m_hGraph = GRAPH_CreateEx(10, 10, GRAPH_WIDTH, GRAPH_HEIGHT, WM_HBKWIN, WM_CF_SHOW, 0, GUI_ID_GRAPH0);

	GRAPH_SetGridDistY(m_hGraph, 25);
	GRAPH_SetGridVis(m_hGraph, 1);
	GRAPH_SetGridFixedX(m_hGraph, 1);

	GRAPH_SCALE_Handle scaleV = GRAPH_SCALE_Create(35, GUI_TA_RIGHT, GRAPH_SCALE_CF_VERTICAL, 20);
	GRAPH_SCALE_SetTextColor(scaleV, GUI_YELLOW);
	//GRAPH_SCALE_SetOff(scaleV, 75);
	GRAPH_SCALE_SetFactor(scaleV, (float) SPEED_MAX_GRAPH / (float) GRAPH_HEIGHT);
	GRAPH_AttachScale(m_hGraph, scaleV);

	GRAPH_SCALE_Handle scaleH = GRAPH_SCALE_Create(GRAPH_HEIGHT / 2, GUI_TA_HCENTER, GRAPH_SCALE_CF_HORIZONTAL, 50);
	GRAPH_SCALE_SetTextColor(scaleH, GUI_DARKGREEN);
	GRAPH_SCALE_SetOff(scaleH, 0);
	GRAPH_AttachScale(m_hGraph, scaleH);

	m_SpeedGraphData = GRAPH_DATA_YT_Create(GUI_LIGHTBLUE, GRAPH_WIDTH, 0, 0);
	m_TorqueGraphData = GRAPH_DATA_YT_Create(GUI_LIGHTRED, GRAPH_WIDTH, 0, 0);

	GRAPH_AttachData(m_hGraph, m_SpeedGraphData);
	GRAPH_AttachData(m_hGraph, m_TorqueGraphData);

	m_statusText = TEXT_Create(0, TEXT_TOP + 15, 80, 50, 0, WM_CF_SHOW, "Hello", GUI_TA_HCENTER | GUI_TA_VCENTER);
	TEXT_SetFont(m_statusText, GUI_FONT_32B_ASCII);
	TEXT_SetBkColor(m_statusText, GUI_GRAY_50);

	m_speedText = TEXT_Create(320, TEXT_TOP, 250, 50, 0, WM_CF_SHOW, "Hello", GUI_TA_LEFT | GUI_TA_VCENTER);
	TEXT_SetFont(m_speedText, GUI_FONT_D48);
	TEXT_SetTextColor(m_speedText, GUI_LIGHTBLUE);

	m_torqueText = TEXT_Create(150, TEXT_TOP, 150, 50, 0, WM_CF_SHOW, "Hello", GUI_TA_LEFT | GUI_TA_VCENTER);
	TEXT_SetFont(m_torqueText, GUI_FONT_D48);
	TEXT_SetTextColor(m_torqueText, GUI_LIGHTRED);

	m_commsErrText = TEXT_Create(350, 10, 150, 20, 0, WM_CF_SHOW, "Hello", GUI_TA_LEFT);
	TEXT_SetFont(m_commsErrText, GUI_FONT_16_ASCII);
	TEXT_SetTextColor(m_commsErrText, GUI_WHITE);
}

static int16_t getSpindleSpeed()
{
	float motorSpeed = motor_getSpeed();

	return (int16_t) (motorSpeed * SPINDLE_GEAR_RATIO);
}

void task_guiInit()
{
	m_thread_id = osThreadNew(task_gui, NULL, &m_attributes);
}

void task_gui(void *argument)
{
	PRINTF("task_gui STARTED\r\n");

	GRAPHICS_Init();

	//GUI_CURSOR_Show();

	createGui();

	while (true)
	{
		//TouchDriver_Poll();

		GUI_Exec();

		// The graph is done in pixels, need to scale the data to fit
		float spindleSpeed = getSpindleSpeed();
		spindleSpeed = spindleSpeed * GRAPH_HEIGHT;
		spindleSpeed = spindleSpeed / SPEED_MAX_GRAPH;

		GRAPH_DATA_YT_AddValue(m_SpeedGraphData, spindleSpeed);

		// Torque is in %
		float torque = motor_getTorque();
		torque = torque * 3; // Scale up a bit
		GRAPH_DATA_YT_AddValue(m_TorqueGraphData, torque);

		// Show ON/OFF status
		bool enabled = motor_getEnabled();
		if (enabled)
		{
			static int32_t flash = 0;
			flash++;
			TEXT_SetTextColor(m_statusText, GUI_LIGHTRED);
			if (flash & 0x10)
				TEXT_SetText(m_statusText, "ON");
			else
				TEXT_SetText(m_statusText, "");
		}
		else
		{
			TEXT_SetTextColor(m_statusText, GUI_LIGHTGREEN);
			TEXT_SetText(m_statusText, "OFF");
		}

		// Speed text
		char buff[32];
		//TEXT_SetDec(m_speedText, (int)getSpindleSpeed(), 5,0,1, 1); // This make it flash badly
		int32_t speedDisplay = (int) getSpindleSpeed();
		speedDisplay = (speedDisplay / 50) * 50; // Only show resolution of 50's
		sprintf(buff, "%04d", speedDisplay);
		TEXT_SetText(m_speedText, buff);

		// Torque text
		int16_t torqueDisplay = motor_getTorque();
		static float torqueAverage = 0;
		torqueAverage = (0.9 * torqueAverage) + (0.1 * torqueDisplay);
		sprintf(buff, "%02d%%", (int) torqueAverage);
		TEXT_SetText(m_torqueText, buff);


		sprintf(buff, "Comms errors: %d", motor_getCommsErrors());
		TEXT_SetText(m_commsErrText, buff);
	}
}

