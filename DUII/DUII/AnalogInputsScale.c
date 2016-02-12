//
//	AnalogInputScale.c
//
//	Gathers user input for analog input scale settings
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DataHandler.h"
#include "AnalogInputSettings.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"
#include "AnalogInputsScale.h"
#include "Utilities.h"
#include "GeneralConfig.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DisplayVoltsLow(WM_HWIN);
void DisplayVoltsHigh(WM_HWIN);
void DisplayValueLow(WM_HWIN hParent);
void ValueLowPaint(WM_HWIN hParent);
void DisplayValueHigh(WM_HWIN hParent);
void ValueHighPaint(WM_HWIN hParent);
void VoltsLowPaint(WM_HWIN);
void VoltsHighPaint(WM_HWIN);
void SaveData(const void *type);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST AnalogInputsScaleScreenList[NUM_ANALOG_SCALE_SCREENS+1] = {
	{ VOLTS_LOW_SCREEN, DisplayVoltsLow, VoltsLowPaint },
	{ VOLTS_HIGH_SCREEN, DisplayVoltsHigh, VoltsHighPaint },
	{ VALUE_LOW_SCREEN, DisplayValueLow, ValueLowPaint },
	{ VALUE_HIGH_SCREEN, DisplayValueHigh, ValueHighPaint },
	{ -1, NULL, NULL }
};

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hAnalogInputsScale;
static void *pVoltsLow = 0;
static void *pVoltsHigh = 0;
static void *pValueLow = 0;
static void *pValueHigh = 0;

char voltsLowString[NUMBER_LENGTH + NUMBER_LENGTH];		// Make these super big to handle sprintf output for numbers like 12345.67891
char voltsHighString[NUMBER_LENGTH + NUMBER_LENGTH];
char valueLowString[NUMBER_LENGTH + NUMBER_LENGTH];
char valueHighString[NUMBER_LENGTH + NUMBER_LENGTH];

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetAnalogInputsScaleWindowHandle(void)
{
	return hAnalogInputsScale;
}

void SetAnalogInputsScaleWindowHandle(WM_HWIN hWin)
{
	hAnalogInputsScale = hWin;
}

void SaveData(const void *type)
{
	if (type == pVoltsLow)
		vehicleData.analogInput[globalInput].lowVoltagePoint = atof(voltsLowString);
	else if (type == pVoltsHigh)
		vehicleData.analogInput[globalInput].highVoltagePoint = atof(voltsHighString);
	else if (type == pValueLow)
		vehicleData.analogInput[globalInput].lowValuePoint = atof(valueLowString);
	else if (type == pValueHigh)
		vehicleData.analogInput[globalInput].highValuePoint = atof(valueHighString);
}

void AnalogInputsScaleCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, GetLastScreen(ANALOG_SETTINGS_SCREEN_TYPE), hAnalogInputsScale);
			break;

		case KB_SAVE_MSG:
			SaveData(pMsg->Data.p);
			break;

		case KB_SAVE_AND_EXIT_MSG:
			SaveData(pMsg->Data.p);
			DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, GetLastScreen(ANALOG_SETTINGS_SCREEN_TYPE), hAnalogInputsScale);
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
// Do not display icons on these screens - there is not enough room
//				WM_InvalidateRect(pMsg->hWin, &SignalStrengthRect);
			}
			break;

		case WM_CREATE:
			AnalogInputsScaleIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == ANALOG_INPUTS_SCALE)
			{
				// Call paint method of center screen
				AnalogInputsScaleScreenList[GetLastScreen(ANALOG_INPUTS_SCALE)].paint(hAnalogInputsScale);

// Do not display icons on these screens - there is not enough room
//				DrawSignalStrength();
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplayVoltsLow(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pVoltsLow)
	////	DestroyKeyboard(pVoltsLow);

	sprintf(scratch, "Volts Low - Analog %d", globalInput);
	sprintf(voltsLowString, "%f", vehicleData.analogInput[globalInput].lowVoltagePoint);
	trimn(voltsLowString, NUMBER_LENGTH);			// Trim off trailing 0s
	pVoltsLow = CreateKeyboard(scratch, voltsLowString, NUMERIC, true, NUMBER_LENGTH, VOLTS_LOW_SCREEN, hParent, true);
}

void DisplayVoltsHigh(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pVoltsHigh)
	////	DestroyKeyboard(pVoltsHigh);

	sprintf(scratch, "Volts High - Analog %d", globalInput);
	sprintf(voltsHighString, "%f", vehicleData.analogInput[globalInput].highVoltagePoint);
	trimn(voltsHighString, NUMBER_LENGTH);
	pVoltsHigh = CreateKeyboard(scratch, voltsHighString, NUMERIC, true, NUMBER_LENGTH, VOLTS_HIGH_SCREEN, hParent, true);
}

void DisplayValueLow(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pValueLow)
	////	DestroyKeyboard(pValueLow);

	sprintf(scratch, "Value Low - Analog %d", globalInput);
	sprintf(valueLowString, "%f", vehicleData.analogInput[globalInput].lowValuePoint);
	trimn(valueLowString, NUMBER_LENGTH);
	pValueLow = CreateKeyboard(scratch, valueLowString, NUMERIC, true, NUMBER_LENGTH, VALUE_LOW_SCREEN, hParent, true);
}

void DisplayValueHigh(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pValueHigh)
	////	DestroyKeyboard(pValueHigh);

	sprintf(scratch, "Value High - Analog %d", globalInput);
	sprintf(valueHighString, "%f", vehicleData.analogInput[globalInput].highValuePoint);
	trimn(valueHighString, NUMBER_LENGTH);
	pValueHigh = CreateKeyboard(scratch, valueHighString, NUMERIC, true, NUMBER_LENGTH, VALUE_HIGH_SCREEN, hParent, true);
}

void VoltsLowPaint(WM_HWIN hParent)
{
}


void VoltsHighPaint(WM_HWIN hParent)
{
}

void ValueLowPaint(WM_HWIN hParent)
{
}

void ValueHighPaint(WM_HWIN hParent)
{
}

void AnalogInputsScaleIgnoreNextKeyRelease(void)
{
}

void DeleteAnalogScaleKeyboards(void)
{
	if (pVoltsHigh)
	{
		DestroyKeyboard(pVoltsHigh);
		pVoltsHigh = 0;
	}
	if (pVoltsLow)
	{
		DestroyKeyboard(pVoltsLow);
		pVoltsLow = 0;
	}
	if (pValueHigh)
	{
		DestroyKeyboard(pValueHigh);
		pValueHigh = 0;
	}
	if (pValueLow)
	{
		DestroyKeyboard(pValueLow);
		pValueLow = 0;
	}
}

/*************************** End of file ****************************/

