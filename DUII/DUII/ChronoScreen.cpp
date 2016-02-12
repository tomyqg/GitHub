
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
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "ChronoScreen.h"
#include "ChronoGauge.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "PredictiveLapTimer.hpp"
#include "DigitalStringOutput.hpp"
#include "Utilities.h"
#include "LEDHandler.h"


/*********************************************************************
*
*       Local Defines and Enumerations
*
**********************************************************************
*/
#define	VERSION_5

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
extern char lockupSemaphore;
extern GUI_CONST_STORAGE GUI_BITMAP bmBestLapEver;
extern GUI_CONST_STORAGE GUI_BITMAP bmBestLapSession;
#ifdef VERSION_1
extern GUI_CONST_STORAGE GUI_BITMAP bmChronometric_wTrans;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeList;
#elif defined VERSION_2
extern GUI_CONST_STORAGE GUI_BITMAP bmChronometric_NoTrans;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeList;
#elif defined VERSION_3
extern GUI_CONST_STORAGE GUI_BITMAP bmChronometric_original;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeList;
#elif defined VERSION_4
extern GUI_CONST_STORAGE GUI_BITMAP bmChronometric_NoTrans_2;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeList;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowDigits;
#elif defined VERSION_5
extern GUI_CONST_STORAGE GUI_BITMAP bmChronometric_NoTrans_2;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeList;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellowDigits;
static ChronoGauge *pChronoGauge = 0;
static Alarm *pYellow = 0;
static Alarm *pRed = 0;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_0;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_1;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_2;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_3;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_4;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_5;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_6;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_7;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_8;
extern GUI_CONST_STORAGE GUI_BITMAP bmYellow_9;
#endif
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
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hChronoScreen;
static WM_HTIMER hScreenTimer;
//static Gauge *pChrono = 0;
static bool firsttimethrough;
//static PredictiveLapTimer *pPredictiveLap = 0;
//static DigitalStringOutput *pLapTime = 0, *pLap = 0, *pSpeed = 0;
//
//static Alarm *pYellow = 0;
//static Alarm *pRed = 0;
//static Alarm *pGreen = 0;
//

#ifdef	VERSION_2
static GUI_MEMDEV_Handle screen, timeList;
#elif defined	VERSION_3
static GUI_MEMDEV_Handle screen, timeList;
#elif defined	VERSION_4
static GUI_MEMDEV_Handle screen, timeList, yellowDigitsList;
#elif defined	VERSION_5
static GUI_MEMDEV_Handle screen, timeList, yellowDigitsList;
//static GUI_MEMDEV_Handle screen2;
#endif
//
//static const GUI_RECT ElapsedTimeRect = { 0, 155, 209, 220 };
//static const GUI_RECT VmaxRect = { 75, 68, 145, 108 };

int time = 0;
int time2 = 0;
#define	GAUGE_SCREEN_REFRESHES_PER_SECOND	(1000 / GAUGE_SCREEN_REFRESH_TIME)
#define	SIZE_OF_TIME_STRIP_IN_PIXELS		2100
#define	NUMBER_OF_STEPS_ON_SECONDS_STRIP	68
#define	PIXEL_HEIGHT_OF_ONE_SECOND			(float)((float)SIZE_OF_TIME_STRIP_IN_PIXELS / (float)NUMBER_OF_STEPS_ON_SECONDS_STRIP)
#define	PIXEL_STEP_PER_REFRESH	(float)(PIXEL_HEIGHT_OF_ONE_SECOND / (float)GAUGE_SCREEN_REFRESHES_PER_SECOND)
//#define	SIZE_OF_TIMESLIP_LINE		

