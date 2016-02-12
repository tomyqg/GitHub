
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DriveScreen.h"
#include "gauge.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "SFScreens.h"
#include "LEDHandler.h"

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
#endif

extern GUI_CONST_STORAGE GUI_BITMAP bmDriveModeBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriveModeBackgroundNewCompass;
extern GUI_CONST_STORAGE GUI_BITMAP bmCompassList;
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmDriveButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmN;
extern GUI_CONST_STORAGE GUI_BITMAP bmNE;
extern GUI_CONST_STORAGE GUI_BITMAP bmE;
extern GUI_CONST_STORAGE GUI_BITMAP bmSE;
extern GUI_CONST_STORAGE GUI_BITMAP bmS;
extern GUI_CONST_STORAGE GUI_BITMAP bmSW;
extern GUI_CONST_STORAGE GUI_BITMAP bmW;
extern GUI_CONST_STORAGE GUI_BITMAP bmNW;
extern char lockupSemaphore;

#ifdef __cplusplus
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Defines and Constants
*
**********************************************************************
*/
enum Headings { NORTH, NORTHEAST, EAST, SOUTHEAST, SOUTH, SOUTHWEST, WEST, NORTHWEST };
const GUI_RECT SpeedRect = { 0, 8, 90, 42 };
const GUI_RECT TimeRect = { 320, 8, 320+115, 42 };

const GUI_RECT MaxSpeedRect = { 95, 99, 95+141, 99+33 };
const GUI_RECT LatGRect = { 385, 100, 385+100, 100+30 };
const GUI_RECT MaxBrakeRect = { 385, 150, 385+100, 150+30 };
const GUI_RECT MaxLaunchRect = { 385, 200, 385+100, 200+30 };
const GUI_RECT TripDistanceRect = { 95, 150, 95+200, 150+30 };
const GUI_RECT ElapsedTimeRect = { 95, 200, 95+200, 200+30 };

#define	RESET_TRIP_BTN		GUI_ID_USER + 1
#define	RESET_MAX_BTN		GUI_ID_USER + 2
#define FEET_PER_MILE		5280.0
#define METERS_PER_MILE		1609.344
#define METERS_PER_KM		1000.0
#define	DRIVE_SCREEN_REFRESH_TIME	50

#define	COMPASS_WINDOW_LEFT			171
#define	COMPASS_WINDOW_TOP			8
#define	COMPASS_WINDOW_WIDTH		138
#define	PIXELS_PER_360_DEGREES		506.0
#define	OFFSET_TO_NORTH_IN_STRIP	105
#define	OFFSET_OF_NORTH_IN_VIEW_WINDOW	69
#define	DEFAULT_NORTH_OFFSET		OFFSET_TO_NORTH_IN_STRIP - OFFSET_OF_NORTH_IN_VIEW_WINDOW
#define	COMPASS_CHANGE_KPH			8.3
#define	COMPASS_CHANGE_MPH			5.0

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
BUTTON_Handle hResetTrip, hResetMax;
static WM_HTIMER hScreenTimer;
static WM_HWIN hDriveScreen;
static bool firsttimethrough;
static GUI_MEMDEV_Handle memDriveScreen;
static int offset, previousOffset, workingOffset;
static float heading;
static int speed, time;
static float float_1, float_2;
static char speedStr[30];
static char yGStr[10], xGStr[10], maxXGStr[10], minXGStr[10];
static char maxSpeedStr[20];
static char timeStr[30];
static char tripDistStr[30];
static char elapsedStr[30];


