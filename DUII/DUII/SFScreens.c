//
//	SFScreens.c
//
//	Gathers user input for start/finish coordinates
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DataHandler.h"
#include "InputScreens.h"
#include "KeyboardAccess.h"
#include "SFScreens.h"
#include "Utilities.h"
#include "GeneralConfig.h"
#include "StandardList.h"
#include "RaceSetupScreens.h"
#include "FlashReadWrite.h"
#include "TrackSettingsScreens.h"
#include "DragAutoXScreens.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/
WM_HWIN hWaitingForCamera;

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmTapScreenAtSF;
extern GUI_CONST_STORAGE GUI_BITMAP bmGPSsignal;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmWaitingForCamera;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoXPresstoStage;
extern GUI_CONST_STORAGE GUI_BITMAP bmChooseGauges;
extern GUI_CONST_STORAGE GUI_BITMAP bmSweepTachButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmAnalogGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmDigitalGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmReadytoLaunchBurnout;
extern GUI_CONST_STORAGE GUI_BITMAP bmReadyToLaunchCorvette;
extern GUI_CONST_STORAGE GUI_BITMAP bmSweepTachButtonOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmAnalogGaugeButtonOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmDigitalGaugeButtonOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutocrossTouchToStage;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragTouchToStage;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutocrossStageBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutocrossTouchToStage;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragAnalogGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragDigitalGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragSweepTachButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoXAnalogGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoXDigitalGaugeButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmAutoXSweepTachButtonOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmDragStageBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneEighthOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneQuarterOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneThousandOn;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneEighthOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneQuarterOff;
extern GUI_CONST_STORAGE GUI_BITMAP bmOneThousandOff;


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void SFScreensSaveData(void);
static void TapAtSFScreenCallback(WM_MESSAGE * pMsg);
static void StageScreenCallback(WM_MESSAGE * pMsg);
static void WaitingForSatellitesCallback(WM_MESSAGE * pMsg);
static void WaitingForCameraCallback(WM_MESSAGE * pMsg);
char ValidateSFCoordinates(void);
int InvalidSFDataPopUp(void);
static void LaunchScreenCallback(WM_MESSAGE * pMsg);
int LaunchDetected(void);
void SetGaugeButtonGraphics(void);
void SetDistanceButtonGraphics(void);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/

// +/-2.6
#define	MAX_LONGITUDE_STRING	10
// +/-3.6
#define	MAX_LATITUDE_STRING		11
// +/-3.6
#define	MAX_HEADING_STRING		10

#define	START_BUTTON			GUI_ID_USER + 1
#define	CANCEL_BUTTON			GUI_ID_USER + 2
#define	RUN_WITHOUT_BUTTON		GUI_ID_USER + 3
#define	SWEEPTACH_BUTTON		GUI_ID_USER + 4
#define	ANALOGGAUGE_BUTTON		GUI_ID_USER + 5
#define	DIGITALGAUGE_BUTTON		GUI_ID_USER + 6
#define	STAGE_BUTTON			GUI_ID_USER + 7
#ifdef _WINDOWS
#define	LAUNCH_BUTTON			GUI_ID_USER + 8
#endif
#define	DRAG_1_8_MILE_BUTTON	GUI_ID_USER + 9
#define	DRAG_1000_FEET_BUTTON	GUI_ID_USER + 10
#define	DRAG_1_4_MILE_BUTTON	GUI_ID_USER + 11

#define	WAIT_FOR_SATELLITE_TIME		500
#define	WAIT_FOR_CAMERA_TIME		500
#define	TOTAL_WAIT_FOR_CAMERA_TIME	10000
#define	WAIT_FOR_LAUNCH_TIME		50

#define	LEFT	18
#define	TOP		10


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hSFScreens, hTapAtSFScreen, hWaitingForSatellites, hStageScreen, hLaunchScreen;
BUTTON_Handle hSweepTachButton, hAnalogGaugeButton, hDigitalGaugeButton, hStageButton;
BUTTON_Handle h1_8thMile, h1000ft, h1_4Mile;

#ifdef _WINDOWS
BUTTON_Handle hLaunchButton;
#endif
static void *pSFKeyboard = 0;
static WM_HTIMER hWaitForSatelliteTimer = 0;
static WM_HTIMER hWaitForCameraTimer = 0;
static WM_HTIMER hWaitForLaunchTimer = 0;
enum GUI_RACE_MODES raceMode;
char WorkString[20];
static int totalWaitForCameraTime;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN hSFScreen;