#ifdef VERSION_5
GUI_CONST_STORAGE GUI_BITMAP *GetBitmapDigit(int digit)
{
	switch (digit)
	{
	default:
	case 0:	return &bmYellow_0;
	case 1:	return &bmYellow_1;
	case 2:	return &bmYellow_2;
	case 3:	return &bmYellow_3;
	case 4:	return &bmYellow_4;
	case 5:	return &bmYellow_5;
	case 6:	return &bmYellow_6;
	case 7:	return &bmYellow_7;
	case 8:	return &bmYellow_8;
	case 9:	return &bmYellow_9;
	}
}
#endif

void _cbChronoCallback(WM_MESSAGE * pMsg)
{
//	int NCode, Id;
int offset;

	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
#ifdef _WINDOWS
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME * 5);
#else
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME * 5);
#endif
			WM_InvalidateWindow(hChronoScreen);
		}
		break;

	case WM_PAINT:

#define	XSIZE	200
#define	YSIZE	120
#define	XSTART_POS	5
#define	YSTART_POS	108

#ifdef	VERSION_1
		if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
			time = 0;
		offset = 0 - (time * PIXEL_STEP_PER_REFRESH);

		GUI_DrawBitmap(&bmTimeList, XSTART_POS, offset);
		GUI_DrawBitmap(&bmChronometric_wTrans, 0, 0);

		if (SlideOutVisible())
			DrawSignalStrength();
#elif defined VERSION_2
		if (CS_IsVisible(CHRONOGRAPH_SCREEN) || firsttimethrough)
		{
			char *pTimeList, *pScreen;
			int x, y, xSize;

			firsttimethrough = FALSE;
			
			pTimeList = (char *)GUI_MEMDEV_GetDataPtr(timeList);
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);

			// Calculate offset into ticker image
			if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time = 0;
			offset = (int)((float)time * PIXEL_STEP_PER_REFRESH);

			// Correct destination address for position of time list on screen
			pScreen += (YSTART_POS * XSIZE_PHYS * 2) + XSTART_POS;

			//xSize = GUI_MEMDEV_GetXSize(timeList);
			xSize = XSIZE;
			pTimeList += offset * xSize;

			// copy visible portion of time ticker tape to the display memory area
			for (y = 0; y < YSIZE; y++)
			{
				for (x = 0; x < xSize; x++)
				{
					*pScreen++ = *pTimeList++;
				}
				pScreen += (XSIZE_PHYS * 2) - xSize;
			}

			if (SlideOutVisible())
				DrawSignalStrength();
	
			GUI_MEMDEV_Write(screen);
			GUI_SelectLCD();
		}
#elif defined VERSION_3
		if (CS_IsVisible(CHRONOGRAPH_SCREEN) || firsttimethrough)
		{
			char *pTimeList, *pScreen;
			int x, y, xSize;

			firsttimethrough = FALSE;
			
			pTimeList = (char *)GUI_MEMDEV_GetDataPtr(timeList);
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);

			// Calculate offset into ticker image
			if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time = 0;
			offset = (int)((float)time * PIXEL_STEP_PER_REFRESH);

			// Put "fresh" image in buffer
			GUI_MEMDEV_Select(screen);
			GUI_DrawBitmap(&bmChronometric_original, 0, 0);
			GUI_MEMDEV_Select(0);

			// Correct destination address for position of time list on screen
			pScreen += (YSTART_POS * XSIZE_PHYS * 2) + XSTART_POS;

			//xSize = GUI_MEMDEV_GetXSize(timeList);
			xSize = XSIZE;
			pTimeList += offset * xSize;

			// copy visible portion of time ticker tape to the display memory area
			for (y = 0; y < YSIZE; y++)
			{
				for (x = 0; x < xSize; x++)
				{
					*pScreen++ &= *pTimeList++;
				}
				pScreen += (XSIZE_PHYS * 2) - xSize;
			}

			if (SlideOutVisible())
				DrawSignalStrength();
	
			GUI_MEMDEV_Write(screen);
			GUI_SelectLCD();
		}