void _cbDriveCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
	float speedFloat;

	switch(pMsg->MsgId)
	{
	case WM_CREATE:
		// First time through - set initial heading and offset
		heading = GetValue(HEADING).fVal;

		// Safety check
		while (heading > 360.0)
			heading -= 360.0;

		// Convert the heading (0-360) to a pixel offset within the graphic of the strip
		offset = previousOffset = workingOffset = (int)((heading / 360.0) * PIXELS_PER_360_DEGREES) + OFFSET_TO_NORTH_IN_STRIP - OFFSET_OF_NORTH_IN_VIEW_WINDOW;
		break;

	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, DRIVE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDriveScreen);
		}

		// Get data values
		// Get speed
		speed = (int)(speedFloat = GetValue(SPEED).fVal);

		// Don't show direction if moving too slowly that integer roundoff makes direction "snap"
		if (speedFloat > (sysData.units == STD ? COMPASS_CHANGE_MPH : COMPASS_CHANGE_KPH))
		{
			heading = GetValue(HEADING).fVal;

			// Safety check
			while (heading > 360.0)
				heading -= 360.0;
			// check for actual movement in the PAINT routine so we explicitly only move one pixel per paint!
		}
		// Convert the heading (0-360) to a pixel offset within the graphic of the strip
		// Note - allow compass to continue to move towards last known target direction even if stopped
		workingOffset = (int)((heading / 360.0) * PIXELS_PER_360_DEGREES) + OFFSET_TO_NORTH_IN_STRIP - OFFSET_OF_NORTH_IN_VIEW_WINDOW;
		sprintf(speedStr, "%d", speed);

		// Get Max Speed
		sprintf(maxSpeedStr, "%d%s", (int)(GetValue(MAX_SPEED).fVal), (sysData.units == STD ? "mph" : "kph"));

		GetTimeString(SHORT12, timeStr);

		// Draw max LatG
		float_1 = GetValue(MAX_Y_G).fVal;
		float_2 = 0.0 - GetValue(MIN_Y_G).fVal;
		if (float_2 > float_1)
			float_1 = float_2;
		sprintf(yGStr, "%0.02fg", float_1);

		sprintf(maxXGStr, "%0.02fg", GetValue(MAX_X_G).fVal);
		sprintf(minXGStr, "%0.02fg", 0.0 - GetValue(MIN_X_G).fVal);

		// Native units is meters.
		// convert to miles or km
		if (STD == sysData.units)		// miles
			float_1 = sysData.driveTripDistance / METERS_PER_MILE;
		else							// km
			float_1 = sysData.driveTripDistance / METERS_PER_KM;
		sprintf(tripDistStr, "%0.1f%s", float_1, (sysData.units == STD ? "miles" : "km"));

		time = sysData.driveTripTime / 1000;
		if (time < 3600)
			sprintf(elapsedStr, "%02d:%02d", time / 60, time % 60);
		else
			sprintf(elapsedStr, "%02d:%02d:%02d", time/3600, (time/60)%60, time % 60);

		break;


	case WM_PAINT:
		if (CS_IsVisible(DRIVE_MODE_SCREEN) || firsttimethrough)
		{
			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore++;
			
			if (firsttimethrough)
			{
				GUI_Clear();			// Clear screen to remove artifacts from previous screen in area at bottom of screen NOT part of memory device area
				firsttimethrough = FALSE;
			}

			// Draw compass strip under background image
			GUI_MEMDEV_Select(memDriveScreen);

			// Draw strip at specified offset
			// check for actual movement in the PAINT routine so we explicitly only move one pixel per paint!
			// Move one or two pixels in the required direction
			if (workingOffset > previousOffset)
			{
				offset++;
				previousOffset++;
				if (workingOffset > previousOffset)
					offset++;
			}
			else if (workingOffset < previousOffset)
			{
				offset--;
				previousOffset--;
				if (workingOffset < previousOffset)
					offset--;
			}

			previousOffset = offset;

			GUI_DrawBitmap(&bmCompassList, COMPASS_WINDOW_LEFT - offset, COMPASS_WINDOW_TOP);

			/* Paint the background window */
			GUI_DrawBitmap(&bmDriveModeBackgroundNewCompass, 0, 0);

			GUI_SetTextMode(GUI_TM_TRANS);
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_FontD32);

			// Draw speed
			GUI_DispStringInRect(speedStr, (GUI_RECT *)&SpeedRect, GUI_TA_HCENTER);

			// Draw time of day if we have satellite lock
			if (HaveSatelliteReception())
			{
				GUI_DispStringInRect(timeStr, (GUI_RECT *)&TimeRect, GUI_TA_RIGHT);
				GUI_SetFont(&GUI_Font24B_ASCII);
				GUI_DispStringAt(timeStr+strlen(timeStr)-2, 440, 22);
			}
			else GUI_SetFont(&GUI_Font24B_ASCII);
			
			// Draw speed units
			GUI_SetColor(GUI_GRAY);
			GUI_DispStringAt((sysData.units == STD ? "MPH" : "KPH"), 20, 43);

			// Draw max speed
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font32B_ASCII);
			GUI_DispStringInRect(maxSpeedStr, (GUI_RECT *)&MaxSpeedRect, GUI_TA_LEFT);

			// Draw max LatG
			GUI_DispStringInRect(yGStr, (GUI_RECT *)&LatGRect, GUI_TA_LEFT);

			// Draw max Brake Gs
			GUI_DispStringInRect(maxXGStr, (GUI_RECT *)&MaxBrakeRect, GUI_TA_LEFT);

			GUI_DispStringInRect(xGStr, (GUI_RECT *)&MaxBrakeRect, GUI_TA_LEFT);

			// Draw max Launch Gs
			GUI_DispStringInRect(minXGStr, (GUI_RECT *)&MaxLaunchRect, GUI_TA_LEFT);

			// Draw trip distance
			GUI_DispStringInRect(tripDistStr, (GUI_RECT *)&TripDistanceRect, GUI_TA_LEFT);

			// Draw elapsed time
			GUI_DispStringInRect(elapsedStr, (GUI_RECT *)&ElapsedTimeRect, GUI_TA_LEFT);

			if (SlideOutVisible())
				DrawSignalStrength();

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
			{
				CheckRaceHelpScreen(DRIVE_MODE_SCREEN);
			}
