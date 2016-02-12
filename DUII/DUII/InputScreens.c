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
#include "InputScreens.h"
#include "StandardList.h"
#include "AnalogInputSettings.h"
#include "DigitalInputSettings.h"
#include "InterTask.h"
#include "VehicleSettingsScreens.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
char globalInput = 0;

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmGrayButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmDummyDigital;
extern GUI_CONST_STORAGE GUI_BITMAP bmDummyAnalog;
extern GUI_CONST_STORAGE GUI_BITMAP bmAnalog;
extern GUI_CONST_STORAGE GUI_BITMAP bmDigital;
extern GUI_CONST_STORAGE GUI_BITMAP bm0;
extern GUI_CONST_STORAGE GUI_BITMAP bm1;
extern GUI_CONST_STORAGE GUI_BITMAP bm2;
extern GUI_CONST_STORAGE GUI_BITMAP bm3;
extern GUI_CONST_STORAGE GUI_BITMAP bm4;
extern GUI_CONST_STORAGE GUI_BITMAP bm5;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void SetAnalogEnableButtonText(BUTTON_Handle hBtn, int input);
void SetDigitalEnableButtonText(BUTTON_Handle hBtn, int input);
int ScreenIDToInput(int input);

void DisplayAnalog(char, WM_HWIN);
void DisplayAnalog0(WM_HWIN);
void DisplayAnalog1(WM_HWIN);
void DisplayAnalog2(WM_HWIN);
void DisplayAnalog3(WM_HWIN);
void DisplayDigital(char, WM_HWIN);
void DisplayDigital4(WM_HWIN);
void DisplayDigital5(WM_HWIN);

void AnalogPaint(char, WM_HWIN);
void Analog0Paint(WM_HWIN);
void Analog1Paint(WM_HWIN);
void Analog2Paint(WM_HWIN);
void Analog3Paint(WM_HWIN);
void DigitalPaint(char, WM_HWIN);
void Digital4Paint(WM_HWIN);
void Digital5Paint(WM_HWIN);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST InputsScreenList[NUM_INPUTS_SCREENS+1] = {
	{ ANALOG0, DisplayAnalog0, Analog0Paint },
	{ ANALOG1, DisplayAnalog1, Analog1Paint },
	{ ANALOG2, DisplayAnalog2, Analog2Paint },
	{ ANALOG3, DisplayAnalog3, Analog3Paint },
	{ DIGITAL4, DisplayDigital4, Digital4Paint },
	{ DIGITAL5, DisplayDigital5, Digital5Paint },
	{ -1, NULL, NULL }
};

#ifdef TEST_KEYBOARD
#define	ID_Q		GUI_ID_USER + 1
#define	ID_W		GUI_ID_USER + 2
#define	ID_E		GUI_ID_USER + 3
#define	ID_R		GUI_ID_USER + 4
#define	ID_T		GUI_ID_USER + 5
#define	ID_Y		GUI_ID_USER + 6
#endif

#define	ENABLE_BUTTON			GUI_ID_USER + 1
#define	EDIT_SCALE_BUTTON		GUI_ID_USER + 2
#define	EDIT_ALARM_BUTTON		GUI_ID_USER + 3
#define	RENAME_INPUT_BUTTON		GUI_ID_USER + 4
#define	RENAME_UNITS_BUTTON		GUI_ID_USER + 5
#define	RENAME_HIGH_BUTTON		GUI_ID_USER + 6
#define	RENAME_LOW_BUTTON		GUI_ID_USER + 7

int test;

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hInputsScreens;
//static int KMCDialogSemaphone = 0;
//static int clickedOnThisScreen;


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetInputsWindowHandle(void)
{
	return hInputsScreens;
}

void SetInputsWindowHandle(WM_HWIN hWin)
{
	hInputsScreens = hWin;
}

void InputsScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	int input;
//	int results;
//	int Sel;
//	WM_HWIN hDlg;

