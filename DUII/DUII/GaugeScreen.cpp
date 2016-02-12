
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
#include "GaugeScreen.h"
#include "gauge.hpp"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "PredictiveLapTimer.hpp"
#include "DigitalStringOutput.hpp"
#include "Utilities.h"
#include "LEDHandler.h"
#include "ConfigurableGaugeCommon.h"

/*********************************************************************
*
*       Local Defines and Enumerations
*
**********************************************************************
*/
#define	SPEED_COLOR			GUI_GREEN
#define	CONFIG_COLOR		GUI_ORANGE
#define	LAP_COLOR			GUI_CYAN

#define	BOLD_LINE_WIDTH		2

#define	SPEED_LINE_START	49
#define	CONFIG_LINE_START	60
#define	TOP_OF_CONFIG_BOX	(CONFIG_LINE_START + 41)
#define	LAP_LINE_START		150
#define	SPEED_Y				10
#define	DIGIT_COLUMN_X		90
#define	LAP_Y				113
#define	GAUGE_START			208
#define	DIGIT_HEIGHT		32

static const GUI_RECT ElapsedTimeRect = { 0, 155, 209, 220 };
const GUI_RECT AnalogGaugeEditRect = { 0, CONFIG_LINE_START, 200, CONFIG_LINE_START + 40 };

const GUI_RECT SpeedRect =       {  90,  10, 200, 10 + DIGIT_HEIGHT };
const GUI_RECT DataElementRect = {  50,  61, 200, 61 + DIGIT_HEIGHT };
const GUI_RECT LapRect =         {  90, 111, 200, 111 + DIGIT_HEIGHT };

const GUI_RECT NameRect =  { 5, 55, 112,  79 };
const GUI_RECT UnitsRect = { 5, 77, 112, 101 };

static const DISPLAY_ELEMENT pData =	{ DataElementRect,	GUI_TA_RIGHT,	CONFIG_COLOR,	&GUI_FontD32 };
static const DISPLAY_ELEMENT pName =	{ NameRect,			GUI_TA_LEFT,	CONFIG_COLOR,	&GUI_Font24B_ASCII };
static const DISPLAY_ELEMENT pUnits =	{ UnitsRect,		GUI_TA_LEFT,	CONFIG_COLOR,	&GUI_Font24B_ASCII };

const CONFIG_ARGS config = { (DISPLAY_ELEMENT *)&pData, (DISPLAY_ELEMENT *)&pName, (DISPLAY_ELEMENT *)&pUnits };


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
	extern GUI_CONST_STORAGE GUI_BITMAP bmLeftHalfDarkAsphalt;
	extern GUI_CONST_STORAGE GUI_BITMAP bmDefaultGauge;
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
*       Local Prototypes
*
**********************************************************************
*/
void GSDrawConfigrect(void);

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hGaugeScreen;
static WM_HTIMER hScreenTimer;
static Gauge *pGauge = 0;
static bool firsttimethrough;
static PredictiveLapTimer *pPredictiveLap = 0;
static float predictiveLap;
static DigitalStringOutput *pLapTime = 0;
static DigitalStringOutput* pSpeed = 0;

static Alarm *pYellow = 0;
static Alarm *pRed = 0;
#ifdef	SHOW_GREEN_LINE
static Alarm *pGreen = 0;
#endif

static GUI_MEMDEV_Handle mem;

char speedStr[20], maxRPMStr[20], lapStr[10], currentRPMStr[10], gearStr[5], elapsedStr[20];
int gsHolding, maxRPM;