#endif

			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(memDriveScreen);
			GUI_SelectLCD();

			// Draw navigation arrow - NOT within the memory device of this screen so draw seperately
			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);
		}
		break;

	case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
      NCode = pMsg->Data.v;                 // Notification code
	   if (NCode == WM_NOTIFICATION_RELEASED)
		{
			if (!SlideOutDeployed())
			{
				if (Id == RESET_TRIP_BTN)
				{
					ResetTrip();
					WM_InvalidateWindow(hDriveScreen);
				}
				else if (Id == RESET_MAX_BTN)
				{
					ResetMax();
					WM_InvalidateWindow(hDriveScreen);
				}
			}
		}
		break;

	case WM_DELETE:
		if (memDriveScreen)
		{
			GUI_MEMDEV_Delete(memDriveScreen);
			memDriveScreen = 0;
		}
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}


/*********************************************************************
*
*       Drive Screen
*/
WM_HWIN Display_DriveScreen(WM_HWIN hParent)
{
	firsttimethrough = true;

	// create a memory area to reduce flicker on paint.
	memDriveScreen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, 232);

	if ((hDriveScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbDriveCallback, 0)) <= 0)
		SystemError();

	// Create Reset buttons
	hResetTrip = BUTTON_CreateEx(150, 230, 160, 40, hDriveScreen, WM_CF_SHOW | WM_CF_STATIC, 0, RESET_TRIP_BTN);
	BUTTON_SetBitmap(hResetTrip, BUTTON_BI_UNPRESSED, &bmDriveButton);
	BUTTON_SetFont(hResetTrip, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hResetTrip, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hResetTrip, BUTTON_BI_PRESSED, GUI_GRAY);
	BUTTON_SetTextColor(hResetTrip, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetText(hResetTrip, "Reset Trip");

	hResetMax = BUTTON_CreateEx(313, 230, 160, 40, hDriveScreen, WM_CF_SHOW | WM_CF_STATIC, 0, RESET_MAX_BTN);
	BUTTON_SetBitmap(hResetMax, BUTTON_BI_UNPRESSED, &bmDriveButton);
	BUTTON_SetFont(hResetMax, &GUI_Font24B_ASCII);
	BUTTON_SetTextColor(hResetMax, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hResetMax, BUTTON_BI_PRESSED, GUI_GRAY);
	BUTTON_SetTextColor(hResetMax, BUTTON_BI_DISABLED, GUI_WHITE);
	BUTTON_SetText(hResetMax, "Reset Max");

	WM_InvalidateWindow(hDriveScreen);

	hScreenTimer = WM_CreateTimer(hDriveScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDriveScreen;
}

/*************************** End of file ****************************/
