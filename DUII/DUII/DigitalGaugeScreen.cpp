
#include <stdio.h>
#include <stdlib.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "DigitalGaugeScreen.h"
#include "DigitalDisplay.hpp"
#include "RunningGauges.h"
#include "Alarm.hpp"
#include "gauge.hpp"
#include <time.h>
#include "DataHandler.h"
#include "PredictiveLapTimer.hpp"
#include "DigitalStringOutput.hpp"
#include "Utilities.h"
#include "LEDHandler.h"
#include "ConfigurableGaugeCommon.h"

#ifdef G_DEBUG
#include <windows.h>
#endif

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
extern GUI_CONST_STORAGE GUI_BITMAP bmDarkAsphalt;
extern GUI_CONST_STORAGE GUI_BITMAP bmBestLapSession;
}
#endif
extern void UpdatePredictiveLapControl(PredictiveLapTimer *pPredictiveLap, float predictiveLap);

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Constants
*
**********************************************************************
*/
#define END_OF_LAP			80
#define	COLOR_BAR_X_SIZE	150
#define	COLOR_BAR_Y_SIZE	3
#define	TROPHY_X_POS		100

#define	LAP_COLOR			GUI_CYAN
#define	CONFIGURABLE_COLOR	GUI_ORANGE
#define	ELAPSED_COLOR		GUI_YELLOW

const GUI_RECT LapNumRect = { 5, 10, END_OF_LAP, 42 };
const GUI_RECT ElapsedTimeRect = { END_OF_LAP + 5, 10, 295, 42 };
const GUI_RECT configurableItemRect = { 310, 10, 470, 95 };

const GUI_RECT LapTimeRect = { 35, 110, 400, 200 };

const GUI_RECT DataElementRect = { 310, 10, 470, 42 };
const GUI_RECT NameRect =		 { 310, 47, 470, 71 };
const GUI_RECT UnitsRect =		 { 310, 71, 470, 95 };

static const DISPLAY_ELEMENT pData =	{ DataElementRect,	GUI_TA_RIGHT,	CONFIGURABLE_COLOR,	&GUI_FontD32 };
static const DISPLAY_ELEMENT pName =	{ NameRect,			GUI_TA_RIGHT,	GUI_WHITE,			&GUI_Font24B_ASCII };
static const DISPLAY_ELEMENT pUnits =	{ UnitsRect,		GUI_TA_RIGHT,	GUI_WHITE,			&GUI_Font24B_ASCII };

const CONFIG_ARGS config = { (DISPLAY_ELEMENT *)&pData, (DISPLAY_ELEMENT *)&pName, (DISPLAY_ELEMENT *)&pUnits };

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hDigitalGaugeScreen;
static WM_HTIMER hScreenTimer;
static PredictiveLapTimer *pPredictiveLap = 0;
static GUI_MEMDEV_Handle mem;
static bool firsttimethrough;

void DGDrawConfigRect(void);


