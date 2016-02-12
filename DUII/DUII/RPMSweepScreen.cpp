#include <stdio.h>
#include <stdlib.h>
#include "DUII.h"
#include "GUI.h"
#include "GeneralConfig.h"
#include "commondefs.h"
#include "ConfigScreens.h"
#include "RunningGauges.h"
#include "DataHandler.h"
#include "RPMSweep.hpp"
#include "RPMSweepScreen.h"
#include "DigitalDisplay.hpp"
#include "PredictiveLapTimer.hpp"
#include "DigitalStringOutput.hpp"
#include <time.h>
#include "Utilities.h"
#include "LEDHandler.h"
#include "ConfigurableGaugeCommon.h"

void UpdatePredictiveLapControl(PredictiveLapTimer *pPredictiveLap, float predictiveLap);

static WM_HWIN hRPMScreen;

static RPMSweep *pSweep = 0;
static DigitalDisplay *pMPH = 0;
//static DigitalDisplay *pLapCounter = 0;
static DigitalStringOutput *pLapTimer = 0;
static PredictiveLapTimer *pPredictiveLap = 0;

static WM_HTIMER hScreenTimer;

static Alarm *pYellow = 0;
static Alarm *pRed = 0;
#ifdef	SHOW_GREEN_LINE
static Alarm *pGreen = 0;
#endif

static bool firsttimethrough;

static GUI_MEMDEV_Handle mem;

#ifdef __cplusplus
extern "C" {
extern char lockupSemaphore;
extern GUI_CONST_STORAGE GUI_BITMAP bmBestLapEver;
extern GUI_CONST_STORAGE GUI_BITMAP bmBestLapSession;
}
#endif
int rpm, gear, lap, holding, lapTime;
float speed, predictiveLap, altitude;

static const GUI_RECT LapRect = { 340, 113, 465, 155 };

// Comment out the BIG_NUMBERS definition to use the original small text and numbers for the configurable
// data item or leave commented for Ken's recommended larger fonts which match the fonts of the speed.
#define	BIG_NUMBERS

#ifdef BIG_NUMBERS
const GUI_RECT DataElementRect =   { 5,  60, 140,  92 };
const GUI_RECT NameRect =		   { 5, 100, 140, 120 };
const GUI_RECT UnitsRect =		   { 5, 120, 140, 140 };
const GUI_RECT SweepTachEditRect = { 0,  60,  90, 135 };

const static DISPLAY_ELEMENT pData =	{ DataElementRect,	GUI_TA_LEFT,	GUI_ORANGE,		&GUI_FontD32 };
const static DISPLAY_ELEMENT pName =	{ NameRect,			GUI_TA_LEFT,	GUI_LIGHTGRAY,	&GUI_Font20B_ASCII };
const static DISPLAY_ELEMENT pUnits =	{ UnitsRect,		GUI_TA_LEFT,	GUI_LIGHTGRAY,	&GUI_Font20B_ASCII };
#else
const GUI_RECT NameRect =		   { 5,  60, 140,  80 };
const GUI_RECT UnitsRect =		   { 5,  80, 140, 100 };
const GUI_RECT DataElementRect =   { 5, 100, 140, 132 };
const GUI_RECT SweepTachEditRect = { 0,  60,  90, 130 };

const static DISPLAY_ELEMENT pData =	{ DataElementRect,	GUI_TA_LEFT,	GUI_WHITE,	&GUI_Font32B_ASCII };
const static DISPLAY_ELEMENT pName =	{ NameRect,			GUI_TA_LEFT,	GUI_GRAY,	&GUI_Font20B_ASCII };
const static DISPLAY_ELEMENT pUnits =	{ UnitsRect,		GUI_TA_LEFT,	GUI_GRAY,	&GUI_Font20B_ASCII };
#endif

const CONFIG_ARGS config = { (DISPLAY_ELEMENT *)&pData, (DISPLAY_ELEMENT *)&pName, (DISPLAY_ELEMENT *)&pUnits };

void SweepHighlightConfigGauge(void)
{
	GUI_SetColor(GUI_RED);
	GUI_DrawRect(SweepTachEditRect.x0, SweepTachEditRect.y0 - 2, SweepTachEditRect.x1 + 20, SweepTachEditRect.y1 + 4);		// Start box some pixels above the actual rectangle because
	// the text is draw butted up against the top of the rect and there is free space on the bottom.  This makes it look centered.
	// Draw second rect one pixel inside first to make box two pixels wide
	GUI_DrawRect(SweepTachEditRect.x0 + 1, SweepTachEditRect.y0 - 1, SweepTachEditRect.x1 + 19, SweepTachEditRect.y1 + 3);
}

