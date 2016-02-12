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
#include "commondefs.h"
#include "DIALOG.h"
#include "ConfigScreens.h"
#include "GeneralConfig.h"
#include "GaugeScreen.h"
#include "FlashReadWrite.h"
#include "GMeterScreen.h"
#include "RunningGauges.h"
#include "TrackSettingsScreens.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "TachSetupScreens.h"
#include "InputScreens.h"
#include "SystemSettingsScreens.h"
#include "Utilities.h"
#include "SFScreens.h"
#include "RaceSetupScreens.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
char alteredTrackData = false;

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmStartFinish;
extern GUI_CONST_STORAGE GUI_BITMAP bmStartFinishText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrackSettingsText;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrackTypeText;
extern GUI_CONST_STORAGE GUI_BITMAP bmLaptimeHold;
extern GUI_CONST_STORAGE GUI_BITMAP bmLapHoldTimeButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTrackType;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenCheck;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmResetPredictiveLap;
extern GUI_CONST_STORAGE GUI_BITMAP bmPredictiveLapText;


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DisplayStartFinish(WM_HWIN);
void DisplayTrackType(WM_HWIN);
void DisplayHoldTime(WM_HWIN);
void DisplayResetPL(WM_HWIN);

void ResetPLPaint(WM_HWIN);
void HoldTimePaint(WM_HWIN);
void StartFinishPaint(WM_HWIN);
void TrackTypePaint(WM_HWIN);

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);

static void TrackSettingMainScreenCallback(WM_MESSAGE * pMsg);
void SetLatButtonText(void);
void SetLongButtonText(void);
void SetHeadButtonText(void);
void SetFinishLatButtonText(void);
void SetFinishLongButtonText(void);
void SetFinishHeadButtonText(void);
int okToResetSF(void);
int TSButtonToScreenID(int Id);
char OpenCourse(void);
char NeedToEditStartFinish(void);
char NeedToDisplayFinish(void);
enum TRACK_TYPES IndexToCourseType(char type);
char CourseTypeToIndex(enum TRACK_TYPES type);



/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST TrackSettingsScreenList[NUM_TRACK_SETTINGS_SCREENS+1] = {
	{ TRACKTYPE_SELECTION, DisplayTrackType, TrackTypePaint },
	{ HOLDTIME_SELECTION, DisplayHoldTime, HoldTimePaint },
	{ STARTFINISH_SELECTION, DisplayStartFinish, StartFinishPaint },
	{ RESETPL_SELECTION, DisplayResetPL, ResetPLPaint },
	{ -1, NULL, NULL }
};

char *pTrackTypes[] = {
	"Road Race - Closed",
	"Oval - Closed",
	"Auto-X/Rally - Open",
//	"Drag 1/8 mile",
//	"Drag 1000 feet",
	"Drag Strip - Open",
	"Hill Climb - Open",
	"Drive - Open",
	NULL
};

char *LapHoldTimes[] = {		// to go to new TrackSettingsScreen
	"0 sec",   "10 sec",  "20 sec",  "30 sec",  "40 sec",  "50 sec",  "60 sec",
	"70 sec",  "80 sec",  "90 sec",  "100 sec", "110 sec", "120 sec",
	"130 sec", "140 sec", "150 sec", "160 sec", "170 sec", "180 sec",
	NULL
};

//
// Widget IDs
//
#define	TRACKTYPE_LIST				GUI_ID_LISTWHEEL0
#define	LAPHOLD_LIST				GUI_ID_LISTWHEEL1