void DisplaySFKeyboard(int screenID, WM_HWIN hPrev)
{
	float fData;
	char *pTitle;
	int size, id;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	if (pSFKeyboard)
		DestroyKeyboard(pSFKeyboard);

	SetLastScreen(SFSCREENS_TYPE, screenID);
	hSFScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, SFScreensCallback, 0);

	switch (screenID)
	{
	default:
	case LONGITUDE_SCREEN:
		fData = trackData.startLineLong;
		pTitle = "Enter S/F Longitude";
		size = MAX_LONGITUDE_STRING;
		id = LONGITUDE_SCREEN;
		break;
	case LATITUDE_SCREEN:
		fData = trackData.startLineLat;
		pTitle = "Enter S/F Latitude";
		size = MAX_LATITUDE_STRING;
		id = LATITUDE_SCREEN;
		break;
	case HEADING_SCREEN:
		fData = trackData.startLineHeading;
		pTitle = "Enter S/F Heading";
		size = MAX_HEADING_STRING;
		id = HEADING_SCREEN;
		break;
	case START_LONGITUDE_SCREEN:
		fData = trackData.startLineLong;
		pTitle = "Enter Start Line Longitude";
		size = MAX_LONGITUDE_STRING;
		id = START_LONGITUDE_SCREEN;
		break;
	case START_LATITUDE_SCREEN:
		fData = trackData.startLineLat;
		pTitle = "Enter Start Line Latitude";
		size = MAX_LATITUDE_STRING;
		id = START_LATITUDE_SCREEN;
		break;
	case START_HEADING_SCREEN:
		fData = trackData.startLineHeading;
		pTitle = "Enter Start Line Heading";
		size = MAX_HEADING_STRING;
		id = START_HEADING_SCREEN;
		break;
	case FINISH_LONGITUDE_SCREEN:
		fData = trackData.finishLineLong;
		pTitle = "Enter Finish Line Longitude";
		size = MAX_LONGITUDE_STRING;
		id = FINISH_LONGITUDE_SCREEN;
		break;
	case FINISH_LATITUDE_SCREEN:
		fData = trackData.finishLineLat;
		pTitle = "Enter Finish Line Latitude";
		size = MAX_LATITUDE_STRING;
		id = FINISH_LATITUDE_SCREEN;
		break;
	case FINISH_HEADING_SCREEN:
		fData = trackData.finishLineHeading;
		pTitle = "Enter Finish Line Heading";
		size = MAX_HEADING_STRING;
		id = FINISH_HEADING_SCREEN;
		break;
	}

	sprintf(WorkString, "%f", fData);
	trim(WorkString);
	pSFKeyboard = CreateKeyboard(pTitle, WorkString, NUMERIC, true, size, id, hSFScreen, true);
}

void SFScreensSaveData(void)
{
	switch (GetLastScreen(SFSCREENS_TYPE))
	{
	case LONGITUDE_SCREEN:
	case START_LONGITUDE_SCREEN:
		trackData.startLineLong = atof(WorkString);
		break;

	case LATITUDE_SCREEN:
	case START_LATITUDE_SCREEN:
		trackData.startLineLat = atof(WorkString);
		break;

	case HEADING_SCREEN:
	case START_HEADING_SCREEN:
		trackData.startLineHeading = atof(WorkString);
		break;

	case FINISH_LONGITUDE_SCREEN:
		trackData.finishLineLong = atof(WorkString);
		break;

	case FINISH_LATITUDE_SCREEN:
		trackData.finishLineLat = atof(WorkString);
		break;

	case FINISH_HEADING_SCREEN:
		trackData.finishLineHeading = atof(WorkString);
		break;
	}

	alteredTrackData = true;
}

void SFScreensCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
		case KB_CANCEL_MSG:
			DeleteSFScreensKeyboards();
			DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, GetLastScreen(TRACK_SETTINGS_SCREENS_TYPE), hSFScreens);
			break;

		case KB_SAVE_MSG:
			SFScreensSaveData();
			break;

		case KB_SAVE_AND_EXIT_MSG:
			SFScreensSaveData();
			DeleteSFScreensKeyboards();
			DisplayNewScreenList(TRACK_SETTINGS_SCREENS_TYPE, GetLastScreen(TRACK_SETTINGS_SCREENS_TYPE), hSFScreens);
			break;

		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			SFScreensIgnoreNextKeyRelease();
			break;

		case WM_DELETE:
			DeleteSFScreensKeyboards();
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}