void _cbGaugeCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			int rpm, gear;

			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hGaugeScreen);

			// Check "box timer" if running
			if (drawFlag)
			{
				if ((GUI_GetTime() - boxStartTime) > BOX_PERSIST_TIME)
				{
					configureGaugeMode = false;
					drawFlag = false;
				}
			}

			// Update the values and set strings in advance to spread processor load
			sprintf(speedStr, "%d", (int)GetValue(SPEED).fVal);

			pGauge->SetValue(rpm = GetValue(VEHICLE_RPM).iVal);
			sprintf(currentRPMStr, "%u", rpm);

			gear = GetValue(GEAR).uVal;
			if ((vehicleData.diffRatio == 0.0) || (0 >= gear))
				sprintf(gearStr, "  ");
			else sprintf(gearStr, "%u ", gear);

			if (selectedRaceMode == ROADRACE_MODE)
			{
				//////int lap;

				sprintf(lapStr, "%u", /*lap =*/ GetValue(LAP_NUMBER).iVal);
				// Update Predictive Lap Control
				//////// During the first lap write the name of the track in the predictive lap box.
				//////if (lap == 0)
				//////	pPredictiveLap->SetValue(sysData.track);
				//////else pPredictiveLap->SetValue(GetValue(PREDICTIVE_LAP).fVal);
				predictiveLap = GetValue(PREDICTIVE_LAP).fVal;

				pLapTime->SetValue(formatLapTime(GetValue(LAP_TIME).uVal, false));
				gsHolding = GetValue(HOLDING).uVal;
			}
			else
			{
				int maxRPM, time;

				maxRPM = GetValue(MAX_RPM).uVal;
				sprintf(maxRPMStr, "%u", maxRPM);

				time = DataValues[ELAPSED_TIME].uVal / 1000;
				if (time < 3600)
					sprintf(elapsedStr, "%02d:%02d", time / 60, time % 60);
				else if (time < 36000)
					sprintf(elapsedStr, "%d:%02d:%02d", time/3600, (time/60)%60, time % 60);
				else 		// > 10 hours
					sprintf(elapsedStr, "%d:%02d", time/3600, (time/60)%60);
			}
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(ANALOG_GAUGE) || firsttimethrough)
		{
			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			firsttimethrough = FALSE;

			GUI_MEMDEV_Select(mem);

#if 1
			GUI_DrawBitmap(&bmLeftHalfDarkAsphalt, 0, 0);
#else
			GUI_ClearRect(0, 0, GAUGE_START - 1, YSIZE_PHYS);
#endif
			GUI_SetTextMode(GUI_TEXTMODE_TRANS);			// Now have background image - make text mode transparent


			// Draw speed units
			GUI_SetColor(SPEED_COLOR);
			GUI_SetFont(&GUI_Font24B_ASCII);
			GUI_DispStringAt((sysData.units == STD ? "mph" : "kph"), 5, 20);
			GUI_FillRect(0, SPEED_LINE_START, 200, SPEED_LINE_START + BOLD_LINE_WIDTH);

			// Draw speed
			GUI_SetFont(&GUI_FontD32);			// Font for digits
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringInRect(speedStr, (GUI_RECT *)&SpeedRect, GUI_TA_RIGHT);

			GUI_SetColor(CONFIG_COLOR);
			GUI_SetFont(&GUI_FontD32);			// Font for digits

			DisplayConfigurableItem( &config );
			GUI_FillRect(0, TOP_OF_CONFIG_BOX, 200, TOP_OF_CONFIG_BOX + BOLD_LINE_WIDTH);

			// Update tach
			pGauge->DrawControl();

			if (selectedRaceMode == ROADRACE_MODE)
			{
				// Display lap number
				GUI_SetColor(LAP_COLOR);
				GUI_SetFont(&GUI_FontD32);			// Font for digits
				GUI_DispStringInRect(lapStr, (GUI_RECT *)&LapRect, GUI_TA_RIGHT);

				//// Draw predictive lap control
				//pPredictiveLap->DrawControl();
				UpdatePredictiveLapControl(pPredictiveLap, predictiveLap);

				// Draw lap time
				pLapTime->DrawControl();

				// Display "Lap" text
				GUI_SetColor(LAP_COLOR);
				GUI_FillRect(0, LAP_LINE_START, 200, LAP_LINE_START + BOLD_LINE_WIDTH);
				GUI_SetFont(&GUI_Font24B_ASCII);
				GUI_DispStringAt("Lap", 5, 123);

				// Draw best lap icon
				if (gsHolding)		// If currently in lap hold time
					switch (DataValues[BEST_LAP].uVal) {
						case 0:
							// undraw icon when lap hold time expires
							break;
						case 1:
							// draw best lap of session icon (checker)
							GUI_DrawBitmap(&bmBestLapSession, 45, 110);
							break;
						case 2:
							// draw best lap ever icon (gold cup)
							GUI_DrawBitmap(&bmBestLapEver, 45, 110);
							break;
					} // switch
			}
			else
			{
				// Draw "session time" text
				GUI_DispStringAt("session time", 40, ElapsedTimeRect.y0 - 30);

				// Draw elapsed time
				GUI_SetFont(&GUI_FontD48);
				GUI_SetColor(GUI_WHITE);
				GUI_DispStringInRect(elapsedStr, (GUI_RECT *)&ElapsedTimeRect, GUI_TA_HCENTER);
			}

			// Display Peak RPM
			if (maxRPM >= vehicleData.tach.scaleEnd)
				GUI_SetColor(GUI_RED);
			else if (maxRPM >= vehicleData.tach.upperRedStart)
				GUI_SetColor(GUI_YELLOW);
			else
				GUI_SetColor(GUI_GRAY);
			GUI_SetFont(&GUI_Font24B_ASCII);
			GUI_DispStringInRect(maxRPMStr, (GUI_RECT *)&PeakRPMRect, GUI_TA_RIGHT);

			// Display current RPM
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringInRect(currentRPMStr, (GUI_RECT *)&RPMRect, GUI_TA_RIGHT);

			// Display Gear text
			GUI_SetFont(&GUI_FontD32);
			GUI_SetColor(GUI_WHITE);
			GUI_DispStringAt(gearStr, 332, 156);

			// Draw box around programmable gauge member(s) if requested
			if (drawFlag)
				GSDrawConfigrect();

			// Draw navigation arrow
			GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

#ifdef	INCLUDE_RACE_HELP
			// If the help screen is still being displayed then show the appropriate help screen
			if (raceTimerActive)
			{
				CheckRaceHelpScreen(ANALOG_GAUGE);
				GSDrawConfigrect();
			}
#endif

			CheckWarningPopups();
			GUI_MEMDEV_CopyToLCD(mem);
			GUI_SelectLCD();

			if (SlideOutVisible())
				DrawSignalStrength();
		}
		break;

	case WM_DELETE:
		if (pGauge)
		{
			delete pGauge;
			pGauge = 0;
		}