#define	TRACKTYPE_BUTTON			GUI_ID_USER + 1
#define	LAPHOLD_BUTTON				GUI_ID_USER + 2
#define	START_FINISH_BUTTON			GUI_ID_USER + 3
#define	CLEAR_SF_BUTTON				GUI_ID_USER + 4
#define	START_LATITUDE_BUTTON		GUI_ID_USER + 5
#define	START_LONGITUDE_BUTTON		GUI_ID_USER + 6
#define	START_HEADING_BUTTON		GUI_ID_USER + 7
#define	RESETPL_BUTTON				GUI_ID_USER + 8
#define	RESET_FASTEST_LAP			GUI_ID_USER + 9
#define	FINISH_LATITUDE_BUTTON		GUI_ID_USER + 10
#define	FINISH_LONGITUDE_BUTTON		GUI_ID_USER + 11
#define	FINISH_HEADING_BUTTON		GUI_ID_USER + 12

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hTrackSettingsScreens;
static int clickedOnVSScreen;
static int clickedOnVSMScreen;
static LISTWHEEL_Handle hTrackTypeWheel;
static LISTWHEEL_Handle hHoldTimeListWheel;
BUTTON_Handle hStartLatButton, hStartLongButton, hStartHeadButton, hFastestLap;
BUTTON_Handle hFinishLatButton = 0, hFinishLongButton = 0, hFinishHeadButton = 0;

WM_HWIN hTrackSettingMainScreen;


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/


///////////////////////////////////////////////////////////////////////////
//
// Special intermediate screen(s)
//
void Display_TrackSettingMainScreen(WM_HWIN hPrev)
{
BUTTON_Handle hButton;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();

	SetScreenType(TRACK_SETTING_SCREEN_TYPE);

	if ((hTrackSettingMainScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, TrackSettingMainScreenCallback, 0)) <= 0)
		SystemError();

	hButton = BUTTON_CreateEx(41, 38, 178, 115, hTrackSettingMainScreen, WM_CF_SHOW, 0, TRACKTYPE_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTrackType);

	hButton = BUTTON_CreateEx(261, 38, 178, 115, hTrackSettingMainScreen, WM_CF_SHOW, 0, LAPHOLD_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmLapHoldTimeButton);

	hButton = BUTTON_CreateEx(41, 156, 178, 115, hTrackSettingMainScreen, WM_CF_SHOW, 0, START_FINISH_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmStartFinish);

	hButton = BUTTON_CreateEx(261, 156, 178, 115, hTrackSettingMainScreen, WM_CF_SHOW, 0, RESETPL_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmResetPredictiveLap);

	hScreenTimer = WM_CreateTimer(hTrackSettingMainScreen, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

static void TrackSettingMainScreenCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(hTrackSettingMainScreen, &BannerRect);
			}
			break;

	case WM_PAINT:
		GUI_DrawBitmap(&bmBackground, 0, 0);
		GUI_DrawBitmap(&bmTrackSettingsText, 60, 1);
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
		DrawSignalStrength();
		break;

	case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
      NCode = pMsg->Data.v;                 // Notification code
	   switch(NCode)
		{
			case WM_NOTIFICATION_CLICKED:
				clickedOnVSMScreen = true;
				break;

			case WM_NOTIFICATION_RELEASED:
				if (clickedOnVSMScreen)
					switch (Id)
					{
						case TRACKTYPE_BUTTON:
							DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, TRACKTYPE_SELECTION, hTrackSettingMainScreen);
							break;

						case LAPHOLD_BUTTON:
							DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, HOLDTIME_SELECTION, hTrackSettingMainScreen);
							break;

						case START_FINISH_BUTTON:
							DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, STARTFINISH_SELECTION, hTrackSettingMainScreen);
							break;

						case RESETPL_BUTTON:
							DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, RESETPL_SELECTION, hTrackSettingMainScreen);
							break;
					}
				break;
	  };
	  break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

WM_HWIN GetTrackSettingMainWindowHandle(void)
{
	return hTrackSettingMainScreen;
}

void SetTrackSettingMainWindowHandle(WM_HWIN hWin)
{
	hTrackSettingMainScreen = hWin;
}


WM_HWIN GetTrackSettingsWindowHandle(void)
{
	return hTrackSettingsScreens;
}

void SetTrackSettingsWindowHandle(WM_HWIN hWin)
{
	hTrackSettingsScreens = hWin;
}