void SFScreensIgnoreNextKeyRelease(void)
{
}

void DeleteSFScreensKeyboards(void)
{
	if (pSFKeyboard)
	{
		DestroyKeyboard(pSFKeyboard);
		pSFKeyboard = 0;
	}
}

char ValidateSFCoordinates(void)
{
	char results = true;

	if ((trackData.startLineHeading < 0.0) || (trackData.startLineHeading > 360.0))
		results = false;
	if ((trackData.startLineLong < -180.0) || (trackData.startLineLong > 180.0))
		results = false;
	if ((trackData.startLineLat < -90.0) || (trackData.startLineLat > 90.0))
		results = false;
	if ((trackData.finishLineHeading < 0.0) || (trackData.finishLineHeading > 360.0))
		results = false;
	if ((trackData.finishLineLong < -180.0) || (trackData.finishLineLong > 180.0))
		results = false;
	if ((trackData.finishLineLat < -90.0) || (trackData.finishLineLat > 90.0))
		results = false;

	if (results == false)
	{
		if (InvalidSFDataPopUp() == GUI_ID_CANCEL)
			return false;			// go back and fix it
		else
		{
			// Auto-fix the data
			if (trackData.startLineHeading < 0.0)
				trackData.startLineHeading = 0.0;
			else if (trackData.startLineHeading > 360.0)
				trackData.startLineHeading = 360.0;

			if (trackData.startLineLong < -180.0)
				trackData.startLineLong = -180.0;
			else if (trackData.startLineLong > 180.0)
				trackData.startLineLong = 180.0;

			if (trackData.startLineLat < -90.0)
				trackData.startLineLat = -90.0;
			else if (trackData.startLineLat > 90.0)
				trackData.startLineLat = 90.0;

			if (trackData.finishLineHeading < 0.0)
				trackData.finishLineHeading = 0.0;
			else if (trackData.finishLineHeading > 360.0)
				trackData.finishLineHeading = 360.0;

			if (trackData.finishLineLong < -180.0)
				trackData.finishLineLong = -180.0;
			else if (trackData.finishLineLong > 180.0)
				trackData.finishLineLong = 180.0;

			if (trackData.finishLineLat < -90.0)
				trackData.finishLineLat = -90.0;
			else if (trackData.finishLineLat > 90.0)
				trackData.finishLineLat = 90.0;

			return true;
		}
	}
	return true;
}

static const GUI_WIDGET_CREATE_INFO InvalidSFResources[] = {
	{ FRAMEWIN_CreateIndirect, "Settings Error", 0, 40, 20, 380, 220, 0, 0 },
	{ TEXT_CreateIndirect, "The values you entered are not", 0, 0, 10, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "valid start/finish coordinates.", 0, 0, 40, 360, 30, TEXT_CF_HCENTER },
	{ TEXT_CreateIndirect, "Would you like to auto-fix or go back?", 0, 0, 70, 360, 30, TEXT_CF_HCENTER },
	{ BUTTON_CreateIndirect, "Auto-fix",	GUI_ID_OK, 35, 115, 120, 60 },
	{ BUTTON_CreateIndirect, "Go Back", GUI_ID_CANCEL, 195, 115, 120, 60 },
};

//
// Tell user about the bad Start/Finish Coordinates data
//
// Returns	GUI_ID_OK - continue
//			GUI_ID_CANCEL - go back and fix it
//
int InvalidSFDataPopUp(void)
{
	SetPopupDefaults(WARNING_POPUP);
	return GUI_ExecDialogBox(InvalidSFResources, GUI_COUNTOF(InvalidSFResources), &popupCallback, WM_GetDesktopWindow(), 10, 10);
}