//	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
//				WM_InvalidateRect(pMsg->hWin, &SignalStrengthRect);
				WM_InvalidateWindow(pMsg->hWin);		// Allow timer to invalidate the whole window so we can show live data on the inputs
			}
			break;

		case WM_CREATE:
			InputsIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == INPUTS_SCREENS_TYPE)
			{
				// Call paint method of center screen
				InputsScreenList[GetLastScreen(INPUTS_SCREENS_TYPE)].paint(hInputsScreens);
				DrawSignalStrength();
			}
			break;

#ifdef TEST_KEYBOARD
		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_CLICKED:
					switch (Id)
					{
						case ID_Q:
							test++;
							break;

						case ID_W:
							test--;
							break;

						default:
							test = 0;
							break;
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
//					if (!clickedOnThisScreen)
//						break;

					switch (Id)
					{
						case ID_Q:
							test++;
							break;

						default:
							test = 0;
							break;
					}
					break;
			}
			break;
#endif

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_RELEASED:
					//pressingbutton_semaphore = 0;
					//if (!clickedOnThisScreen)
					//	break;

					input = GetCurrentScreen();
					input = ScreenIDToInput(input);
					if (input < 4)
						globalInput = input;
					else globalInput = input - 4;

					switch (Id)
					{
						case ENABLE_BUTTON:
							if (input < 4)
							{
								if (vehicleData.analogInput[input].enabled)
									vehicleData.analogInput[input].enabled = false;
								else vehicleData.analogInput[input].enabled = FILTER_NONE;	// no averaging
								
								SetAnalogEnableButtonText(pMsg->hWinSrc, input);
							}
							else
							{
								// If user is attempting to enable digital input 4
								if ((input == 4) && (vehicleData.digitalInput[0].enabled == false))
								{
									// Input 4 mutually exclusive with camera.  If camera enabled warn that this option is not allowed
									if (GetSUCam() != CAMERA_NONE)
									{
										static const GUI_WIDGET_CREATE_INFO Input4DisabledResources[] = {
											{ FRAMEWIN_CreateIndirect, "Invalid Setting", 0, 30, 20, 400, 220, 0, 0 },
											{ TEXT_CreateIndirect, "You cannot enable Digital Input 4", 0, 0, 10, 390, 30, TEXT_CF_HCENTER },
											{ TEXT_CreateIndirect, "if you have a camera selected.",    0, 0, 40, 390, 30, TEXT_CF_HCENTER },
											{ TEXT_CreateIndirect, "Disable camera to use Input 4.",    0, 0, 70, 390, 30, TEXT_CF_HCENTER },
											{ BUTTON_CreateIndirect, "OK", GUI_ID_OK, 145, 115, 100, 60 },
										};

										SetPopupDefaults(ERROR_POPUP);
										GUI_ExecDialogBox(Input4DisabledResources, GUI_COUNTOF(Input4DisabledResources), &popupCallback, hInputsScreens, 10, 10);
										break;
									}
								}
								if (vehicleData.digitalInput[globalInput].enabled)
									vehicleData.digitalInput[globalInput].enabled = false;
								else vehicleData.digitalInput[globalInput].enabled = true;
							
								SetDigitalEnableButtonText(pMsg->hWinSrc, globalInput);
							}
							notifyConfigChange();
							WM_InvalidateWindow(WM_GetParent(pMsg->hWinSrc));
							break;

						case EDIT_SCALE_BUTTON:
							if (input < 4)
								DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, SCALE, hInputsScreens);
							break;

						case EDIT_ALARM_BUTTON:
							if (input < 4)
								DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, ALARM, hInputsScreens);
							else DisplayNewScreenList(DIGITAL_SETTINGS_SCREEN_TYPE, DIG_ALARM_STATE, hInputsScreens);
							break;

						case RENAME_INPUT_BUTTON:
							if (input < 4)
								DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, RENAME_INPUT, hInputsScreens);
							else DisplayNewScreenList(DIGITAL_SETTINGS_SCREEN_TYPE, DIG_RENAME_INPUT, hInputsScreens);
							break;

						case RENAME_UNITS_BUTTON:
							DisplayNewScreenList(ANALOG_SETTINGS_SCREEN_TYPE, RENAME_UNITS, hInputsScreens);
							break;

						case RENAME_HIGH_BUTTON:
							DisplayNewScreenList(DIGITAL_SETTINGS_SCREEN_TYPE, DIG_RENAME_HIGH, hInputsScreens);
							break;

						case RENAME_LOW_BUTTON:
							DisplayNewScreenList(DIGITAL_SETTINGS_SCREEN_TYPE, DIG_RENAME_LOW, hInputsScreens);
							break;
					}
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplayAnalog0(WM_HWIN hParent)
{
	DisplayAnalog(0, hParent);
}
void DisplayAnalog1(WM_HWIN hParent)
{
	DisplayAnalog(1, hParent);
}
void DisplayAnalog2(WM_HWIN hParent)
{
	DisplayAnalog(2, hParent);
}
void DisplayAnalog3(WM_HWIN hParent)
{
	DisplayAnalog(3, hParent);
}

