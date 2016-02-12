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
#include "DigitalInputSettings.h"
#include "StandardList.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"

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
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmDummyDigital;
extern GUI_CONST_STORAGE GUI_BITMAP bmDigital;
extern GUI_CONST_STORAGE GUI_BITMAP bmDigitalAlarm;
extern GUI_CONST_STORAGE GUI_BITMAP bm4;
extern GUI_CONST_STORAGE GUI_BITMAP bm5;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/

void DigDisplayAlarm(WM_HWIN);
void DigDisplayRenameInput(WM_HWIN hParent);
void DigRenameInputPaint(WM_HWIN hParent);
void DisplayRenameHigh(WM_HWIN hParent);
void DisplayRenameLow(WM_HWIN hParent);
void RenameHighPaint(WM_HWIN hParent);
void RenameLowPaint(WM_HWIN hParent);
void DigAlarmPaint(WM_HWIN);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST DigitalInputSettingsScreenList[NUM_DIGITAL_INPUT_SCREENS+1] = {
	{ DIG_ALARM_STATE, DigDisplayAlarm, DigAlarmPaint },
	{ DIG_RENAME_INPUT, DigDisplayRenameInput, DigRenameInputPaint },
	{ DIG_RENAME_HIGH, DisplayRenameHigh, RenameHighPaint },
	{ DIG_RENAME_LOW, DisplayRenameLow, RenameLowPaint },
	{ -1, NULL, NULL }
};

#define	LED_1_BUTTON			GUI_ID_USER + 1
#define	LED_2_BUTTON			GUI_ID_USER + 2
#define	ALARM_STATE_BUTTON		GUI_ID_USER + 3

#define	LEFT_BUTTON_LEFT		25
#define	RIGHT_BUTTON_LEFT		284
#define	ANA_BUTTON_WIDTH		180
#define	ANA_BUTTON_HEIGHT		53
#define	TOP_BUTTON_ROW			76
#define	SECOND_BUTTON_ROW		138
#define	THIRD_BUTTON_ROW		200

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hDigitalInputSettings;
static BUTTON_Handle hLED1Button, hLED2Button, hAlarmStateButton;
static void *pRenameHigh = 0;
static void *pRenameLow = 0;
static void *pRenameInput = 0;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetDigitalInputSettingsWindowHandle(void)
{
	return hDigitalInputSettings;
}

void SetDigitalInputSettingsWindowHandle(WM_HWIN hWin)
{
	hDigitalInputSettings = hWin;
}

void DigitalInputSettingsCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
		case KB_SAVE_AND_EXIT_MSG:
			DisplayNewScreenList(INPUTS_SCREENS_TYPE, GetLastScreen(INPUTS_SCREENS_TYPE), hDigitalInputSettings);
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
			DigitalInputSettingsIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == DIGITAL_SETTINGS_SCREEN_TYPE)
			{
				// Call paint method of center screen
				DigitalInputSettingsScreenList[GetLastScreen(DIGITAL_SETTINGS_SCREEN_TYPE)].paint(hDigitalInputSettings);
				// Call paint method of next screen (so it is valid for peeking)
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
							if (vehicleData.digitalInput[globalInput].EnableLED1)
								vehicleData.digitalInput[globalInput].EnableLED1 = false;
							else vehicleData.digitalInput[globalInput].EnableLED1 = true;
							break;

						case LED_2_BUTTON:
							if (vehicleData.digitalInput[globalInput].EnableLED2)
								vehicleData.digitalInput[globalInput].EnableLED2 = false;
							else vehicleData.digitalInput[globalInput].EnableLED2 = true;
							break;

						case ALARM_STATE_BUTTON:
							if (vehicleData.digitalInput[globalInput].alarmTriggerState == DI_DISABLED)
								vehicleData.digitalInput[globalInput].alarmTriggerState = DI_OFF;
							else if (vehicleData.digitalInput[globalInput].alarmTriggerState == DI_OFF)
								vehicleData.digitalInput[globalInput].alarmTriggerState = DI_ON;
							else vehicleData.digitalInput[globalInput].alarmTriggerState = DI_DISABLED;
							break;
					}

					WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}