///////////////////////////////////////////////////////////////////////////
//
// Special intermediate screen(s)
//
void DisplayWaitingForCameraScreen(WM_HWIN hPrev)
{
BUTTON_Handle hBtn;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	if ((hWaitingForCamera = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, WaitingForCameraCallback, 0)) <= 0)
		SystemError();

	hBtn = BUTTON_CreateEx(139, 200, BUTTON_WIDTH, BUTTON_HEIGHT, hWaitingForCamera, WM_CF_SHOW, 0, RUN_WITHOUT_BUTTON);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetText(hBtn, "Run Without");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	SetScreenType(WAITING_FOR_CAMERA_SCREEN_TYPE);
	hWaitForCameraTimer = WM_CreateTimer(hWaitingForCamera, 0, WAIT_FOR_CAMERA_TIME, 0);
	totalWaitForCameraTime = 0;
}

static void WaitingForCameraCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_NOTIFY_PARENT)
	{
		if (pMsg->Data.v == WM_NOTIFICATION_CLICKED)
			if (WM_GetId(pMsg->hWinSrc) == RUN_WITHOUT_BUTTON)
			{
				GaugeScreensIgnoreNextKeyRelease();		// Exiting this screen by PRESSING a button (ie touch screen currently PRESSED), tell next screen to ignore the next key release
				raceModeState = RM_CAMERA_RECORDING;		// Note: Run Without is equivelent to camera ready as we will record whether or not the camera is there
				GoRacingStateMachine(hWaitingForCamera);
			}
	}
	else if (pMsg->MsgId == WM_PAINT)
	{
		GUI_Clear();
		GUI_DrawBitmap(&bmWaitingForCamera, LEFT, TOP);
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
	}
	else if (pMsg->MsgId == WM_TIMER)
	{
		if (pMsg->Data.v == hWaitForCameraTimer)
		{
			if (GetValue(CAMERA_STATUS).iVal == CAMERA_STATUS_RECORDING)
			{
				raceModeState = RM_CAMERA_RECORDING;
				GoRacingStateMachine(hWaitingForCamera);
			}
			else
			{
				// Keep checking for camera every 1/2 second.  If the camera becomes ready then proceed.
				// If the camera is not ready within 10 seconds proceed without it.
				totalWaitForCameraTime += WAIT_FOR_CAMERA_TIME;
				if (totalWaitForCameraTime > TOTAL_WAIT_FOR_CAMERA_TIME)
				{
					raceModeState = RM_CAMERA_RECORDING;		// Note: Timing out is equivelent to camera ready as we will record whether or not the camera is there
					GoRacingStateMachine(hWaitingForCamera);
				}
				else
				{
					WM_RestartTimer(hWaitForCameraTimer, WAIT_FOR_CAMERA_TIME);
					WM_InvalidateRect(pMsg->hWin, &BannerRect);
				}
			}
		}
	}
	else WM_DefaultProc(pMsg);
}

void DisplayAutoXStageScreen(WM_HWIN hPrev)
{

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();
	SetScreenType(WAITING_FOR_STAGE_SCREEN_TYPE);

	if ((hStageScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, StageScreenCallback, 0)) <= 0)
		SystemError();

	hStageButton = BUTTON_CreateEx(210, 0, 270, 272, hStageScreen, WM_CF_SHOW, 0, STAGE_BUTTON);
	BUTTON_SetBitmap(hStageButton, BUTTON_BI_UNPRESSED, &bmAutocrossTouchToStage);
	hSweepTachButton = BUTTON_CreateEx(24, 29, 103, 66, hStageScreen, WM_CF_SHOW, 0, SWEEPTACH_BUTTON);
	hAnalogGaugeButton = BUTTON_CreateEx(24, 97, 103, 66, hStageScreen, WM_CF_SHOW, 0, ANALOGGAUGE_BUTTON);
	hDigitalGaugeButton = BUTTON_CreateEx(24, 164, 103, 66, hStageScreen, WM_CF_SHOW, 0, DIGITALGAUGE_BUTTON);

	SetGaugeButtonGraphics();
}


