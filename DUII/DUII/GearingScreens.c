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
#include "StandardList.h"
#include "GearingScreens.h"
#include "GearRatios.h"
#include "Utilities.h"
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
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmNarrowSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmTireSize;
extern GUI_CONST_STORAGE GUI_BITMAP bmDiffRatio;
extern GUI_CONST_STORAGE GUI_BITMAP bmTransRatio;
extern GUI_CONST_STORAGE GUI_BITMAP bmHelp;
extern GUI_CONST_STORAGE GUI_BITMAP bmGearRatioButton;

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void DisplayTireSize(void);
void DisplayTireCircumference(void);
void DisplayDiffRatio(WM_HWIN);
void DisplayTransRatio(WM_HWIN);
void DisplayHelp(WM_HWIN);

void TireSizePaint(WM_HWIN);
void DiffRatioPaint(WM_HWIN);
void TransRatioPaint(WM_HWIN);
void HelpPaint(WM_HWIN);
void SetGearButtonText(int button);
int ButtonToScreenID(int Id);
void DisplayTireSizeSelection(WM_HWIN hParent);
void TireSizeSelectionPaint(WM_HWIN hParent);
static void TireSizeScreenCallback(WM_MESSAGE * pMsg);
void TCKeyboardSaveButton(void);
void TCKillKeyboard(void);
void UpdateTireCircumference(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST GearingScreenList[NUM_GEARING_SCREENS+1] = {
	{ TIRESIZE_SELECTION_SCREEN, DisplayTireSizeSelection, TireSizeSelectionPaint },
	{ DIFFRATIO_SCREEN, DisplayDiffRatio, DiffRatioPaint },
	{ TRANSRATIO_SCREEN, DisplayTransRatio, TransRatioPaint },
	{ HELP_SCREEN, DisplayHelp, HelpPaint },
	{ -1, NULL, NULL }
};

#define	TREAD_WIDTH_WHEEL		GUI_ID_LISTWHEEL1
#define	ASPECT_RATIO_WHEEL		GUI_ID_LISTWHEEL2
#define	WHEEL_DIAMETER_WHEEL	GUI_ID_LISTWHEEL3
#define	DIFF_WHEEL_1			GUI_ID_LISTWHEEL4
#define	DIFF_WHEEL_2			GUI_ID_LISTWHEEL5
#define	DIFF_WHEEL_3			GUI_ID_LISTWHEEL6

#define	TIRE_CIRCUMFERENCE_BUTTON		GUI_ID_USER + 1
// Note: the gear 8 buttons definitions MUST BE SEQUENTIAL
#define	FIRST_BUTTON					(GUI_ID_USER + 2)
#define	SECOND_BUTTON					GUI_ID_USER + 3 
#define	THIRD_BUTTON					GUI_ID_USER + 4
#define	FORTH_BUTTON					GUI_ID_USER + 5
#define	FIFTH_BUTTON					GUI_ID_USER + 6
#define	SIXTH_BUTTON					GUI_ID_USER + 7
#define	SEVENTH_BUTTON					GUI_ID_USER + 8
#define	EIGHTH_BUTTON					GUI_ID_USER + 9
#define	TIRE_DIMENSIONS_BUTTON			GUI_ID_USER + 10

char *ZeroThroughNine[] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	NULL
};

char *ZeroThroughNinty[] = {
	"00",
	"10",
	"20",
	"30",
	"40",
	"50",
	"60",
	"70",
	"80",
	"90",
	NULL
};

#define	TREAD_WIDTH_START	355
char *TreadWidth[] = {
	"355",
	"345",
	"335",
	"325",
	"315",
	"305",
	"295",
	"285",
	"275",
	"265",
	"255",
	"245",
	"235",
	"225",
	"215",
	"205",
	"195",
	"185",
	"175",
	"165",
	"155",
	"145",
	"135",
	NULL
};

#define	ASPECT_RATIO_START	80
char *AspectRatio[] = {
	"80",
	"75",
	"70",
	"65",
	"60",
	"55",
	"50",
	"45",
	"40",
	"35",
	"30",
	"25",
	"20",
	NULL
};

