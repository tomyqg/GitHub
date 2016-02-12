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
#include "DualGaugeScreen.h"
#include "RunningGauges.h"
#include "DigitalGaugeScreen.h"
#include "DriveScreen.h"
#include "RPMSweepScreen.h"
#include "GPSInfoScreen.h"
#include "LinearGaugeScreen.h"
#include "ChronoScreen.h"
#ifdef PROFILER
#include "ProfilerScreen.h"
#endif


/*********************************************************************
*
*       Public Data
*
**********************************************************************
*/

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmfourwayarrow;
extern int screen_scrolling;

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
static WM_HWIN hRunningGauges;
static WM_HWIN peekingScreenHandle = 0;
static int peekingScreenID;


#define	NOT_MOVING	0
#define	MOVING_IN_X	1
#define	MOVING_IN_Y	2

struct GAUGE_SCREEN_LIST GaugesScreenList[NUM_GAUGE_SCREENS+1] = {
	{ ANALOG_GAUGE, Display_GaugeScreen },
	{ RPM_SWEEP_GAUGE, Display_RPMSweepScreen },				// default - index = 1 (set in SetDefaultSystemData)
	{ MULTI_ANALOG_GAUGES, DisplayMultiAnalogGaugesScreen },
	{ DIGITAL_GAUGE, Display_DigitalGaugeScreen },
	{ GMETER, Display_GMeterScreen },
	{ TEST_INFO_SCREEN, DisplayTestInfo },
#ifdef PROFILER
	{ PROFILER_SCREEN, DisplayProfilerScreen },
#endif
	{ -1, NULL }
};

struct GAUGE_SCREEN_LIST DriveScreenList[NUM_DRIVE_SCREENS+1] = {
	{ GMETER, Display_GMeterScreen },
	{ ANALOG_GAUGE, Display_GaugeScreen },
	{ DRIVE_MODE_SCREEN, Display_DriveScreen },					// default - index = 2 (set in SetDefaultSystemData)
	{ RPM_SWEEP_GAUGE, Display_RPMSweepScreen },
	{ MULTI_ANALOG_GAUGES, DisplayMultiAnalogGaugesScreen },
	{ GPS_INFO_SCREEN, DisplayGPSInfo },
//	{ CHRONOGRAPH_SCREEN, DisplayChronoScreen },
	{ TEST_INFO_SCREEN, DisplayTestInfo },
#ifdef PROFILER
	{ PROFILER_SCREEN, DisplayProfilerScreen },
#endif
	{ -1, NULL }
};


struct GAUGE_SCREEN_LIST InstrumentsScreenList[NUM_INSTRUMENTS_SCREENS+1] = {
	{ GMETER, Display_GMeterScreen },
	{ ANALOG_GAUGE, Display_GaugeScreen },						// default - index = 1 (set in SetDefaultSystemData)
	{ DRIVE_MODE_SCREEN, Display_DriveScreen },
	{ RPM_SWEEP_GAUGE, Display_RPMSweepScreen },
	{ MULTI_ANALOG_GAUGES, DisplayMultiAnalogGaugesScreen },
	{ GPS_INFO_SCREEN, DisplayGPSInfo },
//	{ CHRONOGRAPH_SCREEN, DisplayChronoScreen },
	{ TEST_INFO_SCREEN, DisplayTestInfo },
#ifdef PROFILER
	{ PROFILER_SCREEN, DisplayProfilerScreen },
#endif
	{ -1, NULL }
};



WM_HWIN GetRunningGaugesWindowHandle(void)
{
	return hRunningGauges;
}

void SetRunningGaugesWindowHandle(WM_HWIN hWin)
{
	hRunningGauges = hWin;
}


void GaugesScreensCallback(WM_MESSAGE * pMsg)
{
	if (pMsg->MsgId == WM_TIMER)
	{
		if (!CheckSlideoutTimer(pMsg->Data.v))
			WM_DefaultProc(pMsg);
	}
	else WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*/
void RunningGauges(int screen, WM_HWIN hPrev)
{
	struct GAUGE_SCREEN_LIST *pScreenList;

	if (hPrev)
		WM_DeleteWindow(hPrev);

	if (peekingScreenHandle)
	{
	  WM_DeleteWindow(peekingScreenHandle);
	  peekingScreenHandle = 0;
	}

	GUI_Clear();

	if (screen == -1)
		screen = 0;

	if (selectedRaceMode == DRIVE_MODE)
	{
		SetLastScreen(DRIVE_MODE_SCREEN_TYPE, screen);
		pScreenList = DriveScreenList;
	}
	else if (selectedRaceMode == INSTRUMENTS_MODE)
	{
		SetLastScreen(INSTRUMENTS_MODE_SCREEN_TYPE, screen);
		pScreenList = InstrumentsScreenList;
	}
	else
	{
		SetLastScreen(GAUGES_SCREEN_TYPE, screen);
		pScreenList = GaugesScreenList;
	}

	//
	// NOTE: WHEN CREATING THIS WINDOW AS A WINDOW INSTEAD OF JUST DRAWING THE CONTROLS TO
	// THE DESKTOP THE BOTTONS WILL SCROLL WHEN PRESSED IF THEIR PARENTS ARE SET TO 0.
	// SETTING THE PARENTS TO THIS WINDOW STOPS THE SCROLLING MOTION WHEN THE BUTTONS ARE PRESSED.
	//
	if ((hRunningGauges = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, GaugesScreensCallback, 0)) <= 0)
		SystemError();

	// Display requested screen
	pScreenList[screen].constructor(hRunningGauges);

	screen_scrolling = ACTIVE;

}

void rgDeleteScreen(int index)
{
  	if (GetCurrentScreen() != peekingScreenID)
		if (peekingScreenHandle)
		  WM_DeleteWindow(peekingScreenHandle);

	peekingScreenID = -1;
	peekingScreenHandle = 0;
}