static void StageScreenCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_NOTIFY_PARENT)
	{
		if (pMsg->Data.v == WM_NOTIFICATION_CLICKED)
		{
			switch (WM_GetId(pMsg->hWinSrc))
			{
			case STAGE_BUTTON:
#ifdef _WINDOWS
				GaugeScreensIgnoreNextKeyRelease();		// Windows sim uses a key press to launch, real hardware uses accelerometer - so we DO NOT want to ignore
														// the next key release on the hardware
#endif
				raceModeState = RM_STAGED;
				GoRacingStateMachine(hStageScreen);
				break;
			case SWEEPTACH_BUTTON:
				if (selectedRaceMode == DRAG_MODE)
					sysData.lastDragDisplay = SG_SWEEPTACH;
				else sysData.lastAutoXDisplay = SG_SWEEPTACH;
				SetGaugeButtonGraphics();
				break;
			case ANALOGGAUGE_BUTTON:
				if (selectedRaceMode == DRAG_MODE)
					sysData.lastDragDisplay = SG_ANALOG;
				else sysData.lastAutoXDisplay = SG_ANALOG;
				SetGaugeButtonGraphics();
				break;
			case DIGITALGAUGE_BUTTON:
				if (selectedRaceMode == DRAG_MODE)
					sysData.lastDragDisplay = SG_DIGITAL;
				else sysData.lastAutoXDisplay = SG_DIGITAL;
				SetGaugeButtonGraphics();
				break;
			case DRAG_1_8_MILE_BUTTON:
				sysData.dragDistance = DD_1_8;
				SetDistanceButtonGraphics();
				break;

			case DRAG_1000_FEET_BUTTON:
				sysData.dragDistance = DD_1000;
				SetDistanceButtonGraphics();
				break;

			case DRAG_1_4_MILE_BUTTON:
				sysData.dragDistance = DD_1_4;
				SetDistanceButtonGraphics();
				break;
			}
		}
	}
	else if (pMsg->MsgId == WM_PAINT)
	{
		GUI_Clear();
		if (selectedRaceMode == AUTOXRALLY_MODE)
			GUI_DrawBitmap(&bmAutocrossStageBackground, 0, 0);
		else GUI_DrawBitmap(&bmDragStageBackground, 0, 0);
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
	}
	else WM_DefaultProc(pMsg);
}


void DisplayDragStageScreen(WM_HWIN hPrev)
{
	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();
	SetScreenType(WAITING_FOR_STAGE_SCREEN_TYPE);

	if ((hStageScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, StageScreenCallback, 0)) <= 0)
		SystemError();

	hStageButton = BUTTON_CreateEx(210, 0, 270, 272, hStageScreen, WM_CF_SHOW, 0, STAGE_BUTTON);
	BUTTON_SetBitmap(hStageButton, BUTTON_BI_UNPRESSED, &bmDragTouchToStage);
	hSweepTachButton = BUTTON_CreateEx(24, 29, 103, 66, hStageScreen, WM_CF_SHOW, 0, SWEEPTACH_BUTTON);
	hAnalogGaugeButton = BUTTON_CreateEx(24, 97, 103, 66, hStageScreen, WM_CF_SHOW, 0, ANALOGGAUGE_BUTTON);
	hDigitalGaugeButton = BUTTON_CreateEx(24, 164, 103, 66, hStageScreen, WM_CF_SHOW, 0, DIGITALGAUGE_BUTTON);
	SetGaugeButtonGraphics();

	h1_8thMile = BUTTON_CreateEx(131, 29, 72, 65, hStageScreen, WM_CF_SHOW, 0, DRAG_1_8_MILE_BUTTON);
	h1_4Mile = BUTTON_CreateEx(131, 97, 72, 65, hStageScreen, WM_CF_SHOW, 0, DRAG_1_4_MILE_BUTTON);
	h1000ft = BUTTON_CreateEx(131, 164, 72, 65, hStageScreen, WM_CF_SHOW, 0, DRAG_1000_FEET_BUTTON);
	SetDistanceButtonGraphics();
}


void SetDistanceButtonGraphics(void)
{
	if (sysData.dragDistance == DD_1_8)
	{
		BUTTON_SetBitmap(h1_4Mile, BUTTON_BI_UNPRESSED, &bmOneQuarterOff);
		BUTTON_SetBitmap(h1000ft, BUTTON_BI_UNPRESSED, &bmOneThousandOff);
		BUTTON_SetBitmap(h1_8thMile, BUTTON_BI_UNPRESSED, &bmOneEighthOn);
	}
	else if (sysData.dragDistance == DD_1000)
	{
		BUTTON_SetBitmap(h1_4Mile, BUTTON_BI_UNPRESSED, &bmOneQuarterOff);
		BUTTON_SetBitmap(h1000ft, BUTTON_BI_UNPRESSED, &bmOneThousandOn);
		BUTTON_SetBitmap(h1_8thMile, BUTTON_BI_UNPRESSED, &bmOneEighthOff);
	}
	else
	{
		BUTTON_SetBitmap(h1_4Mile, BUTTON_BI_UNPRESSED, &bmOneQuarterOn);
		BUTTON_SetBitmap(h1000ft, BUTTON_BI_UNPRESSED, &bmOneThousandOff);
		BUTTON_SetBitmap(h1_8thMile, BUTTON_BI_UNPRESSED, &bmOneEighthOff);
	}
}