void SetAnalogEnableButtonText(BUTTON_Handle hBtn, int input)
{
	char scratch[20];

	sprintf(scratch, "%s Analog %d", vehicleData.analogInput[input].enabled ? "Disable" : "Enable", input);
	BUTTON_SetText(hBtn, scratch);
}

void SetDigitalEnableButtonText(BUTTON_Handle hBtn, int input)
{
	char scratch[20];

	sprintf(scratch, "%s Digital %d", vehicleData.digitalInput[input].enabled ? "Disable" : "Enable", input+4);
	BUTTON_SetText(hBtn, scratch);
}

#define	LINE1					30			
#define	LINE2					70		
#define	LINE3					100
#define	LINE_SPACING			30
#define	LINE_GAP				15
#define	GEAR_TABLE_TOP			(LINE3 + 7)
#define	GEAR_TABLE_VGAP			25
#define	SECOND_COL_GAP			((((BOX_RIGHT - LINE_GAP) - (BOX_LEFT + LINE_GAP)) / 4) + LINE_GAP)
#define	THIRD_COL_GAP			((BOX_RIGHT - BOX_LEFT)/2 + (LINE_GAP/2))
#define	FORTH_COL_GAP			(((((BOX_RIGHT - LINE_GAP) - (BOX_LEFT + LINE_GAP)) / 4) * 3) + LINE_GAP)
#define	FORTH_COL_POS			(BOX_RIGHT - 52)
#define	LED1_POS				(BOX_LEFT + 25)
#define	LED2_POS				(BOX_RIGHT - 70)
#define FILTER_VPOS				(BOX_RIGHT - 37)
#define	LED_VPOS				(BOX_BOTTOM - 25)


void Analog0Paint(WM_HWIN hParent)
{
	AnalogPaint(0, hParent);
}
void Analog1Paint(WM_HWIN hParent)
{
	AnalogPaint(1, hParent);
}

void Analog2Paint(WM_HWIN hParent)
{
	AnalogPaint(2, hParent);
}

void Analog3Paint(WM_HWIN hParent)
{
	AnalogPaint(3, hParent);
}


void DisplayAnalog(char input, WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 48, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, ENABLE_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	SetAnalogEnableButtonText(hBtn, input);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 91, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_SCALE_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmYellowButton);
	BUTTON_SetText(hBtn, "Edit Scale");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 134, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_ALARM_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "Edit Alarm");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 177, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RENAME_INPUT_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Rename Input");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 220, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RENAME_UNITS_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Rename Units");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
}

void AnalogPaint(char input, WM_HWIN hParent)
{
	char scratch[30];
	GUI_CONST_STORAGE GUI_BITMAP *pBanner;

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	switch (input)
	{
	default:
	case 0:	pBanner = &bm0;	break;
	case 1:	pBanner = &bm1;	break;
	case 2:	pBanner = &bm2;	break;
	case 3:	pBanner = &bm3;	break;
	}
	GUI_DrawBitmap(&bmAnalog, 60, 0);
	GUI_DrawBitmap(pBanner, 284, 0);

	// Draw left side square for data
	GUI_DrawRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);

	GUI_DrawBitmap(GetNavigationArrow(0), 0, 229);

	// If alarm disabled - display disabled and exit!
	if (vehicleData.analogInput[input].enabled == false)
	{
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_SetColor(GUI_GRAY);
		GUI_DispStringHCenterAt("Input Disabled", BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + 4);
		return;
	}

	// draw 3 gray lines for field seperators
	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE1, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE1);

	GUI_SetPenSize(2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE2, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE3, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE3);

	// Draw text:
	// Draw alarm trigger state
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_RED);