void TrackSettingsScreensCallback(WM_MESSAGE * pMsg)
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
			TrackSettingsIgnoreNextKeyRelease();
			break;

		case WM_DELETE:
			DeleteSFScreensKeyboards();
			hFinishLatButton = 0;
			hFinishLongButton = 0;
			hFinishHeadButton = 0;
			break;

		case WM_PAINT:
			if (GetScreenType() == TRACK_SETTINGS_SCREENS_TYPE)
			{
				GUI_Clear();		// Clear the background here (and not in the 'constructor') to reduce flicker
				GUI_SetColor(GUI_WHITE);
				GUI_SetFont(&GUI_Font32_ASCII);

				// Call paint method of center screen
				TrackSettingsScreenList[GetLastScreen(TRACK_SETTINGS_SCREENS_TYPE)].paint(hTrackSettingsScreens);
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
						case LAPHOLD_LIST:
							{
								unsigned char temp;

								temp = (Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) * 10;
								if (temp != trackData.laptimeHold)
								{
									trackData.laptimeHold = temp;
									LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
									alteredTrackData = true;
								}
							}
							break;

						case TRACKTYPE_LIST:
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel = LISTWHEEL_GetPos(pMsg->hWinSrc));
							if (IndexToCourseType(Sel) != trackData.courseType)
							{
								sysData.selectedSessionType = TrackTypeToSessionType((enum TRACK_TYPES)(trackData.courseType = IndexToCourseType(Sel)));
								alteredTrackData = true;
							}
							break;
						}
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					clickedOnVSScreen = true;
					switch (Id)
					{
						case TRACKTYPE_LIST:
						case LAPHOLD_LIST:
							pressingbutton_semaphore = 1;
							break;

						default:
							WM_DefaultProc(pMsg);
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
					if (!clickedOnVSScreen)
						break;

					switch (Id)
					{
						case RESET_FASTEST_LAP:
							trackData.bestLapEver = 0;
							WM_Invalidate(hTrackSettingsScreens);
							alteredTrackData = true;
							break;

						case CLEAR_SF_BUTTON:
							if (okToResetSF() == GUI_ID_YES)
							{
								trackData.startLineLat = 0.0;
								trackData.startLineLong = 0.0;
								trackData.startLineHeading = 0.0;
								trackData.finishLineLat = 0.0;
								trackData.finishLineLong = 0.0;
								trackData.finishLineHeading = 0.0;
								SetHeadButtonText();
								SetLongButtonText();
								SetLatButtonText();
								SetFinishHeadButtonText();
								SetFinishLongButtonText();
								SetFinishLatButtonText();
								alteredTrackData = true;
							}
							break;

						case START_LATITUDE_BUTTON:
						case START_LONGITUDE_BUTTON:
						case START_HEADING_BUTTON:
						case FINISH_LATITUDE_BUTTON:
						case FINISH_LONGITUDE_BUTTON:
						case FINISH_HEADING_BUTTON:
							if (NeedToEditStartFinish())
								DisplaySFKeyboard(TSButtonToScreenID(Id), hTrackSettingsScreens);
							break;
					}
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

int TSButtonToScreenID(int Id)
{
	switch (Id)
	{
	default:
	case START_LATITUDE_BUTTON:
		if (OpenCourse())
			return START_LATITUDE_SCREEN;
		else return LATITUDE_SCREEN;
	case START_LONGITUDE_BUTTON:
		if (OpenCourse())
			return START_LONGITUDE_SCREEN;
		else return LONGITUDE_SCREEN;
	case START_HEADING_BUTTON:
		if (OpenCourse())
			return START_HEADING_SCREEN;
		else return HEADING_SCREEN;
	case FINISH_LATITUDE_BUTTON:	return FINISH_LATITUDE_SCREEN;
	case FINISH_LONGITUDE_BUTTON:	return FINISH_LONGITUDE_SCREEN;
	case FINISH_HEADING_BUTTON:		return FINISH_HEADING_SCREEN;
	}
}


void DisplayTrackType(WM_HWIN hParent)
{
	DisplayStdListScreen(hParent, pTrackTypes, TRACKTYPE_LIST, &hTrackTypeWheel, CourseTypeToIndex((enum TRACK_TYPES)trackData.courseType), TEXT_ONLY, "Select Race Type");
}

void DisplayHoldTime(WM_HWIN hParent)
{
	DisplayStdListScreen(hParent, LapHoldTimes, LAPHOLD_LIST, &hHoldTimeListWheel, trackData.laptimeHold / 10, TEXT_ONLY, "On-Screen Time");
}

void TrackTypePaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmTrackTypeText, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(TRACKTYPE_SELECTION), 0, 229);
}



void HoldTimePaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmLaptimeHold, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(HOLDTIME_SELECTION), 0, 229);
}

void SetLatButtonText(void)
{
	char scratch[30];

	sprintf(scratch, "%f", trackData.startLineLat);
	BUTTON_SetText(hStartLatButton, scratch);
}
void SetLongButtonText(void)
{
	char scratch[30];

	sprintf(scratch, "%f", trackData.startLineLong);
	BUTTON_SetText(hStartLongButton, scratch);
}
void SetHeadButtonText(void)
{
	char scratch[30];

	sprintf(scratch, "%.0f", trackData.startLineHeading);
	BUTTON_SetText(hStartHeadButton, scratch);
}

void SetFinishLatButtonText(void)
{
	char scratch[30];

	if (hFinishLatButton)
	{
		sprintf(scratch, "%f", trackData.finishLineLat);
		BUTTON_SetText(hFinishLatButton, scratch);
	}
}
void SetFinishLongButtonText(void)
{
	char scratch[30];

	if (hFinishLongButton)
	{
		sprintf(scratch, "%f", trackData.finishLineLong);
		BUTTON_SetText(hFinishLongButton, scratch);
	}
}
void SetFinishHeadButtonText(void)
{
	char scratch[30];

	if (hFinishHeadButton)
	{
		sprintf(scratch, "%.0f", trackData.finishLineHeading);
		BUTTON_SetText(hFinishHeadButton, scratch);
	}
}

void DisplayStartFinish(WM_HWIN hParent)
{
	BUTTON_Handle hButton;

	if (NeedToDisplayFinish())
	{
		hButton = BUTTON_CreateEx(139, 53, 202, 39, hParent, WM_CF_SHOW, 0, CLEAR_SF_BUTTON);
		BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRedButton);
		BUTTON_SetFont(hButton, &GUI_Font24B_ASCII);
		BUTTON_SetText(hButton, "Clear Start/Finish");

		BUTTON_SetDefaultBkColor(GUI_WHITE, BUTTON_BI_UNPRESSED);
		BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_BI_UNPRESSED);
		BUTTON_SetDefaultFont(&GUI_Font32B_ASCII);
		hStartLatButton = BUTTON_CreateEx(12, 140, 156, 36, hParent, WM_CF_SHOW, 0, START_LATITUDE_BUTTON);
		hStartLongButton = BUTTON_CreateEx(192, 140, 156, 36, hParent, WM_CF_SHOW, 0, START_LONGITUDE_BUTTON);
		hStartHeadButton = BUTTON_CreateEx(377, 140, 86, 36, hParent, WM_CF_SHOW, 0, START_HEADING_BUTTON);
		hFinishLatButton = BUTTON_CreateEx(12, 187, 156, 36, hParent, WM_CF_SHOW, 0, FINISH_LATITUDE_BUTTON);
		hFinishLongButton = BUTTON_CreateEx(192, 187, 156, 36, hParent, WM_CF_SHOW, 0, FINISH_LONGITUDE_BUTTON);
		hFinishHeadButton = BUTTON_CreateEx(377, 187, 86, 36, hParent, WM_CF_SHOW, 0, FINISH_HEADING_BUTTON);
	}
	else
	{
		hButton = BUTTON_CreateEx(139, 55, 202, 39, hParent, WM_CF_SHOW, 0, CLEAR_SF_BUTTON);
		BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRedButton);
		BUTTON_SetFont(hButton, &GUI_Font24B_ASCII);
		if (OpenCourse())
			BUTTON_SetText(hButton, "Clear Start Line");
		else BUTTON_SetText(hButton, "Clear Start/Finish");

		BUTTON_SetDefaultBkColor(GUI_WHITE, BUTTON_BI_UNPRESSED);
		BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_BI_UNPRESSED);
		BUTTON_SetDefaultFont(&GUI_Font32B_ASCII);
		hStartLatButton = BUTTON_CreateEx(12, 152, 156, 36, hParent, WM_CF_SHOW, 0, START_LATITUDE_BUTTON);
		hStartLongButton = BUTTON_CreateEx(192, 152, 156, 36, hParent, WM_CF_SHOW, 0, START_LONGITUDE_BUTTON);
		hStartHeadButton = BUTTON_CreateEx(377, 152, 86, 36, hParent, WM_CF_SHOW, 0, START_HEADING_BUTTON);

		hFinishLatButton = 0;
		hFinishLongButton = 0;
		hFinishHeadButton = 0;
	}
}

void StartFinishPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmStartFinishText, 60, 0);

	GUI_SetColor(GUI_BLACK);
	if (NeedToDisplayFinish())
	{
		GUI_FillRect(0, 43, XSIZE_PHYS, 267);

		GUI_SetPenSize(3);
		GUI_SetColor(GUI_GRAY);
		GUI_DrawLine(0, 100, XSIZE_PHYS, 100);

		GUI_SetColor(GUI_WHITE);
		GUI_SetFont(&GUI_Font32B_ASCII);
		GUI_SetTextMode(GUI_TM_TRANS);
		if (NeedToEditStartFinish())
			GUI_DispStringHCenterAt("Enter Start/Finish", 180, 105);
		else GUI_DispStringHCenterAt("Start/Finish", 180, 105);
		GUI_DispStringHCenterAt("Heading", 420, 105);

		GUI_SetColor(GUI_GREEN);
		GUI_DispStringHCenterAt(sysData.track, XSIZE_PHYS/2, 230);

		SetFinishLatButtonText();
		SetFinishLongButtonText();
		SetFinishHeadButtonText();
	}
	else
	{
		GUI_FillRect(0, 45, XSIZE_PHYS, 240);

		GUI_SetPenSize(3);
		GUI_SetColor(GUI_GRAY);
		GUI_DrawLine(0, 105, XSIZE_PHYS, 105);

		GUI_SetColor(GUI_WHITE);
		GUI_SetFont(&GUI_Font32B_ASCII);
		GUI_SetTextMode(GUI_TM_TRANS);
		if (OpenCourse())
			GUI_DispStringHCenterAt("Enter Start Line", 180, 115);
		else GUI_DispStringHCenterAt("Enter Start/Finish", 180, 115);
		GUI_DispStringHCenterAt("Heading", 420, 115);

		GUI_SetColor(GUI_GREEN);
		GUI_DispStringHCenterAt(sysData.track, XSIZE_PHYS/2, 200);
	}

	SetLatButtonText();
	SetLongButtonText();
	SetHeadButtonText();

	GUI_DrawBitmap(GetNavigationArrow(STARTFINISH_SELECTION), 0, 229);
}


char OpenCourse(void)
{
	if ((trackData.courseType == ROAD_COURSE) || (trackData.courseType == OVAL))
		return false;
	else return true;
}

char NeedToEditStartFinish(void)
{
	if ((trackData.courseType == DRAG_1000) || (trackData.courseType == DRAG_1_8) || (trackData.courseType == DRAG))
		return false;
	else return true;
}

char NeedToDisplayFinish(void)
{
	if ((trackData.courseType == AUTOXRALLY) || (trackData.courseType == HILL_CLIMB) || (trackData.courseType == DRIVE_COURSE) ||
		(trackData.courseType == DRAG) || (trackData.courseType == DRAG_1000) || (trackData.courseType == DRAG_1_8) )
		return true;
	else return false;
}