void SetGaugeButtonGraphics(void)
{
	enum SELECTED_GAUGE selectedGauge;

	if (selectedRaceMode == DRAG_MODE)
	{
		selectedGauge = (enum SELECTED_GAUGE)sysData.lastDragDisplay;

		if (selectedGauge == SG_SWEEPTACH)
			BUTTON_SetBitmap(hSweepTachButton, BUTTON_BI_UNPRESSED, &bmDragSweepTachButtonOn);
		else BUTTON_SetBitmap(hSweepTachButton, BUTTON_BI_UNPRESSED, &bmSweepTachButtonOff);
		if (selectedGauge == SG_ANALOG)
			BUTTON_SetBitmap(hAnalogGaugeButton, BUTTON_BI_UNPRESSED, &bmDragAnalogGaugeButtonOn);
		else BUTTON_SetBitmap(hAnalogGaugeButton, BUTTON_BI_UNPRESSED, &bmAnalogGaugeButtonOff);
		if (selectedGauge == SG_DIGITAL)
			BUTTON_SetBitmap(hDigitalGaugeButton, BUTTON_BI_UNPRESSED, &bmDragDigitalGaugeButtonOn);
		else BUTTON_SetBitmap(hDigitalGaugeButton, BUTTON_BI_UNPRESSED, &bmDigitalGaugeButtonOff);
	}
	else
	{
		selectedGauge = (enum SELECTED_GAUGE)sysData.lastAutoXDisplay;

		if (selectedGauge == SG_SWEEPTACH)
			BUTTON_SetBitmap(hSweepTachButton, BUTTON_BI_UNPRESSED, &bmAutoXSweepTachButtonOn);
		else BUTTON_SetBitmap(hSweepTachButton, BUTTON_BI_UNPRESSED, &bmSweepTachButtonOff);
		if (selectedGauge == SG_ANALOG)
			BUTTON_SetBitmap(hAnalogGaugeButton, BUTTON_BI_UNPRESSED, &bmAutoXAnalogGaugeButtonOn);
		else BUTTON_SetBitmap(hAnalogGaugeButton, BUTTON_BI_UNPRESSED, &bmAnalogGaugeButtonOff);
		if (selectedGauge == SG_DIGITAL)
			BUTTON_SetBitmap(hDigitalGaugeButton, BUTTON_BI_UNPRESSED, &bmAutoXDigitalGaugeButtonOn);
		else BUTTON_SetBitmap(hDigitalGaugeButton, BUTTON_BI_UNPRESSED, &bmDigitalGaugeButtonOff);
	}
}


#define	READY_TO_LAUNCH_X	20
#define	READY_TO_LAUNCH_Y	40

void DisplayLaunchScreen(WM_HWIN hPrev)
{
	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();
	SetScreenType(WAITING_FOR_LAUNCH_SCREEN_TYPE);

	if ((hLaunchScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, LaunchScreenCallback, 0)) <= 0)
		SystemError();

#ifdef _WINDOWS
	hLaunchButton = BUTTON_CreateEx(READY_TO_LAUNCH_X, READY_TO_LAUNCH_Y, 415, 167, hLaunchScreen, WM_CF_SHOW, 0, LAUNCH_BUTTON);
	if (selectedRaceMode == AUTOXRALLY_MODE)
		BUTTON_SetBitmap(hLaunchButton, BUTTON_BI_UNPRESSED, &bmReadyToLaunchCorvette);
	else BUTTON_SetBitmap(hLaunchButton, BUTTON_BI_UNPRESSED, &bmReadytoLaunchBurnout);
#endif

	hWaitForLaunchTimer = WM_CreateTimer(hLaunchScreen, 0, WAIT_FOR_LAUNCH_TIME, 0);
}

