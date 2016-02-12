
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "GPSInfoScreen.h"
#include "RunningGauges.h"
#include "DataHandler.h"
#ifndef _WINDOWS
#include "datadecode.h"
#include "adc.h"
#endif
#include "SFScreens.h"
#include "LEDHandler.h"

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmGPSInfoText;
extern GUI_CONST_STORAGE GUI_BITMAP bmDarkAsphalt;
extern char lockupSemaphore;

#ifndef _WINDOWS
extern int warningLEDPWMSetting;
extern int tachLEDPWMSetting;
extern int backlightPWMSetting;
extern signed int lightCorrection, tempCorrection, totalCorrection;
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

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HTIMER hScreenTimer;
static WM_HWIN hGPSInfo;
static char firsttimethrough;
static WM_HTIMER hTestScreenTimer;
static WM_HWIN hTestInfo;
static char testfirsttimethrough;

#define	TOP_LINE			40
#define	LEFT_COLUMN		25
#define	RIGHT_COLUMN	275
#define	LINE_GAP			40
static GUI_MEMDEV_Handle memGPS;
static GUI_MEMDEV_Handle memTest;


/*********************************************************************
*
*       GPS Info Screen Callback
*/
void GPSInfoCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hGPSInfo);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(GPS_INFO_SCREEN) || firsttimethrough)
		{
			char scratch[50];
			float speed;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			firsttimethrough = FALSE;

			GUI_MEMDEV_Select(memGPS);
			GUI_DrawBitmap(&bmDarkAsphalt, 0, 0);

			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font24_ASCII);
			GUI_SetTextMode(GUI_TM_TRANS);

			// Draw screen title
			GUI_DrawBitmap(&bmGPSInfoText, 0, 0);
//			GUI_DispStringHCenterAt("GPS Satellite Information", XSIZE_PHYS/2, 5);

			// Draw GPS LON
			sprintf(scratch, "GPS Lat: %0.6f   ", (float)GetValue(LATITUDE).dVal);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE);

			// Draw GPS LON
			sprintf(scratch, "GPS Lon: %0.6f   ", (float)GetValue(LONGITUDE).dVal);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 1));

			// Draw Altitude
			sprintf(scratch, "Altitude: %0.0f %s   ", GetValue(ALTITUDE).fVal, sysData.units == METRIC ? "meters" : "feet");
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 2));

			// Draw GPS DOP
			sprintf(scratch, "GPS DOP: %0.2f   ", GetValue(GPS_DOP).fVal);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 3));

			// Draw Number of satellites
			sprintf(scratch, "Num Sats: %d   ", GetValue(GPS_NUMSAT).uVal);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 4));

			// Draw Number of satellites
			// Velocity always returned in current units
			speed = GetValue(SPEED).fVal;
			sprintf(scratch, "Velocity: %0.1f %s   ", speed, sysData.units == METRIC ? "kph" : "mph");
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE);

			// Draw GPS Heading
			if (speed < 0.1)
				sprintf(scratch, "Heading: ---   ");
			else
				sprintf(scratch, "Heading: %0.0f   ", GetValue(HEADING).fVal);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 1));

			// Draw X-G
			sprintf(scratch, "Xg: %+0.2f   ", GetValue(X_G_DAMPED).fVal);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 2));

			// Draw Y-G
			sprintf(scratch, "Yg: %+0.2f   ", GetValue(Y_G_DAMPED).fVal);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 3));

			// Draw Z-G
			// If SU HW Rev < 200 display temp else display Z
			if (suData.suHwRev < 200)
				sprintf(scratch, "Temp: %0.1f %s   ", GetValue(SU_TEMPERATURE).fVal, sysData.units == METRIC ? "C" : "F");
			else sprintf(scratch, "Zg: %+0.2f   ", GetValue(Z_G).fVal);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 4));

			// Draw time of day if we have satellite lock
			if (HaveSatelliteReception())
			{
#ifdef _WINDOWS
				GetTimeString(LONGDANDT, scratch);
#else
				formattime(LONGDANDT, scratch);
#endif
			}
			else strcpy(scratch, "                                       ");

			GUI_DispStringHCenterAt(scratch, XSIZE_PHYS/2, TOP_LINE + (LINE_GAP * 5));

			// Draw navigation arrow
			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
				CheckRaceHelpScreen(GPS_INFO_SCREEN);
#endif

			CheckWarningPopups();
			GUI_MEMDEV_CopyToLCD(memGPS);
			GUI_SelectLCD();
		}
		if (SlideOutVisible())
			DrawSignalStrength();
		break;

	case WM_DELETE:
		GUI_MEMDEV_Delete(memGPS);
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       GPS Info Screen Create
*/
WM_HWIN DisplayGPSInfo(WM_HWIN hParent)
{
	firsttimethrough = true;

	// create a memory area to reduce flicker on paint.
	memGPS = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hGPSInfo = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, GPSInfoCallback, 0)) <= 0)
		SystemError();

	WM_InvalidateWindow(hGPSInfo);

	hScreenTimer = WM_CreateTimer(hGPSInfo, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hGPSInfo;
}