#elif defined VERSION_4
		if (CS_IsVisible(CHRONOGRAPH_SCREEN) || firsttimethrough)
		{
			char *pTimeList, *pScreen;
			char *pYellowDigitsList;
			int x, y, xSize;

			firsttimethrough = FALSE;
			
			pTimeList = (char *)GUI_MEMDEV_GetDataPtr(timeList);
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);
			pYellowDigitsList = (char *)GUI_MEMDEV_GetDataPtr(yellowDigitsList);

			// Calculate offset into ticker image
			if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time = 0;
			offset = (int)((float)time * PIXEL_STEP_PER_REFRESH);

			// Put "fresh" image in buffer
			GUI_MEMDEV_Select(screen);
			GUI_DrawBitmap(&bmChronometric_NoTrans_2, 0, 0);
			GUI_MEMDEV_Select(0);

			// Correct destination address for position of time list on screen
			pScreen += (YSTART_POS * XSIZE_PHYS * 2) + XSTART_POS;

			//xSize = GUI_MEMDEV_GetXSize(timeList);
			xSize = XSIZE;
			pTimeList += offset * xSize;

			// copy visible portion of time ticker tape to the display memory area
			for (y = 0; y < YSIZE; y++)
			{
				for (x = 0; x < xSize; x++)
				{
					*pScreen++ &= *pTimeList++;
				}
				pScreen += (XSIZE_PHYS * 2) - xSize;
			}

			// YELLOW TAPE TICKER
			// Calculate offset into ticker image
			if (time2++ > 10 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time2 = 0;
			offset = (int)((float)time2 * PIXEL_STEP_PER_REFRESH);


			// Correct destination address for position of time list on screen
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);
			pScreen += (11 * XSIZE_PHYS * 2) + (103 * 2);

			// Calculate offset into yellow list
			pYellowDigitsList += offset * 80;

			// copy visible portion of yellow digits tape to the display memory area
			for (y = 0; y < 50; y++)
			{
				for (x = 0; x < (40 * 2); x++)
				{
					*pScreen++ |= *pYellowDigitsList++;
				}
				pScreen += (XSIZE_PHYS * 2) - (40 * 2);
			}

			if (SlideOutVisible())
				DrawSignalStrength();
	
			GUI_MEMDEV_Write(screen);
			GUI_SelectLCD();
		}