//		upper = none, lower = none, text = "No Alarm Set"
//		upper = U, lower = L, text = "L < Alarm > U"
//		upper = U, lower = none, text = "Alarm > U"
//		upper = none, lower = L, text = "L < Alarm"

	if (OFF == vehicleData.analogInput[input].lowerAlarmTriggerComparison && OFF == vehicleData.analogInput[input].upperAlarmTriggerComparison)
		strcpy(scratch, "No Alarm Set");
	else {
		scratch[0] = '\0';
		if (BELOW == vehicleData.analogInput[input].lowerAlarmTriggerComparison)
			sprintf(scratch, "%.2f < ", vehicleData.analogInput[input].lowerAlarmTriggerLevel);
		strcat(scratch, "Alarm");
		if (ABOVE == vehicleData.analogInput[input].upperAlarmTriggerComparison)
			sprintf(scratch, "%s > %.2f", scratch, vehicleData.analogInput[input].upperAlarmTriggerLevel);
	} // else

	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + 4);

	// Actual Voltage reading (live data)
	GUI_SetFont(&GUI_Font32_ASCII);
	GUI_SetColor(GUI_GREEN);
	sprintf(scratch, "Volts: %0.2f", GetAnalogInput(RAW, input));
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + LINE1 + 4);

	// Voltage range selected
	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_SetColor(GUI_WHITE);
	sprintf(scratch, "%dV Range Selected", vehicleData.analogInput[input].inputRange);
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + LINE2 + 6);

	// Low and high graph points text
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt("Low:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringAt("High:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
	GUI_DispStringAt("Low:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringAt("High:", BOX_LEFT + THIRD_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);

	// Draw graph point data values
	GUI_SetColor(GUI_GREEN);
	sprintf(scratch, "%.2f", vehicleData.analogInput[input].lowVoltagePoint);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP);
	sprintf(scratch, "%.2f", vehicleData.analogInput[input].highVoltagePoint);
	GUI_DispStringAt(scratch, BOX_LEFT + SECOND_COL_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);
//	sprintf(scratch, "%.2f", vehicleData.analogInput[input].lowValuePoint);
	sprintf(scratch, "%d", (int)vehicleData.analogInput[input].lowValuePoint);
	GUI_DispStringAt(scratch, FORTH_COL_POS, BOX_TOP + GEAR_TABLE_TOP);
//	sprintf(scratch, "%.2f", vehicleData.analogInput[input].highValuePoint);
	sprintf(scratch, "%d", (int)vehicleData.analogInput[input].highValuePoint);
	GUI_DispStringAt(scratch, FORTH_COL_POS, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);

	// Filtering level selected
	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_SetColor(GUI_LIGHTBLUE);
	switch (vehicleData.analogInput[input].enabled) {
		case FILTER_NONE:
			strcpy (scratch, "NO");
			break;
		case FILTER_LOW:
			strcpy (scratch, "LOW");
		  break;
		case FILTER_MEDIUM:
			strcpy (scratch, "MEDIUM");
		  break;
		case FILTER_HIGH:
			strcpy (scratch, "HIGH");
			break;
		case FILTER_VERY_HIGH:
			strcpy (scratch, "VERY HIGH");
			break;
	} // switch
	strcat(scratch, " Filtering");
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), FILTER_VPOS);
	
	// Draw LED config states
	GUI_SetColor(GUI_RED);
	if (vehicleData.analogInput[input].EnableLED1)
		GUI_DispStringAt("LED1", LED1_POS, LED_VPOS);
	if (vehicleData.analogInput[input].EnableLED2)
		GUI_DispStringAt("LED2", LED2_POS, LED_VPOS);
}


