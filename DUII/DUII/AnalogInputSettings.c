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
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "DIALOG.h"
#include "ConfigScreens.h"
#include "GeneralConfig.h"
#include "FlashReadWrite.h"
#include "DataHandler.h"
#include "AnalogInputSettings.h"
#include "StandardList.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"
#include "AnalogInputsScale.h"
#include "Utilities.h"

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
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmMiniButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTinyBlackButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmDummyDigital;
extern GUI_CONST_STORAGE GUI_BITMAP bmDummyAnalog;
extern GUI_CONST_STORAGE GUI_BITMAP bmAnalog;
extern GUI_CONST_STORAGE GUI_BITMAP bmScale;
extern GUI_CONST_STORAGE GUI_BITMAP bmAlarmText;
extern GUI_CONST_STORAGE GUI_BITMAP bm0;
extern GUI_CONST_STORAGE GUI_BITMAP bm1;
extern GUI_CONST_STORAGE GUI_BITMAP bm2;
extern GUI_CONST_STORAGE GUI_BITMAP bm3;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DisplayScale(WM_HWIN);
void DisplayAlarm(WM_HWIN);
void DisplayRenameInput(WM_HWIN hParent);
void RenameInputPaint(WM_HWIN hParent);
void DisplayRenameUnits(WM_HWIN hParent);
void RenameUnitsPaint(WM_HWIN hParent);
void ScalePaint(WM_HWIN);
void AlarmPaint(WM_HWIN);
void SetFilteringText(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST AnalogInputSettingsScreenList[NUM_ANALOG_INPUT_SCREENS+1] = {
	{ SCALE, DisplayScale, ScalePaint },
	{ ALARM, DisplayAlarm, AlarmPaint },
	{ RENAME_INPUT, DisplayRenameInput, RenameInputPaint },
	{ RENAME_UNITS, DisplayRenameUnits, RenameUnitsPaint },
	{ -1, NULL, NULL }
};

#define	LED_1_BUTTON			GUI_ID_USER + 1
#define	LED_2_BUTTON			GUI_ID_USER + 2
#define	ALARM_CONDITION_BUTTON	GUI_ID_USER + 3
#define	ALARM_VALUE_BUTTON		GUI_ID_USER + 4
#define	RENAME_UNITS_BUTTON		GUI_ID_USER + 5
#define	RENAME_HIGH_BUTTON		GUI_ID_USER + 6
#define	RENAME_LOW_BUTTON		GUI_ID_USER + 7
#define	VOLTS_LOW_BUTTON		GUI_ID_USER + 8
#define	VOLTS_HIGH_BUTTON		GUI_ID_USER + 9
#define	VALUE_LOW_BUTTON		GUI_ID_USER + 10
#define	VALUE_HIGH_BUTTON		GUI_ID_USER + 11
#define	VOLTS_RANGE_BUTTON		GUI_ID_USER + 12
#define	UPPER_ALARM_CONDITION_BUTTON	GUI_ID_USER + 13
#define	UPPER_ALARM_VALUE_BUTTON		GUI_ID_USER + 14
#define	FILTERING_LEVEL_BUTTON	GUI_ID_USER + 15

#define	LEFT_BUTTON_LEFT		25
#define	RIGHT_BUTTON_LEFT		284
#define	ANA_BUTTON_WIDTH		180
#define	TINY_BUTTON_WIDTH		90
#define	ANA_BUTTON_HEIGHT		53
#define	TOP_BUTTON_ROW			76
#define	SECOND_BUTTON_ROW		138
#define	THIRD_BUTTON_ROW		200
#define	ALARM_CONFIG_BOX_H_CENTER	356
#define	ALARM_CONFIG_BOX_H_WIDTH	104
#define	ALARM_CONFIG_ROW_1	55
#define	ALARM_CONFIG_ROW_2	90
#define	ALARM_CONFIG_ROW_3	120
#define	ALARM_CONFIG_ROW_4	180
#define	TOP_ALARM_INDICATOR_ROW			90
#define	SECOND_ALARM_INDICATOR_ROW		150
#define	ALARM_INDICATOR_BUTTON_LEFT		40

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hAnalogInputSettings;
BUTTON_Handle hLED1Button, hLED2Button, hAlarmConditionButton, hAlarmValueButton;
BUTTON_Handle hUpperAlarmConditionButton, hUpperAlarmValueButton;
BUTTON_Handle hVoltsLowButton, hVoltsHighButton, hValueLowButton, hValueHighButton, hVoltsRangeButton, hFilteringLevelButton;
static void *pRenameInput = 0;
static void *pRenameUnits = 0;
static void *pAlarmValue = 0;
char alarmValueString[10+1];
enum UPPER_LOWER gathering;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetAnalogInputSettingsWindowHandle(void)
{
	return hAnalogInputSettings;
}

void SetAnalogInputSettingsWindowHandle(WM_HWIN hWin)
{
	hAnalogInputSettings = hWin;
}

void AISKillKeyboard(void)
{
	InputCleanUpKeyboards();
	//if (pAlarmValue)		// Should alwaye be there - but just in case!
	//{
	//	DestroyKeyboard(pAlarmValue);
	//	pAlarmValue = 0;
	//}
	ResumeScreenScrolling();
	SetScreenType(ANALOG_SETTINGS_SCREEN_TYPE);
	WM_InvalidateWindow(hAnalogInputSettings);
}


void AnalogInputSettingsCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			if (pMsg->Data.p == pAlarmValue)
			{
				AISKillKeyboard();
			}
			else DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), hAnalogInputSettings);
			break;

		case KB_SAVE_AND_EXIT_MSG:
			if (pMsg->Data.p == pAlarmValue)
			{
				if (gathering == UPPER)
					vehicleData.analogInput[globalInput].upperAlarmTriggerLevel = atof(alarmValueString);
				else vehicleData.analogInput[globalInput].lowerAlarmTriggerLevel = atof(alarmValueString);
				AISKillKeyboard();
			}
			else DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), hAnalogInputSettings);
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
// Do not display icons on these screens - there is not enough room
//				WM_InvalidateRect(pMsg->hWin, &SignalStrengthRect);
				WM_InvalidateRect(pMsg->hWin, &InfoTimeRect);
			}
			break;

		case WM_CREATE:
			AnalogInputSettingsIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == ANALOG_SETTINGS_SCREEN_TYPE)
			{
				// Call paint method of center screen
				AnalogInputSettingsScreenList[GetLastScreen(ANALOG_SETTINGS_SCREEN_TYPE)].paint(hAnalogInputSettings);

	// Do not display icons on these screens - there is not enough room
	//			DrawSignalStrength();
				DrawInfo();
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_RELEASED:
					switch (Id)
					{
						case LED_1_BUTTON:
							if (vehicleData.analogInput[globalInput].EnableLED1)
								vehicleData.analogInput[globalInput].EnableLED1 = false;
							else vehicleData.analogInput[globalInput].EnableLED1 = true;
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							break;

						case LED_2_BUTTON:
							if (vehicleData.analogInput[globalInput].EnableLED2)
								vehicleData.analogInput[globalInput].EnableLED2 = false;
							else vehicleData.analogInput[globalInput].EnableLED2 = true;
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							break;

						case ALARM_CONDITION_BUTTON:
							switch (vehicleData.analogInput[globalInput].lowerAlarmTriggerComparison)
							{
								case BELOW:
								//	vehicleData.analogInput[globalInput].lowerAlarmTriggerComparison = EQUAL_TO;
								//	break;
								//case EQUAL_TO:
									vehicleData.analogInput[globalInput].lowerAlarmTriggerComparison = OFF;
									break;
								default:			// Catch any other value for safety (OFF, EQUAL_TO, ABOVE)
									vehicleData.analogInput[globalInput].lowerAlarmTriggerComparison = BELOW;
									break;
							}
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							break;

						case UPPER_ALARM_CONDITION_BUTTON:
							switch (vehicleData.analogInput[globalInput].upperAlarmTriggerComparison)
							{
								case ABOVE:
								//	vehicleData.analogInput[globalInput].upperAlarmTriggerComparison = EQUAL_TO;
								//	break;
								//case EQUAL_TO:
									vehicleData.analogInput[globalInput].upperAlarmTriggerComparison = OFF;
									break;
								default:			// Catch any other value for safety (OFF, EQUAL_TO, BELOW)
									vehicleData.analogInput[globalInput].upperAlarmTriggerComparison = ABOVE;
									break;
							}
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							break;

						case ALARM_VALUE_BUTTON:
							{
								gathering = LOWER;
								InputCleanUpKeyboards();
								sprintf(alarmValueString, "%.2f", vehicleData.analogInput[globalInput].lowerAlarmTriggerLevel);
								pAlarmValue = CreateKeyboard("Enter Lower Alarm Value", alarmValueString, NUMERIC, true, NUMBER_LENGTH, 0, hAnalogInputSettings, false);
								PauseScreenScrolling();
								SetScreenType(ANALOG_NUMBER_ENTRY_STANDALONE);
							}
							break;

						case UPPER_ALARM_VALUE_BUTTON:
							{
								gathering = UPPER;
								InputCleanUpKeyboards();
								sprintf(alarmValueString, "%.2f", vehicleData.analogInput[globalInput].upperAlarmTriggerLevel);
								pAlarmValue = CreateKeyboard("Enter Upper Alarm Value", alarmValueString, NUMERIC, true, NUMBER_LENGTH, 0, hAnalogInputSettings, false);
								PauseScreenScrolling();
								SetScreenType(ANALOG_NUMBER_ENTRY_STANDALONE);
							}
							break;

						case VOLTS_LOW_BUTTON:
							DisplayNewScreenList(ANALOG_INPUTS_SCALE, VOLTS_LOW_SCREEN, hAnalogInputSettings);
							break;

						case VOLTS_HIGH_BUTTON:
							DisplayNewScreenList(ANALOG_INPUTS_SCALE, VOLTS_HIGH_SCREEN, hAnalogInputSettings);
							break;

						case VALUE_LOW_BUTTON:
							DisplayNewScreenList(ANALOG_INPUTS_SCALE, VALUE_LOW_SCREEN, hAnalogInputSettings);
							break;

						case VALUE_HIGH_BUTTON:
							DisplayNewScreenList(ANALOG_INPUTS_SCALE, VALUE_HIGH_SCREEN, hAnalogInputSettings);
							break;

						case VOLTS_RANGE_BUTTON:
							if (vehicleData.analogInput[globalInput].inputRange == 5)
								vehicleData.analogInput[globalInput].inputRange = 10;
							else if (vehicleData.analogInput[globalInput].inputRange == 10)
								vehicleData.analogInput[globalInput].inputRange = 20;
							else vehicleData.analogInput[globalInput].inputRange = 5;
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							notifyConfigChange();		// Tell SU about change in voltage range
							break;

						case FILTERING_LEVEL_BUTTON:
							switch (vehicleData.analogInput[globalInput].enabled)
							{
							case FILTER_NONE:
							default:		vehicleData.analogInput[globalInput].enabled = FILTER_LOW;				break;
							case FILTER_LOW:		vehicleData.analogInput[globalInput].enabled = FILTER_MEDIUM;	break;
							case FILTER_MEDIUM:	vehicleData.analogInput[globalInput].enabled = FILTER_HIGH;			break;
							case FILTER_HIGH:		vehicleData.analogInput[globalInput].enabled = FILTER_VERY_HIGH;break;
							case FILTER_VERY_HIGH:	vehicleData.analogInput[globalInput].enabled = FILTER_NONE;		break;
							}
							SetFilteringText();
							break;
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}


void DisplayScale(WM_HWIN hParent)
{
	BUTTON_SetDefaultBkColor(GUI_RED, BUTTON_BI_UNPRESSED);

	hVoltsLowButton = BUTTON_CreateEx(LEFT_BUTTON_LEFT, TOP_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VOLTS_LOW_BUTTON);
	BUTTON_SetBitmap(hVoltsLowButton, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetTextColor(hVoltsLowButton, BUTTON_BI_UNPRESSED, GUI_GREEN);
	BUTTON_SetTextColor(hVoltsLowButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hVoltsLowButton);

	hVoltsHighButton = BUTTON_CreateEx(LEFT_BUTTON_LEFT, SECOND_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VOLTS_HIGH_BUTTON);
	BUTTON_SetBitmap(hVoltsHighButton, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetTextColor(hVoltsHighButton, BUTTON_BI_UNPRESSED, GUI_RED);
	BUTTON_SetTextColor(hVoltsHighButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hVoltsHighButton);

	hValueLowButton = BUTTON_CreateEx(RIGHT_BUTTON_LEFT, TOP_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VALUE_LOW_BUTTON);
	BUTTON_SetBitmap(hValueLowButton, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetTextColor(hValueLowButton, BUTTON_BI_UNPRESSED, GUI_GREEN);
	BUTTON_SetTextColor(hValueLowButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hValueLowButton);

	hValueHighButton = BUTTON_CreateEx(RIGHT_BUTTON_LEFT, SECOND_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VALUE_HIGH_BUTTON);
	BUTTON_SetBitmap(hValueHighButton, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetTextColor(hValueHighButton, BUTTON_BI_UNPRESSED, GUI_RED);
	BUTTON_SetTextColor(hValueHighButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hValueHighButton);

	hVoltsRangeButton = BUTTON_CreateEx(RIGHT_BUTTON_LEFT, THIRD_BUTTON_ROW, TINY_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, VOLTS_RANGE_BUTTON);
	BUTTON_SetBitmap(hVoltsRangeButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hVoltsRangeButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hVoltsRangeButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hVoltsRangeButton);

	hFilteringLevelButton = BUTTON_CreateEx(LEFT_BUTTON_LEFT + ANA_BUTTON_WIDTH - TINY_BUTTON_WIDTH - 1, THIRD_BUTTON_ROW, TINY_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, FILTERING_LEVEL_BUTTON);
	BUTTON_SetBitmap(hFilteringLevelButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hFilteringLevelButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hFilteringLevelButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hFilteringLevelButton);
}

void ScalePaint(WM_HWIN hParent)
{
	char scratch[30];
	GUI_CONST_STORAGE GUI_BITMAP *pBanner;

	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);

	switch (globalInput)
	{
	default:
	case 0:	pBanner = &bm0;	break;
	case 1:	pBanner = &bm1;	break;
	case 2:	pBanner = &bm2;	break;
	case 3:	pBanner = &bm3;	break;
	}
	GUI_DrawBitmap(&bmAnalog, 0, 0);
	GUI_DrawBitmap(pBanner, 224, 0);
	GUI_DrawBitmap(&bmScale, 253, 0);

	GUI_DispStringHCenterAt("Volts", LEFT_BUTTON_LEFT + (ANA_BUTTON_WIDTH/2), TOP_BUTTON_ROW - 28);
	GUI_DispStringHCenterAt("Value", RIGHT_BUTTON_LEFT + (ANA_BUTTON_WIDTH/2), TOP_BUTTON_ROW - 28);

	GUI_DispStringHCenterAt("LOW", 245, TOP_BUTTON_ROW + 15);
	GUI_DispStringHCenterAt("HIGH", 245, SECOND_BUTTON_ROW + 15);

	sprintf(scratch, "%f", vehicleData.analogInput[globalInput].lowVoltagePoint);
	trimn(scratch, NUMBER_LENGTH);
	BUTTON_SetText(hVoltsLowButton, scratch);

	sprintf(scratch, "%f", vehicleData.analogInput[globalInput].highVoltagePoint);
	trimn(scratch, NUMBER_LENGTH);
	BUTTON_SetText(hVoltsHighButton, scratch);

	sprintf(scratch, "%f", vehicleData.analogInput[globalInput].lowValuePoint);
	trimn(scratch, NUMBER_LENGTH);
	BUTTON_SetText(hValueLowButton, scratch);

	sprintf(scratch, "%f", vehicleData.analogInput[globalInput].highValuePoint);
	trimn(scratch, NUMBER_LENGTH);
	BUTTON_SetText(hValueHighButton, scratch);

	GUI_DispStringAt("Voltage", RIGHT_BUTTON_LEFT + TINY_BUTTON_WIDTH + 10, THIRD_BUTTON_ROW + 4);
	GUI_DispStringAt("Range", RIGHT_BUTTON_LEFT + TINY_BUTTON_WIDTH + 10, THIRD_BUTTON_ROW + 26);
	GUI_DispStringAt("Filtering", LEFT_BUTTON_LEFT + 5, THIRD_BUTTON_ROW + 4);
	GUI_DispStringAt("Level", LEFT_BUTTON_LEFT + 28, THIRD_BUTTON_ROW + 26);

	if (vehicleData.analogInput[globalInput].inputRange == 0)
		vehicleData.analogInput[globalInput].inputRange = 10;
	sprintf(scratch, "%d", vehicleData.analogInput[globalInput].inputRange);
	BUTTON_SetText(hVoltsRangeButton, scratch);

	SetFilteringText();

	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}

void SetFilteringText(void)
{
	char *pText;

	switch (vehicleData.analogInput[globalInput].enabled)
	{
	case FILTER_NONE:
	default:				pText = "NONE";		break;
	case FILTER_LOW:		pText = "LOW";		break;
	case FILTER_MEDIUM:		pText = "MEDIUM";	break;
	case FILTER_HIGH:		pText = "HIGH";		break;
	case FILTER_VERY_HIGH:	pText = "VERY\nHIGH";break;
	}
	BUTTON_SetText(hFilteringLevelButton, pText);
}

void DisplayAlarm(WM_HWIN hParent)
{
	BUTTON_SetDefaultBkColor(GUI_BLACK, BUTTON_BI_UNPRESSED);

	hLED1Button = BUTTON_CreateEx(ALARM_INDICATOR_BUTTON_LEFT, TOP_ALARM_INDICATOR_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, LED_1_BUTTON);
	BUTTON_SetBitmap(hLED1Button, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetSkinClassic(hLED1Button);

	hLED2Button = BUTTON_CreateEx(ALARM_INDICATOR_BUTTON_LEFT, SECOND_ALARM_INDICATOR_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, LED_2_BUTTON);
	BUTTON_SetBitmap(hLED2Button, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetSkinClassic(hLED2Button);

	hAlarmConditionButton = BUTTON_CreateEx(ALARM_CONFIG_BOX_H_CENTER - 95, ALARM_CONFIG_ROW_3, TINY_BUTTON_WIDTH, 
		ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, ALARM_CONDITION_BUTTON);
	BUTTON_SetBitmap(hAlarmConditionButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hAlarmConditionButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hAlarmConditionButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hAlarmConditionButton);

	hAlarmValueButton = BUTTON_CreateEx(ALARM_CONFIG_BOX_H_CENTER - 95, ALARM_CONFIG_ROW_4, TINY_BUTTON_WIDTH, 
		ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, ALARM_VALUE_BUTTON);
	BUTTON_SetBitmap(hAlarmValueButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hAlarmValueButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hAlarmValueButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hAlarmValueButton);

	hUpperAlarmConditionButton = BUTTON_CreateEx(ALARM_CONFIG_BOX_H_CENTER + 5, ALARM_CONFIG_ROW_3, 
		TINY_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, UPPER_ALARM_CONDITION_BUTTON);
	BUTTON_SetBitmap(hUpperAlarmConditionButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hUpperAlarmConditionButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hUpperAlarmConditionButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hUpperAlarmConditionButton);

	hUpperAlarmValueButton = BUTTON_CreateEx(ALARM_CONFIG_BOX_H_CENTER + 5, ALARM_CONFIG_ROW_4, 
		TINY_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, UPPER_ALARM_VALUE_BUTTON);
	BUTTON_SetBitmap(hUpperAlarmValueButton, BUTTON_BI_UNPRESSED, &bmTinyBlackButton);
	BUTTON_SetTextColor(hUpperAlarmValueButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hUpperAlarmValueButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hUpperAlarmValueButton);
}

void AlarmPaint(WM_HWIN hParent)
{
	char scratch[30];
	GUI_CONST_STORAGE GUI_BITMAP *pBanner;

	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	//sprintf(scratch, "Edit Alarm %d", globalInput);
	//GUI_DispStringHCenterAt(scratch, XSIZE_PHYS/2, 7);
	switch (globalInput)
	{
	default:
	case 0:	pBanner = &bm0;	break;
	case 1:	pBanner = &bm1;	break;
	case 2:	pBanner = &bm2;	break;
	case 3:	pBanner = &bm3;	break;
	}
	GUI_DrawBitmap(&bmAnalog, 0, 0);
	GUI_DrawBitmap(pBanner, 224, 0);
	GUI_DrawBitmap(&bmAlarmText, 253, 0);


	// Draw grouping rectangle around Alarm LED Indicator Configuration buttons
	GUI_DrawRoundedRect(28, 50, 233, 255, 10);
	GUI_DispStringHCenterAt("Alarm Indicator", 131, ALARM_CONFIG_ROW_1);

	// Draw grouping rectangle around Alarm Trigger Configuration buttons
	GUI_DrawRoundedRect(252, 50, 460, 255, 10);
	GUI_DispStringHCenterAt("Alarm Level", ALARM_CONFIG_BOX_H_CENTER, ALARM_CONFIG_ROW_1);
	GUI_DispStringHCenterAt("Lower", ALARM_CONFIG_BOX_H_CENTER - (ALARM_CONFIG_BOX_H_WIDTH/2), ALARM_CONFIG_ROW_2);
	GUI_DispStringHCenterAt("Upper", ALARM_CONFIG_BOX_H_CENTER + (ALARM_CONFIG_BOX_H_WIDTH/2), ALARM_CONFIG_ROW_2);

	if (vehicleData.analogInput[globalInput].EnableLED1)
	{
		BUTTON_SetTextColor(hLED1Button, BUTTON_BI_UNPRESSED, GUI_RED);
		BUTTON_SetTextColor(hLED1Button, BUTTON_BI_PRESSED, GUI_RED);
		BUTTON_SetText(hLED1Button, "LED 1 Enabled");
	}
	else
	{
		BUTTON_SetTextColor(hLED1Button, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hLED1Button, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetText(hLED1Button, "LED 1 Disabled");
	}

	if (vehicleData.analogInput[globalInput].EnableLED2)
	{
		BUTTON_SetTextColor(hLED2Button, BUTTON_BI_UNPRESSED, GUI_RED);
		BUTTON_SetTextColor(hLED2Button, BUTTON_BI_PRESSED, GUI_RED);
		BUTTON_SetText(hLED2Button, "LED 2 Enabled");
	}
	else
	{
		BUTTON_SetTextColor(hLED2Button, BUTTON_BI_UNPRESSED, GUI_WHITE);
		BUTTON_SetTextColor(hLED2Button, BUTTON_BI_PRESSED, GUI_WHITE);
		BUTTON_SetText(hLED2Button, "LED 2 Disabled");
	}

	BUTTON_SetText(hAlarmConditionButton, GetAlarmTriggerText(globalInput, LOWER));
	BUTTON_SetText(hUpperAlarmConditionButton, GetAlarmTriggerText(globalInput, UPPER));

//	sprintf(scratch, "%d", vehicleData.analogInput[globalInput].lowerAlarmTriggerLevel);
	sprintf(scratch, "%.2f", vehicleData.analogInput[globalInput].lowerAlarmTriggerLevel);
	BUTTON_SetText(hAlarmValueButton, scratch);
	sprintf(scratch, "%.2f", vehicleData.analogInput[globalInput].upperAlarmTriggerLevel);
	BUTTON_SetText(hUpperAlarmValueButton, scratch);

	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}


void DisplayRenameInput(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pRenameInput)
	////	DestroyKeyboard(pRenameInput);

	sprintf(scratch, "Rename Input %d", globalInput);
	pRenameInput = CreateKeyboard(scratch, vehicleData.analogInput[globalInput].Name, ALPHA, false, IONAMELEN, RENAME_INPUT, hParent, true);
}

void RenameInputPaint(WM_HWIN hParent)
{
}

void DisplayRenameUnits(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	////if (pRenameUnits)
	////	DestroyKeyboard(pRenameUnits);

	sprintf(scratch, "Rename Units %d", globalInput);
	pRenameUnits = CreateKeyboard(scratch, vehicleData.analogInput[globalInput].Units, ALPHA, false, MAX_UNITS-1, RENAME_UNITS, hParent, true);
}

void RenameUnitsPaint(WM_HWIN hParent)
{
}


void AnalogInputSettingsIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}


void DeleteAnalogKeyboards(void)
{
	DeleteAnalogScaleKeyboards();
	if (pAlarmValue)		// Should alwaye be there - but just in case!
	{
		DestroyKeyboard(pAlarmValue);
		pAlarmValue = 0;
	}
	if (pRenameUnits)
	{
		DestroyKeyboard(pRenameUnits);
		pRenameUnits = 0;
	}
	if (pRenameInput)
	{
		DestroyKeyboard(pRenameInput);
		pRenameInput = 0;
	}
}

enum INVALIDANALOGDATASCREENS {
	RANGE_TOO_HIGH,
	RANGE_IN_WRONG_ORDER,
	ALARM_IN_WRONG_ORDER
};

int InvalidAnalogDataPopUp(enum INVALIDANALOGDATASCREENS screen);

//
// Validate that the voltage ranges are in the correct order and that they are within the selected range.
//
// Returns: true - carry on
//			false - go back so the user can fix it
//
char ValidateAnalogData(void)
{
	ANALOG_INPUT_STRUCTURE *pData;

	pData = &vehicleData.analogInput[globalInput];

	if (pData->highVoltagePoint <= pData->lowVoltagePoint)
	{
		if (InvalidAnalogDataPopUp(RANGE_IN_WRONG_ORDER) == GUI_ID_CANCEL)
			return false;
		else
		{
			// Auto-fix the data
			if (pData->highVoltagePoint == pData->lowVoltagePoint)
			{
				pData->highVoltagePoint = (float)pData->inputRange;
				pData->lowVoltagePoint = 0.0;
			}
			else
			{
				float temp;

				temp = pData->highVoltagePoint;
				pData->highVoltagePoint = pData->lowVoltagePoint;
				pData->lowVoltagePoint = temp;
			}
		}
	}
	if ((pData->highVoltagePoint > pData->inputRange) || (pData->lowVoltagePoint < 0.0))
	{
		if (InvalidAnalogDataPopUp(RANGE_TOO_HIGH) == GUI_ID_CANCEL)
			return false;
		else
		{
			// Auto-fix the data
			if (pData->lowVoltagePoint < 0.0)
				pData->lowVoltagePoint = 0.0;
			if (pData->highVoltagePoint > pData->inputRange)
			{
				pData->highVoltagePoint = (float)pData->inputRange;	// Check to see if we introduced a new error
				if (pData->highVoltagePoint <= pData->lowVoltagePoint)
					pData->lowVoltagePoint = 0.0;			// Auto-fix the data
			}
		}
	}

	// Validate alarm settings
	if ((pData->lowerAlarmTriggerComparison == BELOW) && (pData->upperAlarmTriggerComparison == ABOVE))
	{
		if (pData->lowerAlarmTriggerLevel > pData->upperAlarmTriggerLevel)
		{
			if (InvalidAnalogDataPopUp(ALARM_IN_WRONG_ORDER) == GUI_ID_CANCEL)
				return false;
			else // Auto-fix the data
			{
				float temp;

				temp = pData->lowerAlarmTriggerLevel;
				pData->lowerAlarmTriggerLevel = pData->upperAlarmTriggerLevel;
				pData->upperAlarmTriggerLevel = temp;
			}
		}
	}

	return true;
}

static const GUI_WIDGET_CREATE_INFO InvalidRangeResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Error", 0, 40, 20, 380, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The specified voltage values are", 0, 0, 10, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "outside the selected range.", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to auto-fix or go back?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Auto-fix",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO InvalidOrderResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Error", 0, 40, 20, 380, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The selected high and low voltage", 0, 0, 10, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "values are in the wrong order.", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to auto-fix or go back?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Auto-fix",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 195, 115, 120, 60 },
};
static const GUI_WIDGET_CREATE_INFO InvalidAlarmOrderResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Error", 0, 40, 20, 380, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The selected high and low alarm", 0, 0, 10, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "values are in the wrong order.", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to auto-fix or go back?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Auto-fix",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 195, 115, 120, 60 },
};

//
// Tell user about the bad analog data
//
// Returns	GUI_ID_OK - continue
//			GUI_ID_CANCEL - go back and fix it
//
int InvalidAnalogDataPopUp(enum INVALIDANALOGDATASCREENS screen)
{
	SetPopupDefaults(WARNING_POPUP);

	switch (screen)
	{
		case RANGE_TOO_HIGH:
			return GUI_ExecDialogBox(InvalidRangeResources, GUI_COUNTOF(InvalidRangeResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case RANGE_IN_WRONG_ORDER:
			return GUI_ExecDialogBox(InvalidOrderResources, GUI_COUNTOF(InvalidOrderResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
		case ALARM_IN_WRONG_ORDER:
			return GUI_ExecDialogBox(InvalidAlarmOrderResources, GUI_COUNTOF(InvalidAlarmOrderResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
	}
	
	return false;
}


/*************************** End of file ****************************/

