
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
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "GMeterScreen.h"
#include "GMeter.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "ConfigurableGaugeCommon.h"
#include "LEDHandler.h"


/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
extern char lockupSemaphore;
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
void GMDrawConfigRect(void);


/*********************************************************************
*
*		Public Variables
*
**********************************************************************
*/
WM_HWIN hGMeterScreen;

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HTIMER hScreenTimer;
GMeter *pGM = 0;
static bool firsttimethrough;
int errorCounter;
GUI_MEMDEV_Handle memDev;

/*********************************************************************
*
*       Local Defines
*
**********************************************************************
*/
#define	CONFIGURABLE_COLOR	GUI_ORANGE
const GUI_RECT configurableItemRect = { 0, 0, 140, 88 };

//const GUI_RECT DataElementRect = { 0, 8,  150, 40 };
//const GUI_RECT NameRect =		 { 0, 41, 120, 64 };
//const GUI_RECT UnitsRect =		 { 0, 64, 120, 88 };
const GUI_RECT DataElementRect = { 0, 5,  150, 37 };
const GUI_RECT NameRect =		 { 0, 38, 120, 61 };
const GUI_RECT UnitsRect =		 { 0, 61, 120, 85 };

static const DISPLAY_ELEMENT pData =	{ DataElementRect,	GUI_TA_LEFT,	CONFIGURABLE_COLOR,	&GUI_FontD32 };
static const DISPLAY_ELEMENT pName =	{ NameRect,			GUI_TA_LEFT,	GUI_LIGHTGRAY,		&GUI_Font24B_ASCII };
static const DISPLAY_ELEMENT pUnits =	{ UnitsRect,		GUI_TA_LEFT,	GUI_LIGHTGRAY,		&GUI_Font24B_ASCII };

const CONFIG_ARGS config = { (DISPLAY_ELEMENT *)&pData, (DISPLAY_ELEMENT *)&pName, (DISPLAY_ELEMENT *)&pUnits };


void _cbGMeterCallback(WM_MESSAGE * pMsg) {

	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, G_METER_SCREEN_REFRESH_TIME);

			// Check "box timer" if running
			if (drawFlag)
			{
				if ((GUI_GetTime() - boxStartTime) > BOX_PERSIST_TIME)
				{
					configureGaugeMode = false;
					drawFlag = false;
				}
			}

			// Try setting value here to split processing load
			pGM->SetValue(GetValue(X_G_DAMPED).fVal, GetValue(Y_G_DAMPED).fVal);

			WM_InvalidateWindow(hGMeterScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(GMETER) || firsttimethrough)
		{
			firsttimethrough = false;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
			{
				errorCounter++;
				break;
			}
			else lockupSemaphore = true;
			
			GUI_MEMDEV_Select(memDev);

			pGM->DrawControl();

			// Display the selected configurable gauge item
			DisplayConfigurableItem( &config );

			// Draw box around programmable gauge member(s) if requested
#ifdef	INCLUDE_RACE_HELP
			if (drawFlag || raceTimerActive)
#else
			if (drawFlag)
#endif
				GMDrawConfigRect();

			if (SlideOutVisible())
				DrawSignalStrength();

			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(memDev);
			GUI_SelectLCD();
		}
		break;

	case WM_DELETE:
		if (pGM)
		{
			delete pGM;
			pGM = 0;
			GUI_MEMDEV_Delete(memDev);
		}
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

//
// Draw rectangle around configurable item
//
void GMDrawConfigRect(void)
{
	GUI_SetColor(GUI_RED);
	GUI_DrawRect(configurableItemRect.x0, configurableItemRect.y0, configurableItemRect.x1, configurableItemRect.y1);
	// Draw second rect one pixel inside first to make box two pixels wide
	GUI_DrawRect(configurableItemRect.x0 + 1, configurableItemRect.y0 + 1, configurableItemRect.x1 - 1, configurableItemRect.y1 - 1);
}

/*********************************************************************
*
*       G-Meter Screen
*/

WM_HWIN Display_GMeterScreen(WM_HWIN hParent)
{
	firsttimethrough = true;
	errorCounter = 0;

	memDev = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hGMeterScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbGMeterCallback, 0)) <= 0)
		SystemError();

	// KMC NOTE - Create the GMeter BEFORE creating the window.  The callback function is called DURING the CreateWindow call
	// so the GMeter has to be ready to start receiving messages
	if ((pGM = new GMeter(2)) <= 0)
		SystemError();

	hScreenTimer = WM_CreateTimer(hGMeterScreen, 0, G_METER_SCREEN_REFRESH_TIME, 0);

	return(hGMeterScreen);
}

//
// Return true if screen touch in on the configurable gauge item
//
char IfGMeterConfigGaugePressed(int lastx, int lasty)
{
	if ((lastx >= configurableItemRect.x0) && (lastx <= configurableItemRect.x1) && (lasty >= configurableItemRect.y0) && (lasty <= configurableItemRect.y1))
		return true;
	else return false;
}

/*************************** End of file ****************************/

