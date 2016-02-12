
#ifdef PROFILER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "ProfilerScreen.h"
#include "RunningGauges.h"
#include "DataHandler.h"
#ifndef _WINDOWS
#include "profiler.h"
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
extern char lockupSemaphore;

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
#ifdef _WINDOWS
static WM_HTIMER hScreenTimer;
static char firsttimethrough;
static GUI_MEMDEV_Handle memGPS;
#endif
static WM_HTIMER hTestScreenTimer;
static WM_HWIN hProfiler;
static char testfirsttimethrough;
static GUI_MEMDEV_Handle memTest;

#define	TOP_LINE			40
#define	COLUMN_ONE			5
#define	COLUMN_TWO			130
#define	COLUMN_THREE		300
#define	COLUMN_FOUR			415
#define	LINE_GAP			30

/*********************************************************************
*
*       GPS Info Screen Callback
*/
void ProfilerCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hTestScreenTimer)
		{
			WM_RestartTimer(hTestScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hProfiler);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(PROFILER_SCREEN) || testfirsttimethrough)
		{
			char scratch[40];
			char i;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			testfirsttimethrough = FALSE;

			GUI_MEMDEV_Select(memTest);
			GUI_Clear();
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font24_ASCII);

			// Draw screen title
			GUI_DispStringHCenterAt("Task Profiler", XSIZE_PHYS/2, 5);

			for (i = 0; i < NUMBER_OF_TASKS; i++)
			{
				sprintf(scratch, "%s", taskProfile[i].profileName);
				GUI_DispStringAt(scratch, COLUMN_ONE, TOP_LINE + (LINE_GAP * i));

				sprintf(scratch, "Starts: %d", taskProfile[i].numberOfStarts);
				GUI_DispStringAt(scratch, COLUMN_TWO, TOP_LINE + (LINE_GAP * i));

				sprintf(scratch, "Max: %d", taskProfile[i].maxExecTime);
				GUI_DispStringAt(scratch, COLUMN_THREE, TOP_LINE + (LINE_GAP * i));

#ifdef _WINDOWS
				sprintf(scratch, "Total: %d", taskProfile[i].totalExecTime);
#else
				{
					long currentTime;
					int percent;

					currentTime = OS_GetTime32();
					percent = (int) (((float)taskProfile[i].totalExecTime / (float)currentTime) * 100.0);
					sprintf(scratch, "%%: %d", percent);
				}
#endif
				GUI_DispStringAt(scratch, COLUMN_FOUR, TOP_LINE + (LINE_GAP * i));
			}

#if 0
			// Draw Light Level
			sprintf(scratch, "Raw Light Level: %d       ", GetADC(ADC_LIGHT_LEVEL));
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE);

			// Draw LED Board Temperature
			sprintf(scratch, "LED Temp: %d   ", GetADC(ADC_LED_BOARD_TEMP));
			GUI_DispStringAt(scratch, LEFT_COLUMN, TOP_LINE + (LINE_GAP * 1));

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
			sprintf(scratch, "Battery voltage: %2.1f   ", GetValue(VEHICLE_12V).fVal));
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

			//// Draw X-G
			//// If SU HW Rev < 200 display temp else display Z
			//if (suData.su_hw_rev < 200)
			//	sprintf(scratch, "Temp: %.1f %s   ", GetValue(SU_TEMPERATURE).fVal, sysData.units == METRIC ? "C" : "F");
			//else sprintf(scratch, "Z - G: %.2f   ", GetValue(Z_G).fVal);
			//GUI_DispStringAt(scratch, RIGHT_COLUMN, TOP_LINE + (LINE_GAP * 4));
#endif

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
WM_HWIN DisplayProfilerScreen(WM_HWIN hParent)
{
	testfirsttimethrough = true;

	// create a memory area to reduce flicker on paint.
	memTest = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hProfiler = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, ProfilerCallback, 0)) <= 0)
		SystemError();

	WM_InvalidateWindow(hProfiler);

	hTestScreenTimer = WM_CreateTimer(hProfiler, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hProfiler;
}

/*************************** End of file ****************************/

// End ifdef PROFILER
#endif