void DisplayResetPL(WM_HWIN hParent)
{
	BUTTON_Handle hButton;

	hButton = BUTTON_CreateEx(139, 55, 202, 39, hParent, WM_CF_SHOW, 0, RESET_FASTEST_LAP);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRedButton);
	BUTTON_SetFont(hButton, &GUI_Font24B_ASCII);
	BUTTON_SetText(hButton, "Reset Time");

	BUTTON_SetDefaultBkColor(GUI_WHITE, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_BLACK, BUTTON_BI_UNPRESSED);
	BUTTON_SetDefaultFont(&GUI_Font32B_ASCII);
	hFastestLap = BUTTON_CreateEx(12, 152, 156, 36, hParent, WM_CF_SHOW, 0, 0);
}
void ResetPLPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackground, 0, 0);
	GUI_DrawBitmap(&bmPredictiveLapText, 60, 0);

	GUI_SetColor(GUI_BLACK);
	GUI_DrawRect(0, 45, XSIZE_PHYS, 240);
	GUI_FillRect(0, 45, XSIZE_PHYS, 240);

	GUI_SetPenSize(3);
	GUI_SetColor(GUI_GRAY);
	GUI_DrawLine(0, 105, XSIZE_PHYS, 105);

	GUI_SetColor(GUI_WHITE);
	GUI_SetFont(&GUI_Font24B_ASCII);
	GUI_SetTextMode(GUI_TM_TRANS);
	GUI_DispStringHCenterAt("Resetting will clear the fastest lap for this track", XSIZE_PHYS/2, 115);
	GUI_DispStringAt("is the current fastest lap", 190, 155);

	BUTTON_SetText(hFastestLap, formatLapTime(trackData.bestLapEver, true));

	GUI_DrawRoundedRect(10, 150, 170, 190, 5);
	GUI_FillRoundedRect(10, 150, 170, 190, 5);

	GUI_SetFont(&GUI_Font32B_ASCII);
	GUI_SetColor(GUI_GREEN);
	GUI_DispStringHCenterAt(sysData.track, XSIZE_PHYS/2, 200);

	GUI_DrawBitmap(GetNavigationArrow(STARTFINISH_SELECTION), 0, 229);
}

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void TrackSettingsIgnoreNextKeyRelease(void)
{
	clickedOnVSScreen = false;
}

static const GUI_WIDGET_CREATE_INFO ResetSFDialog[] = {
	{ FRAMEWIN_CreateIndirect, "WARNING", 0, 50, 20, 360, 220, 0, 0 },
	{ TEXT_CreateIndirect, "Resetting your current\nStart/Finish Coordinates!", 0, 0, 10, 360, 50, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Are you sure?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "YES", GUI_ID_YES, 60, 115, 100, 60 },
	{ BUTTON_CreateIndirect, "CANCEL", GUI_ID_CANCEL, 200, 115, 100, 60 }
};

//
// Display a modal dialog box to see if the user wants to reset the start finish coordinates
//
// Returns	true - proceed to reset
//				false - do not reset
//
int okToResetSF(void)
{
	// KMC TODO - if DriveMode screen uses WM_CF_MEMDEV_ON_REDRAW then the popup screen makes the
	// window background go brown.  But by allowing the paint to continue the brown does not show.
	// This might be "fixed" for all screens if they all use WM_CF_MEMDEV_ON_REDRAW.
	//if (GetCurrentScreen() != DRIVE_MODE_SCREEN)
	//	glbBlackoutFlag = true;			// Suspend gauge screen updates while the modal popup dialog box is diaplyed

	SetPopupDefaults(WARNING_POPUP);

	return GUI_ExecDialogBox(ResetSFDialog, GUI_COUNTOF(ResetSFDialog), &popupCallback, hTrackSettingsScreens, 10, 10);
}


char CourseTypeToIndex(enum TRACK_TYPES type)
{
	switch (type)
	{
	default:
	case ROAD_COURSE:	return 0;
	case OVAL:			return 1;
	case AUTOXRALLY:	return 2;
	case DRAG_1_8:
	case DRAG_1000:
	case DRAG:			return 3;
	case HILL_CLIMB:	return 4;
	case DRIVE_COURSE:	return 5;
	}
}

enum TRACK_TYPES IndexToCourseType(char type)
{
	switch (type)
	{
	default:
	case 0:	return ROAD_COURSE;
	case 1:	return OVAL;
	case 2:	return AUTOXRALLY;
	case 3: return DRAG;
	case 4:	return HILL_CLIMB;
	case 5:	return DRIVE_COURSE;
	}
}

/*************************** End of file ****************************/