void DisplayDigital(char input, WM_HWIN hParent)
{
	BUTTON_Handle hBtn;

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 48, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, ENABLE_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGreenButton);
	SetDigitalEnableButtonText(hBtn, input);
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 91, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, EDIT_ALARM_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hBtn, "Edit Alarm");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 134, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RENAME_INPUT_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Rename Input");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 177, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RENAME_HIGH_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Rename High");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	hBtn = BUTTON_CreateEx(BUTTON_LEFT, 220, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, RENAME_LOW_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmGrayButton);
	BUTTON_SetText(hBtn, "Rename Low");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
}

void DigitalPaint(char input, WM_HWIN hParent)
{
	char scratch[30];
	GUI_CONST_STORAGE GUI_BITMAP *pBanner;

	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	if (input == 0)
		pBanner = &bm4;
	else pBanner = &bm5;

	GUI_DrawBitmap(&bmDigital, 60, 1);
	GUI_DrawBitmap(pBanner, 284, 1);

	// Draw left side square for data
	GUI_DrawRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);
	GUI_SetColor(GUI_GRAY);
	GUI_FillRoundedRect(BOX_LEFT, BOX_TOP, BOX_RIGHT, BOX_BOTTOM, 5);

	GUI_DrawRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);
	GUI_SetColor(GUI_BLACK);
	GUI_FillRoundedRect(BOX_LEFT+2, BOX_TOP+2, BOX_RIGHT-2, BOX_BOTTOM - 2, 5);

	GUI_DrawBitmap(GetNavigationArrow(0), 0, 229);

	// If alarm disabled - display disabled and exit!
	if (vehicleData.digitalInput[input].enabled == false)
	{
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_SetColor(GUI_GRAY);
		GUI_DispStringHCenterAt("Input Disabled", BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + 4);
		return;
	}

	// draw 3 gray lines for field seperators
	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE1, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE1);

	GUI_SetPenSize(2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE2, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE2);
	GUI_DrawLine(BOX_LEFT + LINE_GAP, BOX_TOP + LINE3, BOX_RIGHT - LINE_GAP, BOX_TOP + LINE3);

	// Draw text:
	// Draw alarm trigger state
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_RED);
	
	//sprintf(scratch, "Alarm when %s", vehicleData.digitalInput[input].alarmTriggerState ? "HIGH" : "LOW");
	switch (vehicleData.digitalInput[globalInput].alarmTriggerState)
	{
	case DI_DISABLED:
		strcpy(scratch, "Alarm Disabled");
		break;
	case DI_ON:
		strcpy(scratch, "Alarm when HIGH");
		break;
	case DI_OFF:
		strcpy(scratch, "Alarm when LOW");
		break;
	}

	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + 4);

	// Display actual Digital reading (live data)
	GUI_SetFont(&GUI_Font32_ASCII);
	GUI_SetColor(GUI_GREEN);
	sprintf(scratch, "State: %s", GetDigitalText(RAW, input));
	GUI_DispStringHCenterAt(scratch, BOX_LEFT + ((BOX_RIGHT - BOX_LEFT)/2), BOX_TOP + LINE1 + 4);

	// Low and high names text
	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringAt("HIGH Name:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringAt("LOW Name:", BOX_LEFT + LINE_GAP, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);

	// Draw high and low state names
	GUI_SetColor(GUI_GREEN);
	GUI_DispStringHCenterAt(vehicleData.digitalInput[input].HighName, BOX_LEFT + FORTH_COL_GAP + 10, BOX_TOP + GEAR_TABLE_TOP);
	GUI_DispStringHCenterAt(vehicleData.digitalInput[input].LowName, BOX_LEFT + FORTH_COL_GAP + 10, BOX_TOP + GEAR_TABLE_TOP + GEAR_TABLE_VGAP);

	// Draw LED config states
	GUI_SetColor(GUI_RED);
	if (vehicleData.digitalInput[input].EnableLED1)
		GUI_DispStringAt("LED1", LED1_POS, LED_VPOS);
	if (vehicleData.digitalInput[input].EnableLED2)
		GUI_DispStringAt("LED2", LED2_POS, LED_VPOS);
}