void _cbDigitalGaugeCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDigitalGaugeScreen);

			// Check "box timer" if running
			if (drawFlag)
			{
				if ((GUI_GetTime() - boxStartTime) > BOX_PERSIST_TIME)
				{
					configureGaugeMode = false;
					drawFlag = false;
				}
			}
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(DIGITAL_GAUGE) || firsttimethrough)
		{
			////GUI_MEMDEV_Handle mControl;
			////int lap;
			char scratch[20];

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			firsttimethrough = FALSE;

			GUI_MEMDEV_Select(mem);

			// Draw background
			GUI_DrawBitmap(&bmDarkAsphalt, 0, 0);

			// Set overall text mode
			GUI_SetTextMode(GUI_TM_TRANS);

			// Display lap number
			GUI_SetColor(LAP_COLOR);
			GUI_SetFont(&GUI_FontD32);
			sprintf(scratch, "%d", /* lap = */ GetValue(LAP_NUMBER).iVal);
			GUI_DispStringInRect(scratch, (GUI_RECT *)&LapNumRect, GUI_TA_LEFT);

			// Draw lap color bar
			GUI_FillRect(0, 0, COLOR_BAR_X_SIZE, COLOR_BAR_Y_SIZE);

			// Display elapsed time
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringInRect(formatElapsedTime(), (GUI_RECT *)&ElapsedTimeRect, GUI_TA_RIGHT);

			// Draw configurable item color bar
			GUI_SetColor(CONFIGURABLE_COLOR);
			GUI_FillRect(XSIZE_PHYS - COLOR_BAR_X_SIZE, 0, XSIZE_PHYS, COLOR_BAR_Y_SIZE);

			// Display the selected configurable gauge item
			DisplayConfigurableItem( &config );

			// Display fixed text
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_Font24B_ASCII);
			GUI_DispStringAt("Lap", 5, 45);

			GUI_SetColor(ELAPSED_COLOR);
			GUI_DispStringHCenterAt("elapsed", XSIZE_PHYS / 2, 45);

			// Draw best lap icon
			if (GetValue(HOLDING).uVal)		// If currently in lap hold time
				switch (DataValues[BEST_LAP].uVal) {
					case 0:
						// undraw icon when lap hold time expires
						break;
					case 1:
						// draw best lap of session icon (checker)
						GUI_DrawBitmap(&bmBestLapSession, TROPHY_X_POS, 10);
						break;
					case 2:
						// draw best lap ever icon (gold cup)
						GUI_DrawBitmap(&bmBestLapEver, TROPHY_X_POS, 10);
						break;
				} // switch

			// Draw Lap Time
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_FontD32);
			GUI_SetFont(&GUI_FontD80);
			GUI_DispStringInRect(formatLapTime(GetValue(LAP_TIME).uVal, false), (GUI_RECT *)&LapTimeRect, GUI_TA_RIGHT);

			// Update Predictive Lap Control
			// During the first lap write the name of the track in the predictive lap box.
			////if (lap == 0)
			////	pPredictiveLap->SetValue(sysData.track);
			////else pPredictiveLap->SetValue(GetValue(PREDICTIVE_LAP).fVal);
			////	mControl = pPredictiveLap->DrawToMemArea();
			////GUI_MEMDEV_Select(mem);
			////GUI_MEMDEV_Write(mControl);
			UpdatePredictiveLapControl(pPredictiveLap, GetValue(PREDICTIVE_LAP).fVal);

			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
			{
				CheckRaceHelpScreen(DIGITAL_GAUGE);
				DGDrawConfigRect();
			}
#endif

			CheckWarningPopups();

			// Draw box around programmable gauge member(s) if requested
			if (drawFlag)
				DGDrawConfigRect();

			GUI_MEMDEV_CopyToLCD(mem);
			GUI_SelectLCD();
		}

		if (SlideOutVisible())
			DrawSignalStrength();

		break;

	case WM_DELETE:
		if(pPredictiveLap)
		{
			delete pPredictiveLap;
			pPredictiveLap = 0;
		}
		if (mem)
		{
			GUI_MEMDEV_Delete(mem);
			mem = 0;
		}
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

void DGDrawConfigRect(void)
{
	GUI_SetColor(GUI_RED);
	GUI_DrawRect(configurableItemRect.x0 - 4, configurableItemRect.y0 - 4, configurableItemRect.x1 + 4, configurableItemRect.y1);
	// Draw second rect one pixel inside first to make box two pixels wide
	GUI_DrawRect(configurableItemRect.x0 - 3, configurableItemRect.y0 - 3, configurableItemRect.x1 + 3, configurableItemRect.y1 - 1);
}

WM_HWIN Display_DigitalGaugeScreen(WM_HWIN hParent)
{
	firsttimethrough = true;
	drawFlag = false;

	if ((hDigitalGaugeScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbDigitalGaugeCallback, 0)) <= 0)
		SystemError();

	if ((pPredictiveLap = new PredictiveLapTimer(50, 212, 400, 50, GetPredictiveScale(), -GetPredictiveScale(), hDigitalGaugeScreen)) <= 0)
		SystemError();

#ifdef G_DEBUG
	pOutput = fopen("Output.txt", "w+");
	iCount = 0;
#endif

	WM_InvalidateWindow(hDigitalGaugeScreen);

	// create the memory area to prevent flicker
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	hScreenTimer = WM_CreateTimer(hDigitalGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDigitalGaugeScreen;
}

//
// Return true if screen touch in on the configurable gauge item
//
char IfDigitalConfigGaugePressed(int lastx, int lasty)
{
	if ((lastx >= configurableItemRect.x0) && (lastx <= configurableItemRect.x1) && (lasty >= configurableItemRect.y0) && (lasty <= configurableItemRect.y1))
		return true;
	else return false;
}
/*************************** End of file ****************************/