/*********************************************************************
*
*       GPS Info Screen Callback
*/
void TestInfoCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hTestScreenTimer)
		{
			WM_RestartTimer(hTestScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hTestInfo);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(TEST_INFO_SCREEN) || testfirsttimethrough)
		{
			char scratch[40];

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			testfirsttimethrough = FALSE;

			GUI_MEMDEV_Select(memTest);
			GUI_Clear();
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font24_ASCII);

			// Draw screen title
			GUI_DispStringHCenterAt("Test Information", XSIZE_PHYS/2, 5);

			// Draw Light Level
#ifdef _WINDOWS
			strcpy(scratch, "Light Level: N/A");
#else
			sprintf(scratch, "Raw Light Level: %d       ", GetADC(ADC_LIGHT_LEVEL));
#endif
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE);

			// Draw LED Board Temperature
#ifdef _WINDOWS
			strcpy(scratch, "LED Temp: N/A");
#else
			sprintf(scratch, "LED Temp: %d   ", GetADC(ADC_LED_BOARD_TEMP));
#endif
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 1));

#ifndef _WINDOWS
	//		// Draw Side LED
	//		sprintf(scratch, "Side LED intensity: %d   ", warningLEDPWMSetting);
			// Draw raw light reading
			sprintf(scratch, "Normalized Light: %d       ", GetNormalizedLightIntensity());
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 2));

			// Draw Tach LED
			sprintf(scratch, "Tach LED intensity: %d   ", tachLEDPWMSetting);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 3));

			// Draw Backlight
			sprintf(scratch, "Backlight intensity: %d   ", backlightPWMSetting);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 4));

			// Draw Vehicle Power
			sprintf(scratch, "Battery voltage: %2.1f   ", GetValue(VEHICLE_12V).fVal);
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 5));
			
			
#if LIGHTTEST			
			// Draw Light level when adjustments made
			sprintf(scratch, "Light set at: %d   ", sysData.lightLevelWhenIntensitySet);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE);

			// Draw light correction
			sprintf(scratch, "Light Correction: %d   ", lightCorrection);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 1));

			// Draw temp correction
			sprintf(scratch, "Temp Correction: %d   ", tempCorrection);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 2));

			// Draw total correction
			sprintf(scratch, "Total Correction: %d   ", totalCorrection);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 3));
#else
			// Show total distance
			sprintf(scratch, "Distance Mi: %1.1f   ", (float) sysData.totalDistanceEver * (39.0 / 12.0 / 5280.0));
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE);

			// Draw total operating time
			sprintf(scratch, "Operating Hrs: %1.1f   ", sysData.totalTimeEver / 3600.0);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 1));

			// Draw Highest RPM
			sprintf(scratch, "Highest Rev: %d   ", sysData.maxRpmEver);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 2));
#endif
			// Draw SU, DU info
			sprintf(scratch, "DF: %d, %d   ", unsavedSysData.dataflash[0].bytesPerPage, unsavedSysData.dataflash[1].bytesPerPage);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 4));
			
			// Draw SU, DU info
			sprintf(scratch, "SU %d.%02d, DU %d.%02d  ", suData.suSwRev / 100, suData.suSwRev % 100, unsavedSysData.duSwRev / 100,  unsavedSysData.duSwRev % 100);
			GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 5));
#endif

			//// Draw X-G
			//// If SU HW Rev < 200 display temp else display Z
			//if (suData.su_hw_rev < 200)
			//	sprintf(scratch, "Temp: %.1f %s   ", GetValue(SU_TEMPERATURE).fVal, sysData.units == METRIC ? "C" : "F");
			//else sprintf(scratch, "Z - G: %.2f   ", GetValue(Z_G).fVal);
			//GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 4));

			// Draw navigation arrow
			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);
			CheckWarningPopups();
			GUI_MEMDEV_CopyToLCD(memTest);
			GUI_SelectLCD();
		}
		break;

	case WM_DELETE:
		GUI_MEMDEV_Delete(memTest);
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       GPS Info Screen Create
*/
WM_HWIN DisplayTestInfo(WM_HWIN hParent)
{
	testfirsttimethrough = true;

	// create a memory area to reduce flicker on paint.
	memTest = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hTestInfo = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, TestInfoCallback, 0)) <= 0)
		SystemError();

	WM_InvalidateWindow(hTestInfo);

	hTestScreenTimer = WM_CreateTimer(hTestInfo, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hTestInfo;
}

/*************************** End of file ****************************/