#define	WHEEL_DIAMETER_START	20
char *WheelDiameter[] = {
	"20",
	"19",
	"18",
	"17",
	"16",
	"15",
	"14",
	"13",
	"12",
	"11",
	"10",
	NULL
};

#define	NUMLEN				10
#define	ZERO_DIFF_RATIO		(0.0001F)


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hGearingScreens, hTireSizeScreen;
//static int KMCDialogSemaphone = 0;
//static int clickedOnThisScreen;

static LISTWHEEL_Handle hTreadWidthListWheel, hAspectRatioWheel, hWheelDiameterWheel;
static LISTWHEEL_Handle hDiffWheel1, hDiffWheel2, hDiffWheel3;
BUTTON_Handle hGearButton[8], hTireDimensionButton, hTireCircumferenceButton;
static char buttonsCreated = false;
static void *pEntryKeyboard = 0;
char tireCircumference[NUMLEN+1];
static int gearingScreenSuspended = false;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetGearingWindowHandle(void)
{
	return hGearingScreens;
}

void SetGearingWindowHandle(WM_HWIN hWin)
{
	hGearingScreens = hWin;
}

void GearingScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	int Sel;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			GearingIgnoreNextKeyRelease();
			break;

		case WM_DELETE:
			buttonsCreated = false;
			break;

		case KB_CANCEL_MSG:
			if (pMsg->Data.p == pEntryKeyboard)
				TCKillKeyboard();
			break;

		case KB_SAVE_AND_EXIT_MSG:
			if (pMsg->Data.p == pEntryKeyboard)
				TCKeyboardSaveButton();
			break;

		case WM_PAINT:
			if ((GetScreenType() == GEARING_SCREENS_TYPE) && (!gearingScreenSuspended))
			{
				// Call paint method of center screen
				GearingScreenList[GetLastScreen(GEARING_SCREENS_TYPE)].paint(hGearingScreens);
				DrawSignalStrength();
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_SEL_CHANGED:
					if (abs(LISTWHEEL_GetVelocity(pMsg->hWinSrc)) <= 1)
					{
						switch(Id)
						{
						case DIFF_WHEEL_1:
							if ((vehicleData.diffRatio = ((float)(((Sel = LISTWHEEL_GetPos(hDiffWheel1)) * 100) + (LISTWHEEL_GetPos(hDiffWheel2) * 10) + LISTWHEEL_GetPos(hDiffWheel3)) / 100.0) + .00001) < ZERO_DIFF_RATIO)
								vehicleData.diffRatio = 0.0;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case DIFF_WHEEL_2:
							if ((vehicleData.diffRatio = ((float)((LISTWHEEL_GetPos(hDiffWheel1) * 100) + ((Sel = LISTWHEEL_GetPos(hDiffWheel2)) * 10) + LISTWHEEL_GetPos(hDiffWheel3)) / 100.0) + .00001) < ZERO_DIFF_RATIO)
								vehicleData.diffRatio = 0.0;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case DIFF_WHEEL_3:
							if ((vehicleData.diffRatio = ((float)((LISTWHEEL_GetPos(hDiffWheel1) * 100) + (LISTWHEEL_GetPos(hDiffWheel2) * 10) + (Sel = LISTWHEEL_GetPos(hDiffWheel3))) / 100.0) + .00001) < ZERO_DIFF_RATIO)
								vehicleData.diffRatio = 0.0;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						}
					}
					break;

				case WM_NOTIFICATION_CLICKED:
//					clickedOnThisScreen = true;
					switch (Id)
					{
						//case TREAD_WIDTH_WHEEL:
						//case ASPECT_RATIO_WHEEL:
						//case WHEEL_DIAMETER_WHEEL:
						case DIFF_WHEEL_1:
						case DIFF_WHEEL_2:
						case DIFF_WHEEL_3:
							pressingbutton_semaphore = 1;
							break;

						default:
							WM_DefaultProc(pMsg);
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;

					if (Id == TIRE_CIRCUMFERENCE_BUTTON)
					{
						DisplayTireCircumference();
					}
					else if (Id == TIRE_DIMENSIONS_BUTTON)
					{
						DisplayTireSize();
					}

					else if (buttonsCreated)
						switch (Id)
						{
						case FIRST_BUTTON:
						case SECOND_BUTTON:
						case THIRD_BUTTON:
						case FORTH_BUTTON:
						case FIFTH_BUTTON:
						case SIXTH_BUTTON:
						case SEVENTH_BUTTON:
						case EIGHTH_BUTTON:
							DisplayNewScreenList(GEAR_RATIOS, ButtonToScreenID(Id), hGearingScreens);
							break;
						}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

int ButtonToScreenID(int Id)
 {
	 switch (Id)
	 {
		case FIRST_BUTTON:		return FIRST_GEAR_SCREEN;
		case SECOND_BUTTON:		return SECOND_GEAR_SCREEN;
		case THIRD_BUTTON:		return THIRD_GEAR_SCREEN;
		case FORTH_BUTTON:		return FOURTH_GEAR_SCREEN;
		case FIFTH_BUTTON:		return FIFTH_GEAR_SCREEN;
		case SIXTH_BUTTON:		return SIXTH_GEAR_SCREEN;
		case SEVENTH_BUTTON:	return SEVENTH_GEAR_SCREEN;
		case EIGHTH_BUTTON:		return EIGHTH_GEAR_SCREEN;
		default:				return 0;
	 }
 }

WM_HWIN GetTireSizeScreenWindowHandle(void)
{
	return hTireSizeScreen;
}


void DisplayTireCircumference(void)
{
	SetScreenScrollingHOnly();

	gearingScreenSuspended = true;

	SetScreenType(TIRE_DIMENSIONS_SCREEN_TYPE);
	SetLastScreen(TIRE_DIMENSIONS_SCREEN_TYPE, TIRECIRCUMFERENCE_SCREEN);

	if (pEntryKeyboard)
		DestroyKeyboard(pEntryKeyboard);

	GetTireCircumferenceString(false, tireCircumference);

	pEntryKeyboard = CreateKeyboard("Enter Tire Circumference", tireCircumference, NUMERIC, true, NUMLEN, 0, hGearingScreens, false);
}

//
// Get the current tire circumference in the current units.
// If withUnits is true display the text of the units aswell
// Put the string at the pointer provided
//
void GetTireCircumferenceString(char withUnits, char *scratch)
{
	float circumference;
	char *pUnits;

	if (sysData.units == METRIC)
	{
		circumference = vehicleData.tireCircumference * INCHES_TO_CM;
		pUnits = " cm";
	}
	else
	{
		circumference = vehicleData.tireCircumference;
		pUnits = " in";
	}

	sprintf(scratch, "%.2f", circumference);
	trim(scratch);

	if (withUnits)
		strcat(scratch, pUnits);
}

void TCKeyboardSaveButton(void)
{
	TCKillKeyboard();
	EnsurePositive(tireCircumference);
	vehicleData.tireCircumference = atof(tireCircumference);
	if (sysData.units == METRIC)
		vehicleData.tireCircumference *= CM_TO_INCHES;		// Store circumference natively as inches

	// Range check circumference
	if (vehicleData.tireCircumference > 199.0)
		vehicleData.tireCircumference = 199.0;

	vehicleData.userEnteredTireDimensions = false;
}

void TCKillKeyboard(void)
{
	if (pEntryKeyboard)		// Should alwaye be there - but just in case!
	{
		DestroyKeyboard(pEntryKeyboard);
		pEntryKeyboard = 0;
	}
	ResumeScreenScrolling();

	gearingScreenSuspended = false;
	SetScreenType(GEARING_SCREENS_TYPE);
	WM_InvalidateWindow(hGearingScreens);
}

void DisplayTireSize(void)
{
	WM_DeleteWindow(hGearingScreens);
	SetScreenScrollingHOnly();
	SetScreenType(TIRE_DIMENSIONS_SCREEN_TYPE);
	SetLastScreen(TIRE_DIMENSIONS_SCREEN_TYPE, TIRESIZE_SCREEN);

	if ((hTireSizeScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, TireSizeScreenCallback, 0)) <= 0)
		SystemError();

	DisplayTripleListScreen(hTireSizeScreen,
		TreadWidth, TREAD_WIDTH_WHEEL, &hTreadWidthListWheel, (TREAD_WIDTH_START - vehicleData.treadWidth) / 10,
		AspectRatio, ASPECT_RATIO_WHEEL, &hAspectRatioWheel, (ASPECT_RATIO_START - vehicleData.aspectRatio) / 5,
		WheelDiameter, WHEEL_DIAMETER_WHEEL, &hWheelDiameterWheel, WHEEL_DIAMETER_START - vehicleData.wheelDiameter,
		"Select Tire Size",
		0, 0);

	hScreenTimer = WM_CreateTimer(hTireSizeScreen, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

static void TireSizeScreenCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id, Sel;

	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
			WM_InvalidateRect(pMsg->hWin, &BannerRect);
		}
		break;

	case WM_PAINT:
		TireSizePaint(hTireSizeScreen);
		break;

	case WM_NOTIFY_PARENT:
		Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
		NCode = pMsg->Data.v;                 // Notification code
		switch(NCode)
		{
		case WM_NOTIFICATION_SEL_CHANGED:
			if (abs(LISTWHEEL_GetVelocity(pMsg->hWinSrc)) <= 1)
			{
				switch(Id)
				{
				case TREAD_WIDTH_WHEEL:
					vehicleData.treadWidth = TREAD_WIDTH_START - ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) * 10);
					LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
					UpdateTireCircumference();
					break;
				case ASPECT_RATIO_WHEEL:
					vehicleData.aspectRatio = ASPECT_RATIO_START - ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) * 5);
					LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
					UpdateTireCircumference();
					break;
				case WHEEL_DIAMETER_WHEEL:
					vehicleData.wheelDiameter = WHEEL_DIAMETER_START - (Sel = LISTWHEEL_GetPos(pMsg->hWinSrc));
					LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
					UpdateTireCircumference();
					break;
				}
			}
			break;

		default:
			WM_DefaultProc(pMsg);
		};
		break;

	default:
		WM_DefaultProc(pMsg);
	};
}