static void LaunchScreenCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_PAINT)
	{
		GUI_Clear();
#ifndef _WINDOWS
		if (selectedRaceMode == AUTOXRALLY_MODE)
			GUI_DrawBitmap(&bmReadyToLaunchCorvette, READY_TO_LAUNCH_X, READY_TO_LAUNCH_Y);
		else GUI_DrawBitmap(&bmReadytoLaunchBurnout, READY_TO_LAUNCH_X, READY_TO_LAUNCH_Y);
#endif
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
	}
	else if (pMsg->MsgId == WM_TIMER)
	{
		if (pMsg->Data.v == hWaitForLaunchTimer)
		{
			if (LaunchDetected())
			{
#ifdef _WINDOWS
				GaugeScreensIgnoreNextKeyRelease();		// Windows sim uses a key press to launch, real hardware uses accelerometer - so we DO NOT want to ignore
														// the next key release on the hardware
#endif
				raceModeState = RM_LAUNCH_DETECTED;
				GoRacingStateMachine(hLaunchScreen);
			}
			else WM_RestartTimer(hWaitForLaunchTimer, WAIT_FOR_LAUNCH_TIME);
		}
	}
#ifdef _WINDOWS
	else if (pMsg->MsgId == WM_NOTIFY_PARENT)
	{
		if (pMsg->Data.v == WM_NOTIFICATION_CLICKED)
		{
			if (WM_GetId(pMsg->hWinSrc) == LAUNCH_BUTTON)
			{
				GaugeScreensIgnoreNextKeyRelease();
				raceModeState = RM_LAUNCH_DETECTED;
				GoRacingStateMachine(hLaunchScreen);
				unsavedSysData.systemMode = DU2_AUTOX_WAITFINISH;
			}
		}
	}
#endif
	else WM_DefaultProc(pMsg);
}

int LaunchDetected(void)
{
	if ((unsavedSysData.systemMode == DU2_AUTOX) || (unsavedSysData.systemMode == DU2_AUTOX_WAITFINISH) || (unsavedSysData.systemMode == DU2_DRAG))
		return true;
	else return false;
}


void DisplayTapAtSFScreen(WM_HWIN hPrev)
{
BUTTON_Handle hButton;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenScrollingHOnly();
	SetScreenType(WAITING_FOR_SF_SCREEN_TYPE);

	if ((hTapAtSFScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, TapAtSFScreenCallback, 0)) <= 0)
		SystemError();

// KMC 2/11/12 - fix to shrink TapScreenAtSF button to allow laft arrow button to be seen
//	hButton = BUTTON_CreateEx(0, 0, 480, 272, hTapAtSFScreen, WM_CF_SHOW, 0, START_BUTTON);
	hButton = BUTTON_CreateEx(30, 0, 440, 272, hTapAtSFScreen, WM_CF_SHOW, 0, START_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmTapScreenAtSF);
}

WM_HWIN GetSFWindowHandle(void)
{
	return hTapAtSFScreen;
}


WM_HWIN GetLaunchScreenWindowHandle(void)
{
	return hLaunchScreen;
}

WM_HWIN GetStageWindowHandle(void)
{
	return hStageScreen;
}

static void TapAtSFScreenCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_NOTIFY_PARENT)
	{
		if (pMsg->Data.v == WM_NOTIFICATION_CLICKED)
			if (WM_GetId(pMsg->hWinSrc) == START_BUTTON)
				StartPressed(hTapAtSFScreen);
	}
	else if (pMsg->MsgId == WM_PAINT)
	{
		GUI_Clear();
		GUI_DrawBitmap(&bmleftarrow, 0, 229);
	}
	else WM_DefaultProc(pMsg);
}

enum WFSTYPE typeSave;
WM_HWIN parentSave;

