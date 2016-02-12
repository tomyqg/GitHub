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
File        : LinearGaugeScreen.cpp
Purpose     : Controlling screen for multiple linear gauges display
---------------------------END-OF-HEADER------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "LinearGaugeScreen.h"
#include "LinearGauge.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "LEDHandler.h"
#include "Alarm.hpp"
#include "DualGaugeScreen.h"

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmAnalogs;
extern GUI_CONST_STORAGE GUI_BITMAP bmDarkAsphalt;
extern char lockupSemaphore;
extern char inputEnabled[];		// Defined in DualGaugeScreen.cpp

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
LinearGauge *CreateLinearInputGauge(char input, WM_HWIN hParent, int hpos, int vpos);

/*********************************************************************
*
*       Public variables
*
**********************************************************************
*/

#ifdef __cplusplus
}
#endif


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Valiables
*
**********************************************************************
*/
static WM_HWIN hLinearGaugesScreen;
static WM_HTIMER hScreenTimer;
static LinearGauge *pAnalog0Gauge = 0;
static LinearGauge *pAnalog1Gauge = 0;
static LinearGauge *pAnalog2Gauge = 0;
static LinearGauge *pAnalog3Gauge = 0;
static bool firsttimethrough;
static GUI_MEMDEV_Handle memLinearGaugesScreen;


void _cbLinearGaugeCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hLinearGaugesScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(MULTI_ANALOG_GAUGES) || firsttimethrough)
		{
			float value;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			GUI_MEMDEV_Select(memLinearGaugesScreen);

			GUI_Clear();
			GUI_SetTextMode(GUI_TEXTMODE_TRANS);

			GUI_DrawBitmap(&bmDarkAsphalt, 0, 0);
			GUI_DrawBitmap(&bmAnalogs, 0, 0);

			// Draw all 4 backgrounds
			pAnalog0Gauge->DrawBackground();
			pAnalog1Gauge->DrawBackground();
			pAnalog2Gauge->DrawBackground();
			pAnalog3Gauge->DrawBackground();

			// Draw pointers
			if (inputEnabled[0])
				value = GetValue(ANALOG_INPUT_0).fVal;
			else value = 0;
			pAnalog0Gauge->SetValue(value);

			if (inputEnabled[1])
				value = GetValue(ANALOG_INPUT_1).fVal;
			else value = 0;
			pAnalog1Gauge->SetValue(value);

			if (inputEnabled[2])
				value = GetValue(ANALOG_INPUT_2).fVal;
			else value = 0;
			pAnalog2Gauge->SetValue(value);

			if (inputEnabled[3])
				value = GetValue(ANALOG_INPUT_3).fVal;
			else value = 0;
			pAnalog3Gauge->SetValue(value);

			firsttimethrough = false;

			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
			{
				CheckRaceHelpScreen(MULTI_ANALOG_GAUGES);
			}
#endif

			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(memLinearGaugesScreen);
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
		if (pAnalog2Gauge)
		{
			delete pAnalog2Gauge;
			pAnalog2Gauge = 0;
		}
		if (pAnalog3Gauge)
		{
			delete pAnalog3Gauge;
			pAnalog3Gauge = 0;
		}
		GUI_MEMDEV_Delete(memLinearGaugesScreen);
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       Linear Gauge Screen
*/

WM_HWIN DisplayMultiAnalogGaugesScreen(WM_HWIN hParent)
{
	if (DetermineAnalogScreenSelection() <= 2)
	{
		return Display_DualGaugeScreen(hParent);
	}

	// Displaying LinearGauges - continue here with Linear Gauges creating
	firsttimethrough = true;
	if ((hLinearGaugesScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbLinearGaugeCallback, 0)) <= 0)
		SystemError();

	pAnalog0Gauge = CreateLinearInputGauge(0, hLinearGaugesScreen, 5, 35);
	pAnalog1Gauge = CreateLinearInputGauge(1, hLinearGaugesScreen, 245, 35);
	pAnalog2Gauge = CreateLinearInputGauge(2, hLinearGaugesScreen, 5, 150);
	pAnalog3Gauge = CreateLinearInputGauge(3, hLinearGaugesScreen, 245, 150);

	WM_InvalidateWindow(hLinearGaugesScreen);

	// create the memory area to prevent flicker
	memLinearGaugesScreen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	hScreenTimer = WM_CreateTimer(hLinearGaugesScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hLinearGaugesScreen;
}


LinearGauge *CreateLinearInputGauge(char input, WM_HWIN hParent, int hpos, int vpos)
{
	float upperLimit, lowerLimit, temp;
	char *pName, *pUnits;
	LinearGauge *pGauge;

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

	if ((inputEnabled[input] = vehicleData.analogInput[input].enabled))
	{
		pName = vehicleData.analogInput[input].Name;
		pUnits = vehicleData.analogInput[input].Units;
	}
	else
	{
		pName = "Disabled";
		pUnits = "";
	}

	if ((pGauge = new LinearGauge(hpos, vpos, 230, 110, lowerLimit, upperLimit,
		//vehicleData.analogInput[input].lowerAlarmTriggerComparison == BELOW ? 1 : 0, vehicleData.analogInput[input].lowerAlarmTriggerLevel,
		//vehicleData.analogInput[input].lowerAlarmTriggerComparison == ABOVE ? 1 : 0, vehicleData.analogInput[input].lowerAlarmTriggerLevel,
		vehicleData.analogInput[input].lowerAlarmTriggerComparison == OFF ? 0 : 1, vehicleData.analogInput[input].lowerAlarmTriggerLevel,
		vehicleData.analogInput[input].upperAlarmTriggerComparison == OFF ? 0 : 1, vehicleData.analogInput[input].upperAlarmTriggerLevel,
		pName, pUnits, 0, hParent, (0 != vehicleData.analogInput[input].enabled))) <= 0)
		SystemError();

	if (!inputEnabled[input])
		pGauge->SetValue(0.0);

	return pGauge;
}

/*************************** End of file ****************************/