void UpdateTireCircumference(void)
{
	vehicleData.userEnteredTireDimensions = true;
	vehicleData.tireCircumference = ((vehicleData.treadWidth * vehicleData.aspectRatio * MILLIMETERS_TO_INCHES * 2 / 100) + vehicleData.wheelDiameter) * PI;
}


void DisplayTireSizeSelection(WM_HWIN hParent)
{
	BUTTON_SetDefaultFont(&GUI_Font24B_ASCII);

	hTireDimensionButton = BUTTON_CreateEx(BUTTON_LEFT, 174, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TIRE_DIMENSIONS_BUTTON);
	BUTTON_SetBitmap(hTireDimensionButton, BUTTON_BI_UNPRESSED, &bmGreenButton);
	BUTTON_SetText(hTireDimensionButton, "Tire Size");
	BUTTON_SetTextColor(hTireDimensionButton, BUTTON_BI_UNPRESSED, GUI_BLACK);

	hTireCircumferenceButton = BUTTON_CreateEx(BUTTON_LEFT, 218, BUTTON_WIDTH, BUTTON_HEIGHT, hParent, WM_CF_SHOW, 0, TIRE_CIRCUMFERENCE_BUTTON);
	BUTTON_SetBitmap(hTireCircumferenceButton, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetText(hTireCircumferenceButton, "Tire Circumference");
	BUTTON_SetTextColor(hTireCircumferenceButton, BUTTON_BI_UNPRESSED, GUI_WHITE);
}

void DisplayDiffRatio(WM_HWIN hParent)
{
	int hundreds, tens, ones, temp;

	temp = (int)((vehicleData.diffRatio * 100.0) + 0.5);
	hundreds = temp / 100;
	tens = (temp / 10) % 10;
	ones = temp % 10;
	DisplayTripleListScreen(hParent,
		ZeroThroughNine, DIFF_WHEEL_1, &hDiffWheel1, hundreds,
		ZeroThroughNine, DIFF_WHEEL_2, &hDiffWheel2, tens,
		ZeroThroughNine, DIFF_WHEEL_3, &hDiffWheel3, ones,
		"Select Diff Ratio",
		0, 0);
}

void DisplayTransRatio(WM_HWIN hParent)
{
	BUTTON_SetDefaultFont(&GUI_Font24B_ASCII);
	BUTTON_SetDefaultBkColor(GUI_BLACK, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_GREEN, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_RED, BUTTON_BI_PRESSED);

	hGearButton[0] = BUTTON_CreateEx(30, 50, 119, 45, hParent, WM_CF_SHOW, 0, FIRST_BUTTON);
	BUTTON_SetBitmap(hGearButton[0], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(0);

	hGearButton[1] = BUTTON_CreateEx(180, 50, 119, 45, hParent, WM_CF_SHOW, 0, SECOND_BUTTON);
	BUTTON_SetBitmap(hGearButton[1], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(1);

	hGearButton[2] = BUTTON_CreateEx(330, 50, 119, 45, hParent, WM_CF_SHOW, 0, THIRD_BUTTON);
	BUTTON_SetBitmap(hGearButton[2], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(2);

	hGearButton[3] = BUTTON_CreateEx(30, 120, 119, 45, hParent, WM_CF_SHOW, 0, FORTH_BUTTON);
	BUTTON_SetBitmap(hGearButton[3], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(3);

	hGearButton[4] = BUTTON_CreateEx(180, 120, 119, 45, hParent, WM_CF_SHOW, 0, FIFTH_BUTTON);
	BUTTON_SetBitmap(hGearButton[4], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(4);

	hGearButton[5] = BUTTON_CreateEx(330, 120, 119, 45, hParent, WM_CF_SHOW, 0, SIXTH_BUTTON);
	BUTTON_SetBitmap(hGearButton[5], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(5);

	hGearButton[6] = BUTTON_CreateEx(30, 190, 119, 45, hParent, WM_CF_SHOW, 0, SEVENTH_BUTTON);
	BUTTON_SetBitmap(hGearButton[6], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(6);

	hGearButton[7] = BUTTON_CreateEx(180, 190, 119, 45, hParent, WM_CF_SHOW, 0, EIGHTH_BUTTON);
	BUTTON_SetBitmap(hGearButton[7], BUTTON_BI_UNPRESSED, &bmGearRatioButton);
	SetGearButtonText(7);

	buttonsCreated = true;
}

void SetGearButtonText(int index)
{
	char scratch[20];

	sprintf(scratch, "%f", vehicleData.gearRatios[index]);
	trim(scratch);
	BUTTON_SetText(hGearButton[index], scratch);
}


void DisplayHelp(WM_HWIN hParent)
{
}


void TireSizePaint(WM_HWIN hParent)
{
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_1, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_2, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_3, 49);
	GUI_DrawBitmap(&bmTireSize, 60, 0);

	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_SetColor(GUI_GREEN);
	GUI_DispStringHCenterAt("ex = 205 / 50 x 15", TRIPLE_SPINNER_TEXT_START + (BUTTON_WIDTH / 2), 165);

	// Draw characters "/" and "x" between spinners
	GUI_SetColor(GUI_LIGHTBLUE);
	GUI_DispStringHCenterAt("/", TRIPLE_SPINNER_START_1 + 73, 140);
	GUI_DispStringHCenterAt("x", TRIPLE_SPINNER_START_2 + 74, 138);

	GUI_SetFont(&GUI_Font16_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_DispStringHCenterAt("Width / Aspect Ratio x Rim Size", TRIPLE_SPINNER_TEXT_START + (BUTTON_WIDTH / 2), 190);

	GUI_DrawBitmap(GetNavigationArrow(TIRESIZE_SCREEN), 0, 229);
	
	DrawSignalStrength();
}

void DiffRatioPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_1, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_2, 49);
	GUI_DrawBitmap(&bmNarrowSpinner, TRIPLE_SPINNER_START_3, 49);
#if 1
	GUI_DrawBitmap(&bmDiffRatio, 60, 0);
#else
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringHCenterAt("Diff Ratio", XSIZE_PHYS/2, 4);
#endif
	// Draw "." between spinners
	GUI_SetFont(&GUI_FontD32);
	GUI_SetColor(GUI_LIGHTBLUE);
	GUI_DispStringHCenterAt(".", TRIPLE_SPINNER_START_2 - 5, 125);

	GUI_DrawBitmap(GetNavigationArrow(DIFFRATIO_SCREEN), 0, 229);
}

void TransRatioPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
#if 1
	GUI_DrawBitmap(&bmTransRatio, 60, 0);
#else
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_DispStringHCenterAt("Trans Ratio", XSIZE_PHYS/2, 4);
#endif


	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font20B_ASCII);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_DispStringHCenterAt("1st Gear",  90, 95);
	GUI_DispStringHCenterAt("2nd Gear", 240, 95);
	GUI_DispStringHCenterAt("3rd Gear", 390, 95);
	GUI_DispStringHCenterAt("4th Gear",  90, 166);
	GUI_DispStringHCenterAt("5th Gear", 240, 166);
	GUI_DispStringHCenterAt("6th Gear", 390, 166);
	GUI_DispStringHCenterAt("7th Gear",  90, 236);
	GUI_DispStringHCenterAt("8th Gear", 240, 236);

	GUI_SetColor(GUI_YELLOW);
	GUI_SetFont(&GUI_Font20_ASCII);
	GUI_DispStringHCenterAt("Touch to Enter\nTransmission\nRatios", 390, 190);

	GUI_DrawBitmap(GetNavigationArrow(TRANSRATIO_SCREEN), 0, 229);
}

#define	LINE_HEIGHT		15
#define	PARA_1_START	70
#define	PARA_2_START	205
#define	LEFT_MARGIN		40

void HelpPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmHelp, 60, 0);

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);

	GUI_SetFont(&GUI_Font16B_ASCII);
	GUI_DispStringAt("GEAR RATIOS AND TIRE SIZES", LEFT_MARGIN, 55);

	GUI_SetFont(&GUI_Font16_ASCII);
	GUI_DispStringAt("In order to determine which gear is selected in the vehicle the",			LEFT_MARGIN, PARA_1_START);
	GUI_DispStringAt("complete gearing information about the vehicle must be known.  This",		LEFT_MARGIN, PARA_1_START + LINE_HEIGHT);
	GUI_DispStringAt("includes the gear ratios as well as the differential ratio and the",		LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*2));
	GUI_DispStringAt("tire rolling circumference.  However, if you do not wish to use this",	LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*3));
	GUI_DispStringAt("feature it can be disabled by entering a differential ratio of 0.00.",	LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*4));
	GUI_DispStringAt("If this value is entered you will be asked to confirm that you do not",	LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*5));
	GUI_DispStringAt("wish to calculate gear ratios.  If accepted then the calculated gear",	LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*6));
	GUI_DispStringAt("position will not be displayed.",											LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*7));

	GUI_DrawBitmap(GetNavigationArrow(HELP_SCREEN), 0, 229);
}

void TireSizeSelectionPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmTireSize, 60, 0);

	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TM_TRANS);

	GUI_SetFont(&GUI_Font16B_ASCII);
	GUI_DispStringAt("TIRE SIZE", LEFT_MARGIN, 55);
	GUI_SetFont(&GUI_Font16_ASCII);
	GUI_DispStringAt("In order to determine which gear is selected in the vehicle, the",		LEFT_MARGIN, PARA_1_START);
	GUI_DispStringAt("circumference. (distance around) the tire must be measured.  This is",	LEFT_MARGIN, PARA_1_START + LINE_HEIGHT);
	GUI_DispStringAt("the distance the vehicle travels for every revolution of the tire.",		LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*2));
	GUI_DispStringAt("You may enter the number on the sidewall of the tire using the",			LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*3));
	GUI_DispStringAt("calculator or you can measure the distance around the tire with a tape",	LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*4));
	GUI_DispStringAt("measure.  Most tire circumferences are in the range of 50in (127cm)",		LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*5));
	GUI_DispStringAt("to 150in (381cm).",														LEFT_MARGIN, PARA_1_START + (LINE_HEIGHT*6));

	GUI_DrawBitmap(GetNavigationArrow(TIRESIZE_SELECTION_SCREEN), 0, 229);
}

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void GearingIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}


/*************************** End of file ****************************/

