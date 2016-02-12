/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2009  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.00 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : MainTask.C
Purpose     : Main program, called from after main after initialisation
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DualGaugeScreen.h"
#include "gauge.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "LEDHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmDualGaugeBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmLeftGauge;
extern GUI_CONST_STORAGE GUI_BITMAP bmRightGauge;
extern char lockupSemaphore;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
Gauge *CreateAnalogInputGauge(int input, Alarm **pAlarm, WM_HWIN hParent, int hpos, GUI_CONST_STORAGE GUI_BITMAP *pImage);

/*********************************************************************
*
*       Public variables
*
**********************************************************************
*/
char inputEnabled[4];

#ifdef __cplusplus
}
#endif


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
int GetInput(int n);
GlobalDataIndexType GetInputID(int input);

/*********************************************************************
*
*       Local Valiables
*
**********************************************************************
*/
static WM_HWIN hDualGaugeScreen;
static WM_HTIMER hScreenTimer;
Gauge *pAnalog0Gauge = 0;
Gauge *pAnalog1Gauge = 0;
static bool firsttimethrough;
static Alarm *pAnalog0Red = 0;
static Alarm *pAnalog1Red = 0;
static GUI_MEMDEV_Handle memDualGaugeScreen;

void _cbDualGaugeCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDualGaugeScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(MULTI_ANALOG_GAUGES) || firsttimethrough)
		{
			GUI_MEMDEV_Handle mControl;
			int temp;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			GUI_MEMDEV_Select(memDualGaugeScreen);

			firsttimethrough = false;

			GUI_DrawBitmap(&bmDualGaugeBackground, 0, 0);

			if ((temp = GetInput(1)) != -1)
				pAnalog0Gauge->SetValue(GetValue(GetInputID(temp)).fVal);
			if ((temp = GetInput(2)) != -1)
				pAnalog1Gauge->SetValue(GetValue(GetInputID(temp)).fVal);

			mControl = pAnalog0Gauge->DrawToMemArea();
			GUI_MEMDEV_Select(memDualGaugeScreen);
			GUI_MEMDEV_Write(mControl);

			mControl = pAnalog1Gauge->DrawToMemArea();
			GUI_MEMDEV_Select(memDualGaugeScreen);
			GUI_MEMDEV_Write(mControl);

			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
			{
				CheckRaceHelpScreen(MULTI_ANALOG_GAUGES);
			}
#endif

			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(memDualGaugeScreen);

			GUI_SelectLCD();
		}
		if (SlideOutVisible())
			DrawSignalStrength();
		break;

	case WM_DELETE:
		if (pAnalog0Gauge)
		{
			delete pAnalog0Gauge;
			pAnalog0Gauge = 0;
		}
		if (pAnalog1Gauge)
		{
			delete pAnalog1Gauge;
			pAnalog1Gauge = 0;
		}
		if (pAnalog0Red)
		{
			if (pAnalog0Red->Next)
				delete pAnalog0Red->Next;
			delete pAnalog0Red;
			pAnalog0Red = 0;
		}
		if (pAnalog1Red)
		{
			if (pAnalog1Red->Next)
				delete pAnalog1Red->Next;
			delete pAnalog1Red;
			pAnalog1Red = 0;
		}
		GUI_MEMDEV_Delete(memDualGaugeScreen);
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       Dual Gauge Screen
*/

WM_HWIN Display_DualGaugeScreen(WM_HWIN hParent)
{
	firsttimethrough = true;

	if ((hDualGaugeScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbDualGaugeCallback, 0)) <= 0)
		SystemError();

	// Display the FIRST TWO enabled analog inputs.  Note that there SHOULD only be
	// one or two inputs enabled.
	pAnalog0Gauge = CreateAnalogInputGauge(GetInput(1), &pAnalog0Red, hDualGaugeScreen,   0, &bmLeftGauge);
	pAnalog1Gauge = CreateAnalogInputGauge(GetInput(2), &pAnalog1Red, hDualGaugeScreen, 250, &bmRightGauge);

	WM_InvalidateWindow(hDualGaugeScreen);

	// create the memory area to prevent flicker
	memDualGaugeScreen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	hScreenTimer = WM_CreateTimer(hDualGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDualGaugeScreen;
}


Gauge *CreateAnalogInputGauge(int input, Alarm **pAlarm, WM_HWIN hParent, int hpos, GUI_CONST_STORAGE GUI_BITMAP *pImage)
{
	float upperLimit, lowerLimit, temp;
	char *pName, *pUnits;
	Gauge *pGauge;
	Alarm *pAlarmL, *pAlarmH;
	static char inputNum[5];

	if ((input != -1 ) && (inputEnabled[input] = vehicleData.analogInput[input].enabled))
	{
		upperLimit = vehicleData.analogInput[input].highValuePoint;
		lowerLimit = vehicleData.analogInput[input].lowValuePoint;
		
		// handle inverse user values
		if (upperLimit < lowerLimit) {
			temp = upperLimit;
			upperLimit = lowerLimit;
			lowerLimit = temp;
		}
		
		if ((upperLimit == 0.0) && (lowerLimit == 0.0))
		{
			upperLimit = 100.0;
			lowerLimit = 0.0;
		}
		temp = (upperLimit - lowerLimit) * 0.1;

		pAlarmL = pAlarmH = 0;
		if (vehicleData.analogInput[input].lowerAlarmTriggerComparison != OFF)
			pAlarmL = new Alarm(true, (float)vehicleData.analogInput[input].lowerAlarmTriggerLevel, lowerLimit, GUI_RED, GUI_DARKRED, LOW, NULL, NULL);
		if (vehicleData.analogInput[input].upperAlarmTriggerComparison != OFF)
			pAlarmH = new Alarm(true, (float)vehicleData.analogInput[input].upperAlarmTriggerLevel, upperLimit, GUI_RED, GUI_DARKRED, HIGH, NULL, NULL);

		if ((pAlarmL == 0) && (pAlarmH == 0))
			*pAlarm = 0;
		else if (pAlarmL)
		{
			*pAlarm = pAlarmL;
			if (pAlarmH)
				pAlarmL->Next = pAlarmH;
		}
		else *pAlarm = pAlarmH;

		pName = vehicleData.analogInput[input].Name;
		pUnits = vehicleData.analogInput[input].Units;
		sprintf(inputNum, "A%d", input);
	}
	else
	{
		upperLimit = 100.0;
		lowerLimit = 0.0;
		temp = 10.0;
		*pAlarm = 0;
		pName = "Disabled";
		pUnits = "";
		inputNum[0] = 0;
	}

	if ((pGauge = new Gauge(hpos, 42, 115, lowerLimit, upperLimit,
			pName, pUnits, inputNum, *pAlarm, (temp * 2), temp, 0, 0, true, hParent, pImage)) <= 0)
		SystemError();

	if (!inputEnabled[input])
		pGauge->SetValue(0.0);

	return pGauge;
}

//
// Returns index to the nth enabled input.
// If n inputs not enabled return -1
//
int GetInput(int n)
{
	int count, index;

	for (count = index = 0; index < 4; index++)
	{
		if (vehicleData.analogInput[index].enabled)
			count++;
		if (count == n)
			return index;
	}
	return -1;
}

GlobalDataIndexType GetInputID(int input)
{
	switch (input)
	{
	default:
	case 0:	return ANALOG_INPUT_0;
	case 1:	return ANALOG_INPUT_1;
	case 2:	return ANALOG_INPUT_2;
	case 3:	return ANALOG_INPUT_3;
	}
}

/*************************** End of file ****************************/