void _cbRPMSweepCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hRPMScreen);

			// Check "box timer" if running
			if (drawFlag)
			{
				if ((GUI_GetTime() - boxStartTime) > BOX_PERSIST_TIME)
				{
					configureGaugeMode = false;
					drawFlag = false;
				}
			}

			// Update all values for next paint (to spread the load)
			rpm = GetValue(VEHICLE_RPM).iVal;
			gear = GetValue(GEAR).iVal;
			speed = GetValue(SPEED).fVal;
			lap = GetValue(LAP_NUMBER).iVal;
			holding = GetValue(HOLDING).uVal;
			predictiveLap = GetValue(PREDICTIVE_LAP).fVal;
			lapTime = GetValue(LAP_TIME).uVal;
			altitude = GetValue(ALTITUDE).fVal;
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(RPM_SWEEP_GAUGE) || firsttimethrough)
		{
			if (!lockupSemaphore)			// If main has not run since our last paint - exit!
			{
				lockupSemaphore = true;
			
				firsttimethrough = FALSE;
	
				GUI_MEMDEV_Select(mem);
				GUI_Clear();
				
				pSweep->SetValue(rpm);
				if (vehicleData.diffRatio != 0.0)
					pSweep->SetGear(gear);
				pSweep->DrawControl();		
	
				pMPH->SetValue(speed);
				pMPH->DrawControl();
	
				if (selectedRaceMode == ROADRACE_MODE)
				{
					char scratch[30];
					int pos;
	
					// Draw "lap" text
					GUI_SetFont(&GUI_Font32B_ASCII);
					GUI_SetColor(GUI_CYAN);
					sprintf(scratch, "%d", lap);
					if (lap > 99)
						pos = 340;
					else pos = 360;
					GUI_DispStringAt("Lap", pos, 118);
					GUI_SetFont(&GUI_FontD32);
					GUI_DispStringInRect(scratch, (GUI_RECT *)&LapRect, GUI_TA_RIGHT);
	
					// Draw best lap icon
					if (holding)		// If currently in lap hold time
						switch (DataValues[BEST_LAP].uVal) {
							case 0:
								// undraw icon when lap hold time expires
								break;
							case 1:
								// draw best lap of session icon (checker)
								GUI_DrawBitmap(&bmBestLapSession, LapRect.x0 - 40, LapRect.y0 + 5);
								break;
							case 2:
								// draw best lap ever icon (gold cup)
								GUI_DrawBitmap(&bmBestLapEver, LapRect.x0 - 40, LapRect.y0 + 5);
								break;
						} // switch
	
					GUI_SetFont(&GUI_FontD32);
					GUI_SetColor(GUI_WHITE);
	
					// Update Predictive Lap Control
					UpdatePredictiveLapControl(pPredictiveLap, predictiveLap);

					// Update Lap Timer Display
					pLapTimer->SetValue(formatLapTime(lapTime, false));
					pLapTimer->DrawControl();
				}
				else
				{
				// Since elapsed time does not use a memory device it must be drawn down here after all memory device objects are handled
					int time;
					char scratch[30];
	
					// Draw "session time" text
					GUI_SetColor(GUI_LIGHTGRAY);
					GUI_SetFont(&GUI_Font32B_ASCII);
					GUI_DispStringAt("session time", 270, 160);
	
					// Draw elapsed time
					time = DataValues[ELAPSED_TIME].uVal / 1000;
					if (time < 3600)
						sprintf(scratch, "%02d:%02d", time / 60, time % 60);
					else if (time < 36000)
						sprintf(scratch, "%d:%02d:%02d", time/3600, (time/60)%60, time % 60);
					else 		// > 10 hours
						sprintf(scratch, "%d:%02d", time/3600, (time/60)%60);
					GUI_SetColor(GUI_WHITE);
					GUI_SetFont(&GUI_FontD48);
					GUI_DispStringHCenterAt(scratch, 348, 200);
				}
	
				// Display the selected configurable gauge item
				if (GetConfigurableItem() == ALTITUDE)
					if (altitude >= 1000.0)			// If 4 or more digits to display
						GUI_SetFont(&GUI_Font32B_ASCII);			// Then use a smaller font
				DisplayConfigurableItem( &config );
				
				// Draw box around programmable gauge member(s) if requested
				if (drawFlag)
					SweepHighlightConfigGauge();
	
#ifdef	INCLUDE_RACE_HELP
				// If the help screen is still being displayed then show the appropriate help screen
				if (raceTimerActive)
				{
					CheckRaceHelpScreen(RPM_SWEEP_GAUGE);
					SweepHighlightConfigGauge();
				}
#endif

				CheckWarningPopups();
	
				if (SlideOutVisible())
					DrawSignalStrength();
	
				GUI_DrawBitmap(&bmthreewayarrow, 0, 229);

				GUI_MEMDEV_CopyToLCD(mem);
				GUI_SelectLCD();
			}
		}
		break;

	case WM_DELETE:
		if (pMPH)
		{
			delete pMPH;
			pMPH = 0;
		}
		if (pSweep)
		{
			delete pSweep;
			pSweep = 0;
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
		if (pPredictiveLap)
		{
			delete pPredictiveLap;
			pPredictiveLap = 0;
		}
		if(pLapTimer)
		{
			delete pLapTimer;
			pLapTimer = 0;
		}
		GUI_MEMDEV_Delete(mem);

		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

WM_HWIN Display_RPMSweepScreen(WM_HWIN hParent)
{
#ifdef	SHOW_GREEN_LINE
	if (vehicleData.tach.greenStart != 0)
		pGreen = new Alarm(true, vehicleData.tach.greenStart, vehicleData.tach.upperYellowStart, GUI_GREEN, GUI_DARKGREEN, NORMAL, NULL, NULL);
	else pGreen = 0;
	pYellow = new Alarm(true, vehicleData.tach.upperYellowStart, vehicleData.tach.upperRedStart, GUI_YELLOW, GUI_DARKYELLOW, NORMAL, pGreen, NULL);
#else
	pYellow = new Alarm(true, vehicleData.tach.upperYellowStart, vehicleData.tach.upperRedStart, GUI_YELLOW, GUI_DARKYELLOW, NORMAL, NULL, NULL);
#endif
	pRed = new Alarm(true, vehicleData.tach.upperRedStart, CalculateScaleTop(vehicleData.tach.scaleEnd), GUI_RED, GUI_DARKRED, HIGH, pYellow, NULL);

	firsttimethrough = true;
	drawFlag = false;
	lapTime = holding = lap = gear = rpm = 0;			// Initialize data variables
	altitude = predictiveLap = speed = 0.0;

	if ((hRPMScreen = WM_CreateWindowAsChild(0, 0, XSIZE_PHYS, YSIZE_PHYS, hParent, WM_CF_SHOW, _cbRPMSweepCallback, 0)) <= 0)
		SystemError();

	if ((pSweep = new RPMSweep(0, 0, 480, 272, 0, CalculateScaleTop(vehicleData.tach.scaleEnd), 1000, 500, pRed, (DataValues[GEAR].iVal >= 0 && vehicleData.diffRatio > 0.0))) <= 0)
		SystemError();

	if ((pMPH = new DigitalDisplay(5, 5, 150, 50, "", sysData.units == METRIC ? "kph" : "mph",
			0, 250, 0, 0, Left, NULL, false, GUI_FontD32, GUI_Font32B_ASCII, hRPMScreen)) <= 0)
		SystemError();

	if (selectedRaceMode == ROADRACE_MODE)
	{
		if ((pPredictiveLap = new PredictiveLapTimer(130, 220, 345, 50, GetPredictiveScale(), -GetPredictiveScale(), hRPMScreen)) <= 0)
			SystemError();

		if ((pLapTimer = new DigitalStringOutput(225, 160, 245, 60, Left, hRPMScreen, GUI_FontD48, GUI_WHITE)) <= 0)
			SystemError();
	}

	hScreenTimer = WM_CreateTimer(hRPMScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	// create a memory area to reduce flicker on paint.
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	return hRPMScreen;
}

//
// Return true if screen touch in on the configurable gauge item
//
char IfRPMSweepConfigGaugePressed(int lastx, int lasty)
{
	if ((lastx >= SweepTachEditRect.x0) && (lastx <= SweepTachEditRect.x1) && (lasty >= SweepTachEditRect.y0) && (lasty <= SweepTachEditRect.y1))
		return true;
	else return false;
}

//
// Update Predictive Lap Control
//	Handles alternate text display of Predictive Lap Control in the following way:
//		If waiting for start record speed display waiting message
//		If waiting for stop timer then display remaining time
//		If waiting for Lap 1 display track name
//		else display predictive time
//
void UpdatePredictiveLapControl(PredictiveLapTimer *pPredictiveLap, float predictiveLap)
{
	char scratch[40];

	// During the first lap write the name of the track in the predictive lap box.
	if (unsavedSysData.waitingForRecordSpeed) {
		float ftemp = (float) ((sysData.multiSettings_2 & 0xFFFF0000) >> 16);
		if (STD == sysData.units)
			ftemp *= KILOMETERS_TO_MILES;
		
		sprintf(scratch, "Recording Starts: %d%s", (int) (ftemp + 0.5), (sysData.units == STD ? "mph" : "kph"));
		pPredictiveLap->SetValue(scratch);
	}	
	else if (unsavedSysData.timeToStopRecording)
	{
		sprintf(scratch, "Recording Stops: %ds", unsavedSysData.timeToStopRecording);
		pPredictiveLap->SetValue(scratch);
	}
	else if (lap == 0)
		pPredictiveLap->SetValue(sysData.track);
	else pPredictiveLap->SetValue(predictiveLap);

	pPredictiveLap->DrawControl();
}