#elif defined VERSION_5
//		if (CS_IsVisible(CHRONOGRAPH_SCREEN) || firsttimethrough)
//		{
//			char *pTimeList, *pScreen, *pScreen2;
//			char *pYellowDigitsList;
//			int x, y, xSize;
//
//			firsttimethrough = FALSE;
//			
//			pTimeList = (char *)GUI_MEMDEV_GetDataPtr(timeList);
//			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);
//			pScreen2 = (char *)GUI_MEMDEV_GetDataPtr(screen2);
//			pYellowDigitsList = (char *)GUI_MEMDEV_GetDataPtr(yellowDigitsList);
//
//			// Put "fresh" image in buffer
//			GUI_MEMDEV_Select(screen2);
//			memcpy((void *)pScreen2, (void *)pScreen, XSIZE_PHYS * YSIZE_PHYS * sizeof(int));
//	//		GUI_MEMDEV_Select(0);
//
//			// Correct destination address for position of time list on screen
//			pScreen2 += (YSTART_POS * XSIZE_PHYS * 2) + XSTART_POS;
//
//			// Calculate offset into ticker image
//			if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
//				time = 0;
//			offset = (int)((float)time * PIXEL_STEP_PER_REFRESH);
//			//xSize = GUI_MEMDEV_GetXSize(timeList);
//			xSize = XSIZE;
//			pTimeList += offset * xSize;
//
//			// copy visible portion of time ticker tape to the display memory area
//			for (y = 0; y < YSIZE; y++)
//			{
//				for (x = 0; x < xSize; x++)
//				{
//					*pScreen2++ &= *pTimeList++;
//				}
//				pScreen2 += (XSIZE_PHYS * 2) - xSize;
//			}
//
//			// YELLOW TAPE TICKER
//			// Calculate offset into ticker image
//			if (time2++ > 10 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
//				time2 = 0;
//			offset = (int)((float)time2 * PIXEL_STEP_PER_REFRESH);
//
//			// Correct destination address for position of time list on screen
//			pScreen2 = (char *)GUI_MEMDEV_GetDataPtr(screen2);
//			pScreen2 += (11 * XSIZE_PHYS * 2) + (103 * 2);
//
//			// Calculate offset into yellow list
//			pYellowDigitsList += offset * 80;
//
//			// copy visible portion of yellow digits tape to the display memory area
//			for (y = 0; y < 50; y++)
//			{
//				for (x = 0; x < (40 * 2); x++)
//				{
//					*pScreen2++ |= *pYellowDigitsList++;
//				}
//				pScreen2 += (XSIZE_PHYS * 2) - (40 * 2);
//			}
//	
////			GUI_MEMDEV_Select(screen2);
//
//			if (SlideOutVisible())
//				DrawSignalStrength();
//
//			GUI_MEMDEV_Write(screen2);
//			GUI_SelectLCD();
//		}
		if (CS_IsVisible(CHRONOGRAPH_SCREEN) || firsttimethrough)
		{
			char *pTimeList, *pScreen;
			char *pYellowDigitsList;
			int x, y, xSize;
			GUI_MEMDEV_Handle mControl;

			firsttimethrough = FALSE;
			
			pTimeList = (char *)GUI_MEMDEV_GetDataPtr(timeList);
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);
			pYellowDigitsList = (char *)GUI_MEMDEV_GetDataPtr(yellowDigitsList);

			// Calculate offset into ticker image
			if (time++ > 60 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time = 0;
			offset = (int)((float)time * PIXEL_STEP_PER_REFRESH);

			// Put "fresh" image in buffer
			GUI_MEMDEV_Select(screen);
			GUI_DrawBitmap(&bmChronometric_NoTrans_2, 0, 0);
			GUI_MEMDEV_Select(0);


			// Update tach
			pChronoGauge->SetValue(GetValue(VEHICLE_RPM).iVal);
			mControl = pChronoGauge->DrawToMemArea();
			GUI_MEMDEV_Select(screen);
			GUI_MEMDEV_Write(mControl);


			// Correct destination address for position of time list on screen
			pScreen += (YSTART_POS * XSIZE_PHYS * 2) + XSTART_POS;

			//xSize = GUI_MEMDEV_GetXSize(timeList);
			xSize = XSIZE;
			pTimeList += offset * xSize;

			// copy visible portion of time ticker tape to the display memory area
			for (y = 0; y < YSIZE; y++)
			{
				for (x = 0; x < xSize; x++)
				{
					*pScreen++ &= *pTimeList++;
				}
				pScreen += (XSIZE_PHYS * 2) - xSize;
			}

			// YELLOW TAPE TICKER
			// Calculate offset into ticker image
			if (time2++ > 10 * GAUGE_SCREEN_REFRESHES_PER_SECOND)
				time2 = 0;
			offset = (int)((float)time2 * PIXEL_STEP_PER_REFRESH);


			// Correct destination address for position of time list on screen
			pScreen = (char *)GUI_MEMDEV_GetDataPtr(screen);
			pScreen += (11 * XSIZE_PHYS * 2) + (103 * 2);

			// Calculate offset into yellow list
			pYellowDigitsList += offset * 80;

			// copy visible portion of yellow digits tape to the display memory area
			for (y = 0; y < 50; y++)
			{
				for (x = 0; x < (40 * 2); x++)
				{
					*pScreen++ |= *pYellowDigitsList++;
				}
				pScreen += (XSIZE_PHYS * 2) - (40 * 2);
			}

			if (SlideOutVisible())
				DrawSignalStrength();
	
			// Draw test bitmap yellow digit
			{
				//GUI_DrawBitmap(GetBitmapDigit((time / 10) % 10), 98, 8);
				GUI_DrawBitmap(GetBitmapDigit((time / 10) % 10), 44, 8);
				GUI_DrawBitmap(GetBitmapDigit((time / 100) % 10), 3, 8);
			}


			GUI_MEMDEV_CopyToLCD(screen);
	//		GUI_MEMDEV_Write(screen);
			GUI_SelectLCD();
		}