void DisplayWaitingForSatellites(enum WFSTYPE type, enum GUI_RACE_MODES mode, WM_HWIN hPrev)
{
	BUTTON_Handle hBtn;

	if ((typeSave = type) == DISPLAY_TRACKLIST_TYPE)
	{
		parentSave = hPrev;
	}
	else
	{
		raceMode = mode;
		if (hPrev)
			WM_DeleteWindow(hPrev);
	}

	SetScreenType(WAITING_FOR_SATELLITES_SCREEN_TYPE);

	SetScreenScrollingHOnly();

//	if ((hWaitingForSatellites = WM_CreateWindow(0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, WaitingForSatellitesCallback, 0)) <= 0)
	if ((hWaitingForSatellites = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, WaitingForSatellitesCallback, 0)) <= 0)
		SystemError();

	hBtn = BUTTON_CreateEx(139, 200, BUTTON_WIDTH, BUTTON_HEIGHT, hWaitingForSatellites, WM_CF_SHOW, 0, CANCEL_BUTTON);
	BUTTON_SetFont(hBtn, &GUI_Font24_ASCII);
	BUTTON_SetBitmap(hBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetText(hBtn, "Cancel");
	BUTTON_SetTextColor(hBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);

	hWaitForSatelliteTimer = WM_CreateTimer(hWaitingForSatellites, 0, WAIT_FOR_SATELLITE_TIME, 0);
}

static void WaitingForSatellitesCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_NOTIFY_PARENT)
	{
		if (pMsg->Data.v == WM_NOTIFICATION_CLICKED)
			if (WM_GetId(pMsg->hWinSrc) == CANCEL_BUTTON)
			{
				if (typeSave == DISPLAY_TRACKLIST_TYPE)
					sysData.radius = WHOLE_WORLD;
				KillWaitingForSatelliteScreen(false);
			}
	}
	else if (pMsg->MsgId == WM_PAINT)
	{
		GUI_Clear();
		GUI_SetFont(&GUI_Font24_ASCII);
		GUI_SetColor(GUI_WHITE);
		GUI_SetTextMode(GUI_TEXTMODE_TRANS);
		GUI_DrawBitmap(&bmGPSsignal, LEFT, TOP);
		GUI_DispStringAt("Waiting for", LEFT + 50, TOP + 50);
		GUI_DispStringAt("satellite signal", LEFT + 50, TOP + 75);

		GUI_DrawBitmap(&bmleftarrow, 0, 229);
	}
	else if (pMsg->MsgId == WM_TIMER)
	{
		if (pMsg->Data.v == hWaitForSatelliteTimer)
		{
			if (HaveSatelliteReception())
				KillWaitingForSatelliteScreen(true);
			else
			{
				WM_RestartTimer(hWaitForSatelliteTimer, WAIT_FOR_SATELLITE_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
		}
	}
	else WM_DefaultProc(pMsg);
}

int HaveSatelliteReception(void)
{
	if ((GetValue(GPS_NUMSAT).uVal) < GPS_GOOD)
		return false;
	else return true;
}

void KillWaitingForSatelliteScreen(char foundSatellites)
{
	if (typeSave == GO_RACING_TYPE)
	{
		if (foundSatellites)
			raceModeState = RM_FOUND_SATELLITES;
		else raceModeState = RM_CANCELLED_WFS;
		GoRacingStateMachine(hWaitingForSatellites);
	}
	else
	{
		ResumeScreenScrolling();
		WM_DeleteWindow(hWaitingForSatellites);
		SetScreenType(RACE_SETUP_SCREENS_TYPE);
		SetWorkingTrackName();
		FinishDisplayTrackSelection(parentSave);
		GUI_Clear();
	}
}

static WM_HWIN hDragAutoXGauges;



WM_HWIN GetDragAutoXGaugesWindowHandle(void)
{
	return hDragAutoXGauges;
}


/*********************************************************************
*
*/
void DragAutoXGauges(WM_HWIN hPrev)
{
	enum SELECTED_GAUGE screen;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	SetScreenType(DRAG_AUTOX_GAUGE_SCREEN_TYPE);

	GUI_Clear();

	if (selectedRaceMode == DRAG_MODE)
		screen = (enum SELECTED_GAUGE)sysData.lastDragDisplay;
	else screen = (enum SELECTED_GAUGE)sysData.lastAutoXDisplay;

	switch (screen)
	{
	case SG_SWEEPTACH:
		hDragAutoXGauges = Display_DAXRPMSweepScreen();
		return;
	case SG_ANALOG:
		hDragAutoXGauges = Display_DAXGaugeScreen();
		return;
	case SG_DIGITAL:
		hDragAutoXGauges = Display_DAXDigitalGaugeScreen();
		break;
	}
}



/*************************** End of file ****************************/