//void DisplayAnalog1(WM_HWIN hParent)
//{
//#ifdef TEST_KEYBOARD
//	BUTTON_Handle hButton;
//
//	BUTTON_SetDefaultBkColor(GUI_WHITE, BUTTON_BI_UNPRESSED);
//	BUTTON_SetDefaultTextColor(BUTTON_BI_UNPRESSED, GUI_BLACK);
//	BUTTON_SetDefaultFont(&GUI_Font32B_ASCII);
//	hButton = BUTTON_CreateEx(23, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_Q);
//	BUTTON_SetText(hButton, "Q");
//
//	hButton = BUTTON_CreateEx(67, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_W);
//	BUTTON_SetText(hButton, "W");
//
//	hButton = BUTTON_CreateEx(111, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_E);
//	BUTTON_SetText(hButton, "E");
//
//	hButton = BUTTON_CreateEx(157, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_R);
//	BUTTON_SetText(hButton, "R");
//
//	hButton = BUTTON_CreateEx(202, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_T);
//	BUTTON_SetText(hButton, "T");
//
//	hButton = BUTTON_CreateEx(248, 96, 35, 33, hParent, WM_CF_SHOW, 0, ID_Y);
//	BUTTON_SetText(hButton, "Y");
//#endif
//}

void DisplayDigital4(WM_HWIN hParent)
{
	DisplayDigital(0, hParent);
}

void DisplayDigital5(WM_HWIN hParent)
{
	DisplayDigital(1, hParent);
}

//void Analog1Paint(WM_HWIN hParent)
//{
//#ifdef TEST_KEYBOARD
//	GUI_DrawBitmap(&bmKMC_TestKeyboard, 0, 0);
//#else
//	GUI_DrawBitmap(&bmBackground, 0, 0);
//	GUI_DrawBitmap(&bmDummyAnalog, 5, 35);
//	GUI_DrawBitmap(&bmanalog1, 60, 1);
//	GUI_DrawBitmap(GetNavigationArrow(0), 0, 229);
//#endif
//}


void Digital4Paint(WM_HWIN hParent)
{
	DigitalPaint(0, hParent);
}


void Digital5Paint(WM_HWIN hParent)
{
	DigitalPaint(1, hParent);
}


static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void InputsIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}

int ScreenIDToInput(int input)
{
	switch (input)
	{
		case ANALOG0:	return 0;
		case ANALOG1:	return 1;
		case ANALOG2:	return 2;
		case ANALOG3:	return 3;
		case DIGITAL4:	return 4;
		case DIGITAL5:	return 5;
		default:			return 0;
	}
}

float GetAnalogInput(enum RAW_OR_USER type, int input)
{
	switch (input)
	{
		case 0:	return type == RAW ? GetRawValue(ANALOG_INPUT_0).fVal : GetValue(ANALOG_INPUT_0).fVal;
		case 1:	return type == RAW ? GetRawValue(ANALOG_INPUT_1).fVal : GetValue(ANALOG_INPUT_1).fVal;
		case 2:	return type == RAW ? GetRawValue(ANALOG_INPUT_2).fVal : GetValue(ANALOG_INPUT_2).fVal;
		case 3:	return type == RAW ? GetRawValue(ANALOG_INPUT_3).fVal : GetValue(ANALOG_INPUT_3).fVal;
		default:	return (float)0.0;
	}
}

char *GetDigitalText(enum RAW_OR_USER type, int input)
{
	char value;

	if (input == 0)
		value = (char)GetValue(DIGITAL_INPUT_4).uVal;
	else value = (char)GetValue(DIGITAL_INPUT_5).uVal;

	if (value)
		return type == RAW ? "HIGH" : vehicleData.digitalInput[input].HighName;
	else return type == RAW ? "LOW" : vehicleData.digitalInput[input].LowName;
}

char *GetAlarmTriggerText(char input, enum UPPER_LOWER upperLower)
{
	switch ((upperLower == UPPER ? vehicleData.analogInput[input].upperAlarmTriggerComparison : vehicleData.analogInput[input].lowerAlarmTriggerComparison))
	{
		case ABOVE:		return "Above";		
		case BELOW:		return "Below";		
		case EQUAL_TO:	return "Equal To";
		default:
		case OFF:		return "Off";			
	}
}

void InputCleanUpKeyboards(void)
{
	DeleteDigitalKeyboards();
	DeleteAnalogKeyboards();
}

/*************************** End of file ****************************/