#ifdef	SHOW_GREEN_LINE
		if (pGreen)
		{
			delete pGreen;
			pGreen = 0;
		}
#endif
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
		if(pPredictiveLap)
		{
			delete pPredictiveLap;
			pPredictiveLap = 0;
		}
		if(pLapTime)
		{
			delete pLapTime;
			pLapTime = 0;
		}
		if (pSpeed)
		{
			delete pSpeed;
			pSpeed = 0;
		}
		GUI_MEMDEV_Delete(mem);
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

//
// Draw rectangle around configurable item
//
void GSDrawConfigrect(void)
{
	GUI_SetColor(GUI_RED);
	GUI_DrawRect(AnalogGaugeEditRect.x0, AnalogGaugeEditRect.y0 - 5, AnalogGaugeEditRect.x1 + 4, AnalogGaugeEditRect.y1 + 1);		// Start box some pixels above the actual rectangle because
	// the text is draw butted up against the top of the rect and there is free space on the bottom.  This makes it look centered.
	// Draw second rect one pixel inside first to make box two pixels wide
	GUI_DrawRect(AnalogGaugeEditRect.x0 + 1, AnalogGaugeEditRect.y0 - 4, AnalogGaugeEditRect.x1 + 3, AnalogGaugeEditRect.y1);
}

/*********************************************************************
*
*       Gauge Screen
*/
WM_HWIN Display_GaugeScreen(WM_HWIN hParent)
{
#ifdef	SHOW_GREEN_LINE
	if (vehicleData.tach.greenStart != 0)
		pGreen = new Alarm(true, vehicleData.tach.greenStart, vehicleData.tach.upperYellowStart, SPEED_COLOR, GUI_DARKGREEN, NORMAL, NULL, NULL);
	else pGreen = 0;
	pYellow = new Alarm(true, vehicleData.tach.upperYellowStart, vehicleData.tach.upperRedStart, GUI_YELLOW, GUI_DARKYELLOW, NORMAL, pGreen, NULL);
#else
	pYellow = new Alarm(true, vehicleData.tach.upperYellowStart, vehicleData.tach.upperRedStart, GUI_YELLOW, GUI_DARKYELLOW, NORMAL, NULL, NULL);
#endif
	pRed = new Alarm(true, vehicleData.tach.upperRedStart, CalculateScaleTop(vehicleData.tach.scaleEnd), GUI_RED, GUI_DARKRED, HIGH, pYellow, NULL);

	firsttimethrough = true;
	drawFlag = false;

	if ((hGaugeScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbGaugeCallback, 0)) <= 0)
		SystemError();

	if ((pGauge = new Gauge(GAUGE_START, 0, 136, 0, CalculateScaleTop(vehicleData.tach.scaleEnd), "", "", "", pRed, 1000, 500, 0, 1000, false, hGaugeScreen, &bmDefaultGauge)) <= 0)
		SystemError();
	pGauge->ShowDigital(false);		// No longer want to use gauges own digital image - we'll handle it manually on the screen

	if (selectedRaceMode == ROADRACE_MODE)
	{
		if ((pLapTime = new DigitalStringOutput(0, 163, 210, 48, Left, hGaugeScreen, GUI_FontD48, GUI_WHITE)) <= 0)
			SystemError();
		else pLapTime->SetValue("");

		if ((pPredictiveLap = new PredictiveLapTimer(30, 220, 207, 50, GetPredictiveScale(), -GetPredictiveScale(), hGaugeScreen)) <= 0)
			SystemError();
		else pPredictiveLap->SetValue(0.0);
	}
	else if ((pSpeed = new DigitalStringOutput(45, 10, 140, 48, Left, hGaugeScreen, GUI_FontD48, GUI_WHITE)) <= 0)
		SystemError();

	WM_InvalidateWindow(hGaugeScreen);

	hScreenTimer = WM_CreateTimer(hGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	// create a memory area to reduce flicker on paint.
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	return hGaugeScreen;
}

//
// Return true if screen touch in on the configurable gauge item
//
char IfAnalogConfigGaugePressed(int lastx, int lasty)
{
	if ((lastx >= AnalogGaugeEditRect.x0) && (lastx <= AnalogGaugeEditRect.x1) && (lasty >= AnalogGaugeEditRect.y0) && (lasty <= AnalogGaugeEditRect.y1))
		return true;
	else return false;
}


/*************************** End of file ****************************/