#endif
		break;

	case WM_DELETE:
#ifdef	VERSION_2
		GUI_MEMDEV_Delete(screen);
		GUI_MEMDEV_Delete(timeList);
#elif defined	VERSION_3
		GUI_MEMDEV_Delete(screen);
		GUI_MEMDEV_Delete(timeList);
#elif defined	VERSION_4
		GUI_MEMDEV_Delete(screen);
		GUI_MEMDEV_Delete(timeList);
		GUI_MEMDEV_Delete(yellowDigitsList);
#elif defined	VERSION_5
		//GUI_MEMDEV_Delete(screen);
		//GUI_MEMDEV_Delete(screen2);
		//GUI_MEMDEV_Delete(timeList);
		//GUI_MEMDEV_Delete(yellowDigitsList);
		GUI_MEMDEV_Delete(screen);
		GUI_MEMDEV_Delete(timeList);
		GUI_MEMDEV_Delete(yellowDigitsList);

		if (pYellow)
		{
			delete pYellow;
			pYellow = 0;
		}
		if (pRed)
		{
			delete pRed;
			pRed = 0;
		}
		if (pChronoGauge)
		{
			delete pChronoGauge;
			pChronoGauge = 0;
		}
#endif
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}
//#define	LAP_TIME_RIGHT	210

/*********************************************************************
*
*       Chrono Screen
*/
WM_HWIN DisplayChronoScreen(WM_HWIN hParent)
{
	//if (vehicleData.tach.greenStart != 0)
	//	pGreen = new Alarm(true, vehicleData.tach.greenStart, vehicleData.tach.upperYellowStart, GUI_GREEN, GUI_DARKGREEN, NORMAL, NULL, NULL);
	//else pGreen = 0;
#ifdef VERSION_5
	pYellow = new Alarm(true, vehicleData.tach.upperYellowStart, vehicleData.tach.upperRedStart, GUI_YELLOW, GUI_DARKYELLOW, NORMAL, NULL, NULL);
	pRed = new Alarm(true, vehicleData.tach.upperRedStart, CalculateScaleTop(vehicleData.tach.scaleEnd), GUI_RED, GUI_DARKRED, HIGH, pYellow, NULL);
#endif
	firsttimethrough = true;

#ifdef	VERSION_1
	if ((hChronoScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW | WM_CF_MEMDEV, _cbChronoCallback, 0)) <= 0)
#elif defined VERSION_2
	if ((hChronoScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbChronoCallback, 0)) <= 0)
#elif defined VERSION_3
	if ((hChronoScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbChronoCallback, 0)) <= 0)
#elif defined VERSION_4
	if ((hChronoScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbChronoCallback, 0)) <= 0)
#elif defined VERSION_5
	if ((hChronoScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbChronoCallback, 0)) <= 0)
		SystemError();

	if ((pChronoGauge = new ChronoGauge(145, 0, 136, 0, CalculateScaleTop(vehicleData.tach.scaleEnd), "", "", "", pRed, 1000, 500, 0, 1000, false, hChronoScreen, &bmDefaultGauge)) <= 0)