void DigDisplayAlarm(WM_HWIN hParent)
{
	hLED1Button = BUTTON_CreateEx(LEFT_BUTTON_LEFT, TOP_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, LED_1_BUTTON);
	BUTTON_SetBitmap(hLED1Button, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetSkinClassic(hLED1Button);

	hLED2Button = BUTTON_CreateEx(LEFT_BUTTON_LEFT, SECOND_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, LED_2_BUTTON);
	BUTTON_SetBitmap(hLED2Button, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetSkinClassic(hLED2Button);

	hAlarmStateButton = BUTTON_CreateEx(RIGHT_BUTTON_LEFT, TOP_BUTTON_ROW, ANA_BUTTON_WIDTH, ANA_BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, ALARM_STATE_BUTTON);
	BUTTON_SetBitmap(hAlarmStateButton, BUTTON_BI_UNPRESSED, &bmMiniButton);
	BUTTON_SetTextColor(hAlarmStateButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hAlarmStateButton, BUTTON_BI_PRESSED, GUI_WHITE);
	BUTTON_SetSkinClassic(hAlarmStateButton);
}

void DigAlarmPaint(WM_HWIN hParent)
{
	//char scratch[30];
	GUI_CONST_STORAGE GUI_BITMAP *pBanner;

	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	//sprintf(scratch, "Edit Alarm %d", globalInput + 4);
	//GUI_DispStringHCenterAt(scratch, XSIZE_PHYS/2, 7);
	switch (globalInput)
	{
	default:
	case 0:	pBanner = &bm4;	break;
	case 1:	pBanner = &bm5;	break;
	}
	GUI_DrawBitmap(&bmDigital, 0, 0);
	GUI_DrawBitmap(pBanner, 224, 0);
	GUI_DrawBitmap(&bmDigitalAlarm, 250, 0);

	GUI_DispStringHCenterAt("Alarm Indicator", LEFT_BUTTON_LEFT + (ANA_BUTTON_WIDTH/2), TOP_BUTTON_ROW - 28);
	GUI_DispStringHCenterAt("Alarm State", RIGHT_BUTTON_LEFT + (ANA_BUTTON_WIDTH/2), TOP_BUTTON_ROW - 28);

	if (vehicleData.digitalInput[globalInput].EnableLED1)
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

	if (vehicleData.digitalInput[globalInput].EnableLED2)
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

	switch (vehicleData.digitalInput[globalInput].alarmTriggerState)
	{
	case DI_DISABLED:
		BUTTON_SetText(hAlarmStateButton, "Disabled");
		break;
	case DI_ON:
		BUTTON_SetText(hAlarmStateButton, vehicleData.digitalInput[globalInput].HighName);
		break;
	case DI_OFF:
		BUTTON_SetText(hAlarmStateButton, vehicleData.digitalInput[globalInput].LowName);
		break;
	}
	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}


void DigDisplayRenameInput(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	//if (pRenameInput)
	//	DestroyKeyboard(pRenameInput);
	sprintf(scratch, "Rename Input %d", globalInput + 4);
	pRenameInput = CreateKeyboard(scratch, vehicleData.digitalInput[globalInput].Name, ALPHA, false, IONAMELEN, DIG_RENAME_INPUT, hParent, true);
}


void DigRenameInputPaint(WM_HWIN hParent)
{
}

void DisplayRenameHigh(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	//if (pRenameHigh)
	//	DestroyKeyboard(pRenameHigh);
	sprintf(scratch, "Rename High %d", globalInput + 4);
	pRenameHigh = CreateKeyboard(scratch, vehicleData.digitalInput[globalInput].HighName, ALPHA, false, MAX_HILO_NAME, DIG_RENAME_HIGH, hParent, true);
}

void RenameHighPaint(WM_HWIN hParent)
{
}

void DisplayRenameLow(WM_HWIN hParent)
{
	char scratch[30];

	InputCleanUpKeyboards();
	//if (pRenameLow)
	//	DestroyKeyboard(pRenameLow);
	sprintf(scratch, "Rename Low %d", globalInput + 4);
	pRenameLow = CreateKeyboard(scratch, vehicleData.digitalInput[globalInput].LowName, ALPHA, false, MAX_HILO_NAME, DIG_RENAME_LOW, hParent, true);
}

void RenameLowPaint(WM_HWIN hParent)
{
}


void DigitalInputSettingsIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}

void DeleteDigitalKeyboards(void)
{
	if (pRenameHigh)
	{
		DestroyKeyboard(pRenameHigh);
		pRenameHigh = 0;
	}
	if (pRenameLow)
	{
		DestroyKeyboard(pRenameLow);
		pRenameLow = 0;
	}
	if (pRenameInput)
	{
		DestroyKeyboard(pRenameInput);
		pRenameInput = 0;
	}
}

/*************************** End of file ****************************/