#endif
		SystemError();

	//if (selectedRaceMode == ROADRACE_MODE)
	//{
	//	if ((pSpeed = new DigitalStringOutput(50, 5, 140, 70, Left, hChronoScreen, GUI_FontD48, GUI_WHITE)) <= 0)
	//		SystemError();

	//	if ((pLap = new DigitalStringOutput(80, 113, 65, 40, Left, hChronoScreen, GUI_FontD32, GUI_WHITE)) <= 0)
	//		SystemError();

	//	if ((pLapTime = new DigitalStringOutput(0, 160, 210, 50, Left, hChronoScreen, GUI_FontD48, GUI_WHITE)) <= 0)
	//		SystemError();

	//	if ((pPredictiveLap = new PredictiveLapTimer(30, 220, 207, 50, GetPredictiveScale(), -GetPredictiveScale(), hChronoScreen)) <= 0)
	//		SystemError();
	//}
	//else if ((pSpeed = new DigitalStringOutput(50, 10, 140, 70, Left, hChronoScreen, GUI_FontD48, GUI_WHITE)) <= 0)
	//	SystemError();

	//WM_InvalidateWindow(hChronoScreen);

	hScreenTimer = WM_CreateTimer(hChronoScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	// create a memory area to hold the lap time strip

	//mem = GUI_MEMDEV_Create(0, 0, 100, SIZE_OF_TIME_STRIP_IN_PIXELS);
#ifdef	VERSION_2
	timeList = GUI_MEMDEV_Create(0, 0, 100, 2100);
	GUI_MEMDEV_Select(timeList);
	GUI_DrawBitmap(&bmTimeList, 0, 0);

	screen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	GUI_MEMDEV_Select(screen);
	GUI_DrawBitmap(&bmChronometric_NoTrans, 0, 0);

	GUI_MEMDEV_Select(0);
#elif defined	VERSION_3
	timeList = GUI_MEMDEV_Create(0, 0, 100, 2100);
	GUI_MEMDEV_Select(timeList);
	GUI_DrawBitmap(&bmTimeList, 0, 0);

	screen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	//GUI_MEMDEV_Select(screen);
	//GUI_DrawBitmap(&bmChronometric_NoTrans, 0, 0);

	GUI_MEMDEV_Select(0);
#elif defined	VERSION_4
	timeList = GUI_MEMDEV_Create(0, 0, 100, 2100);
	GUI_MEMDEV_Select(timeList);
	GUI_DrawBitmap(&bmTimeList, 0, 0);

	yellowDigitsList = GUI_MEMDEV_Create(0, 0, 40, 442);
	GUI_MEMDEV_Select(yellowDigitsList);
	GUI_DrawBitmap(&bmYellowDigits, 0, 0);

	screen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	//GUI_MEMDEV_Select(screen);
	//GUI_DrawBitmap(&bmChronometric_NoTrans_2, 0, 0);

	GUI_MEMDEV_Select(0);
#elif defined	VERSION_5
	//timeList = GUI_MEMDEV_Create(0, 0, 100, 2100);
	//GUI_MEMDEV_Select(timeList);
	//GUI_DrawBitmap(&bmTimeList, 0, 0);

	//yellowDigitsList = GUI_MEMDEV_Create(0, 0, 40, 442);
	//GUI_MEMDEV_Select(yellowDigitsList);
	//GUI_DrawBitmap(&bmYellowDigits, 0, 0);

	//screen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	//GUI_MEMDEV_Select(screen);
	//GUI_DrawBitmap(&bmChronometric_NoTrans_2, 0, 0);

	//screen2 = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	//GUI_MEMDEV_Select(0);
	timeList = GUI_MEMDEV_Create(0, 0, 100, 2100);
	GUI_MEMDEV_Select(timeList);
	GUI_DrawBitmap(&bmTimeList, 0, 0);

	yellowDigitsList = GUI_MEMDEV_Create(0, 0, 40, 442);
	GUI_MEMDEV_Select(yellowDigitsList);
	GUI_DrawBitmap(&bmYellowDigits, 0, 0);

	screen = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);
	//GUI_MEMDEV_Select(screen);
	//GUI_DrawBitmap(&bmChronometric_NoTrans_2, 0, 0);

	GUI_MEMDEV_Select(0);
#endif

	return hChronoScreen;
}

/*************************** End of file ****************************/
