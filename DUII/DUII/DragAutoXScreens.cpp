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
#include "DragAutoXScreens.h"
#include "DigitalDisplay.hpp"
#include "PredictiveLapTimer.hpp"
#include "DigitalStringOutput.hpp"
#include <time.h>
#include "Utilities.h"
#include "LEDHandler.h"
#include "gauge.hpp"


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void DAXAnalogCallback(WM_MESSAGE * pMsg);
void DAXDigitalCallback(WM_MESSAGE * pMsg);
void DAXRPMSweepCallback(WM_MESSAGE * pMsg);


/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hDAXGaugeScreen;
static WM_HTIMER hScreenTimer;
static GUI_MEMDEV_Handle mem;
static DigitalStringOutput *pLapTime = 0, *pGap = 0;
static DigitalStringOutput *pDistance = 0;
static DigitalStringOutput *pSpeed = 0;
static DigitalStringOutput *pLapTimer = 0;
static Gauge *pGauge = 0;
static PredictiveLapTimer *pPredictiveLap = 0;
static RPMSweep *pSweep = 0;
static DigitalDisplay *pMPH = 0;
static Alarm *pYellow = 0;
static Alarm *pRed = 0;
#ifdef	SHOW_GREEN_LINE
static Alarm *pGreen = 0;
#endif
/*********************************************************************
*
*       External Data and Code References
*
**********************************************************************
*/
#ifdef __cplusplus
extern "C" {
extern char lockupSemaphore;
extern GUI_CONST_STORAGE GUI_BITMAP bmDarkAsphalt;
}
#endif
extern void UpdatePredictiveLapControl(PredictiveLapTimer *pPredictiveLap, float predictiveLap);


/*********************************************************************
*
*       Local Constants
*
**********************************************************************
*/
// Rect = U/L coordinates, L/R coordinates (NOT size)
static const GUI_RECT VmaxRect = { 75, 68, 145, 108 };
static const GUI_RECT RPMVmaxRect = { 0, 70, 75, 110 };
const GUI_RECT AnalogPredictiveLapRect = { 30, 220, 237, 270 };
const GUI_RECT SweepTachPredictiveLapRect = { 130, 220, 470, 270 };
const GUI_RECT DigitalPredictiveLapRect = { 50, 212, 450, 262 };

void DAXRPMSweepCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			// Check for end of run
			if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
			{
				if (pPredictiveLap)
				{
					delete pPredictiveLap;
					pPredictiveLap = 0;
				}
			}
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDAXGaugeScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(NULL))				// Test to prevent flicker if a popup is displayed
		{
			char scratch[30], *pText;
			float ftemp;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			GUI_MEMDEV_Select(mem);
			GUI_Clear();

			pSweep->SetValue(GetValue(VEHICLE_RPM).iVal);

			if (vehicleData.diffRatio != 0.0)
				pSweep->SetGear(GetValue(GEAR).iVal);

			pMPH->SetValue(GetValue(SPEED).fVal);

			if (pPredictiveLap)
				pPredictiveLap->SetValue((float) GetValue(PREDICTIVE_LAP).fVal);

			pLapTimer->SetValue(formatLapTime(GetValue(LAP_TIME).uVal, false));

			pSweep->DrawControl();		
			
			pMPH->DrawControl();
			
			// Draw VMax value
			if (selectedRaceMode == DRAG_MODE)
			{
				ftemp = GetValue(MAX_SPEED).fVal;
				GUI_SetColor(GUI_PURPLE);
				pText = "vmax";
			}
			else
			{
				ftemp = GetValue(SPEED_PEAKS).fVal;
				if (ftemp <= 0)
				{
					GUI_SetColor(GUI_ORANGE);
					ftemp = 0 - ftemp;
					pText = "vmin";
				}
				else
				{
					GUI_SetColor(GUI_PURPLE);
					pText = "vmax";
				}
			}

			sprintf(scratch, "%d", (int)ftemp);
			GUI_SetFont(&GUI_FontD32);
			GUI_DispStringInRect(scratch, (GUI_RECT *)&RPMVmaxRect, GUI_TA_RIGHT);

			// Draw Vmax/Vmin
			GUI_SetFont(&GUI_Font32B_ASCII);
			GUI_DispStringAt(pText, 10, 107);

			if (pPredictiveLap)
			{
//				pPredictiveLap->DrawControl();
				UpdatePredictiveLapControl(pPredictiveLap, GetValue(PREDICTIVE_LAP).fVal);
			}
			else
			{
				GUI_SetColor(GUI_WHITE);
				GUI_DrawRectEx(&SweepTachPredictiveLapRect);
				if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
				{
					GUI_DispStringInRect("Run Complete", (GUI_RECT *)&SweepTachPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
				}
				else GUI_DispStringInRect("Tap Screen At Finish Line", (GUI_RECT *)&SweepTachPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
			}
			
			pLapTimer->DrawControl();

			GUI_DrawBitmap(&bmleftarrow, 0, 229);
			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(mem);
			GUI_SelectLCD();
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

WM_HWIN Display_DAXRPMSweepScreen(void)
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

	// create a memory area to reduce flicker on paint.
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hDAXGaugeScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, DAXRPMSweepCallback, 0)) <= 0)
		SystemError();

	if ((pSweep = new RPMSweep(0, 0, 480, 272, 0, CalculateScaleTop(vehicleData.tach.scaleEnd), 1000, 500, pRed, (DataValues[GEAR].iVal >= 0 && vehicleData.diffRatio > 0.0))) <= 0)
		SystemError();

	if ((pMPH = new DigitalDisplay(5, 5, 150, 50, "", sysData.units == METRIC ? "kph" : "mph",
			0, 250, 0, 0, Left, NULL, false, GUI_FontD32, GUI_Font32B_ASCII, hDAXGaugeScreen)) <= 0)
		SystemError();

	if (trackData.finishLineLat || selectedRaceMode == DRAG_MODE)
		if ((pPredictiveLap = new PredictiveLapTimer(SweepTachPredictiveLapRect.x0, SweepTachPredictiveLapRect.y0,
				SweepTachPredictiveLapRect.x1 - SweepTachPredictiveLapRect.x0, SweepTachPredictiveLapRect.y1 - SweepTachPredictiveLapRect.y0,
				GetPredictiveScale(), -GetPredictiveScale(), hDAXGaugeScreen)) <= 0)
			SystemError();

	if ((pLapTimer = new DigitalStringOutput(225, 160, 245, 60, Left, hDAXGaugeScreen, GUI_FontD48, GUI_WHITE)) <= 0)
		SystemError();

	hScreenTimer = WM_CreateTimer(hDAXGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDAXGaugeScreen;
}


void DAXAnalogCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			// Check for end of run
			if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
			{
				if (pPredictiveLap)
				{
					delete pPredictiveLap;
					pPredictiveLap = 0;
				}
			}
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDAXGaugeScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(NULL))				// Test to prevent flicker if a popup is displayed
		{
			char scratch[30], *pText;
			float ftemp;
			int itmp, rpm;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			GUI_SetColor(GUI_WHITE);
			GUI_SetFont(&GUI_FontD48);

			GUI_MEMDEV_Select(mem);
			GUI_Clear();

			// Draw background
			GUI_DrawBitmap(&bmDarkAsphalt, 0, 0);

			// Set overall text mode
			GUI_SetTextMode(GUI_TM_TRANS);

			// Draw speed
			sprintf(scratch, "%d ", (int)GetValue(SPEED).fVal);
			pSpeed->SetValue(scratch);
			pSpeed->DrawControl();

			// Draw speed units
			GUI_SetFont(&GUI_Font24B_ASCII);
			GUI_SetColor(GUI_GRAY);
			GUI_DispStringAt((sysData.units == STD ? "mph" : "kph"), 5, 35);
			
			// Update tach
			pGauge->SetValue(rpm = GetValue(VEHICLE_RPM).iVal);
			pGauge->DrawControl();

			// Draw VMax value
			GUI_SetFont(&GUI_FontD32);
			if (selectedRaceMode == DRAG_MODE)
			{
				ftemp = GetValue(MAX_SPEED).fVal;
				GUI_SetColor(GUI_PURPLE);
				pText = "vmax";
			}
			else
			{
				ftemp = GetValue(SPEED_PEAKS).fVal;
				if (ftemp <= 0)
				{
					GUI_SetColor(GUI_ORANGE);
					ftemp = 0 - ftemp;
					pText = "vmin";
				}
				else
				{
					GUI_SetColor(GUI_PURPLE);
					pText = "vmax";
				}
			}
			sprintf(scratch, "%d", (int)ftemp);
			GUI_DispStringInRect(scratch, (GUI_RECT *)&VmaxRect, GUI_TA_RIGHT);

			// Draw Vmax/Vmin
			GUI_SetFont(&GUI_Font24B_ASCII);
			GUI_DispStringAt(pText, 5, 80);


			// Update Predictive Lap Control
			if (pPredictiveLap)
			{
				//pPredictiveLap->SetValue(GetValue(PREDICTIVE_LAP).fVal);
				//pPredictiveLap->DrawControl();
				UpdatePredictiveLapControl(pPredictiveLap, GetValue(PREDICTIVE_LAP).fVal);
			}
			else
			{
				GUI_SetFont(&GUI_Font20_ASCII);
				GUI_SetColor(GUI_WHITE);
				GUI_DrawRectEx(&AnalogPredictiveLapRect);
				if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
				{
					GUI_DispStringInRect("Run Complete", (GUI_RECT *)&AnalogPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
				}
				else GUI_DispStringInRect("Tap Screen At Finish Line", (GUI_RECT *)&AnalogPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
			}

			// Draw lap time
			pLapTimer->SetValue(formatLapTime(GetValue(LAP_TIME).uVal, false));
			pLapTimer->DrawControl();

			// Display Peak RPM
			if ((itmp = GetValue(MAX_RPM).uVal) >= vehicleData.tach.scaleEnd)
				GUI_SetColor(GUI_RED);
			else if (itmp >= vehicleData.tach.upperRedStart)
				GUI_SetColor(GUI_YELLOW);
			else
				GUI_SetColor(GUI_GRAY);
			GUI_SetFont(&GUI_Font24B_ASCII);
			sprintf(scratch, "%d", itmp);
			GUI_DispStringInRect(scratch, (GUI_RECT *)&PeakRPMRect, GUI_TA_RIGHT);

			// Display current RPM
			GUI_SetColor(GUI_WHITE);
			sprintf(scratch, "%d", rpm);
			GUI_DispStringInRect(scratch, (GUI_RECT *)&RPMRect, GUI_TA_RIGHT);

			// Display Gear text
			GUI_SetColor(GUI_GREEN);
			GUI_SetFont(&GUI_FontD32);
			if ((vehicleData.diffRatio == 0.0) || (0 == (itmp = GetValue(GEAR).uVal)))
				sprintf(scratch, "  ");
			else
				sprintf(scratch, "%d ", itmp);
			GUI_SetColor(GUI_BLACK);
			GUI_DispStringAt(scratch, 334, 158);
			GUI_SetColor(GUI_LIGHTBLUE);
			GUI_DispStringAt(scratch, 332, 156);

			// Draw navigation arrow
			GUI_DrawBitmap(&bmleftarrow, 0, 229);
			CheckWarningPopups();
			GUI_MEMDEV_CopyToLCD(mem);
			GUI_SelectLCD();
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
		if(pLapTimer)
		{
			delete pLapTimer;
			pLapTimer = 0;
		}
		if (pSpeed)
		{
			delete pSpeed;
			pSpeed = 0;
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

/*********************************************************************
*
*       Gauge Screen
*/
WM_HWIN Display_DAXGaugeScreen(void)
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

	// create a memory area to reduce flicker on paint.
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hDAXGaugeScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, DAXAnalogCallback, 0)) <= 0)
		SystemError();

	if ((pGauge = new Gauge(208, 0, 136, 0, CalculateScaleTop(vehicleData.tach.scaleEnd), "", "", "", pRed, 1000, 500, 0, 1000, false, hDAXGaugeScreen, &bmDefaultGauge)) <= 0)
		SystemError();
	pGauge->ShowDigital(false);		// No longer want to use gauges own digital image - we'll handle it manually on the screen

	if ((pSpeed = new DigitalStringOutput(50, 5, 140, 70, Left, hDAXGaugeScreen, GUI_FontD48, GUI_WHITE)) <= 0)
		SystemError();

	if ((pLapTimer = new DigitalStringOutput(0, 160, 209, 50, Left, hDAXGaugeScreen, GUI_FontD48, GUI_WHITE)) <= 0)
		SystemError();

	if (trackData.finishLineLat || selectedRaceMode == DRAG_MODE)
		if ((pPredictiveLap = new PredictiveLapTimer(AnalogPredictiveLapRect.x0, AnalogPredictiveLapRect.y0,
				AnalogPredictiveLapRect.x1 - AnalogPredictiveLapRect.x0, AnalogPredictiveLapRect.y1 - AnalogPredictiveLapRect.y0, GetPredictiveScale(), -GetPredictiveScale(), hDAXGaugeScreen)) <= 0)
			SystemError();

	WM_InvalidateWindow(hDAXGaugeScreen);

	hScreenTimer = WM_CreateTimer(hDAXGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDAXGaugeScreen;
}


void DAXDigitalCallback(WM_MESSAGE * pMsg)
{
	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (pMsg->Data.v == hScreenTimer)
		{
			// Check for end of run
			if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
			{
				if (pPredictiveLap)
				{
					delete pPredictiveLap;
					pPredictiveLap = 0;
				}
				if(pGap)
				{
					delete pGap;
					pGap = 0;
				}
			}
			WM_RestartTimer(hScreenTimer, GAUGE_SCREEN_REFRESH_TIME);
			WM_InvalidateWindow(hDAXGaugeScreen);
		}
		break;

	case WM_PAINT:
		if (CS_IsVisible(NULL))				// Test to prevent flicker if a popup is displayed
		{
			char scratch[20];
			float gap, ftemp;

			if (lockupSemaphore)			// If main has not run since our last paint - exit!
				break;
			else lockupSemaphore = true;
			
			GUI_MEMDEV_Select(mem);
			GUI_Clear();

			// Draw background
			GUI_DrawBitmap(&bmDarkAsphalt, 0, 0);

			// Set overall text mode
			GUI_SetTextMode(GUI_TM_TRANS);

			// Draw speed
			if (selectedRaceMode == DRAG_MODE)
				ftemp = GetValue(MAX_SPEED).fVal;
			else ftemp = GetValue(SPEED).fVal;
			sprintf(scratch, "%d", (int)ftemp);
			pSpeed->SetValue(scratch);
			pSpeed->DrawControl();

			// Update Predictive Lap Control
			if (pPredictiveLap)
			{
				//pPredictiveLap->SetValue(gap = GetValue(PREDICTIVE_LAP).fVal);
				//pPredictiveLap->DrawControl();
				UpdatePredictiveLapControl(pPredictiveLap, gap = GetValue(PREDICTIVE_LAP).fVal);
			}

			// Draw lap time
			pLapTime->SetValue(formatLapTime(GetValue(LAP_TIME).uVal, false));
			pLapTime->DrawControl();

			// Display fixed text
			GUI_SetColor(GUI_GRAY);
			GUI_SetFont(&GUI_Font24B_ASCII);

			if (sysData.units == METRIC)
				GUI_DispStringAt("meters", 205, 48);
			else GUI_DispStringAt("feet", 215, 48);

			if (pGap)
				GUI_DispStringAt("Gap", 410, 48);

			// Draw speed units
			GUI_DispStringAt((sysData.units == STD ? "mph" : "kph"), 50, 48);
			
			if (!pPredictiveLap)
			{
				GUI_SetColor(GUI_WHITE);
				GUI_DrawRectEx(&DigitalPredictiveLapRect);
				if ((unsavedSysData.systemMode == DU2_AUTOX_COMPLETE) || (unsavedSysData.systemMode == DU2_DRAG_COMPLETE))
				{
					GUI_DispStringInRect("Run Complete", (GUI_RECT *)&DigitalPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
				}
				else GUI_DispStringInRect("Tap Screen At Finish Line", (GUI_RECT *)&DigitalPredictiveLapRect, GUI_TA_HCENTER | GUI_TA_VCENTER);
			}

			// Display distance
			sprintf(scratch, "%d", (int)(GetValue(LAP_DISTANCE).fVal + 0.5));		// round up
			pDistance->SetValue(scratch);
			pDistance->DrawControl();

			if (pGap)
			{
				// Display timing gap
				if (gap < 0.0)
					pGap->SetColor(GUI_GREEN);
				else pGap->SetColor(GUI_RED);
				sprintf(scratch, "%+0.2f", gap);
				pGap->SetValue(scratch);
				pGap->DrawControl();
			}

			GUI_DrawBitmap(&bmleftarrow, 0, 229);
			CheckWarningPopups();

			GUI_MEMDEV_CopyToLCD(mem);
			GUI_SelectLCD();
		}
		break;

	case WM_DELETE:
		if(pPredictiveLap)
		{
			delete pPredictiveLap;
			pPredictiveLap = 0;
		}
		if(pDistance)
		{
			delete pDistance;
			pDistance = 0;
		}
		if(pLapTime)
		{
			delete pLapTime;
			pLapTime = 0;
		}
		if(pGap)
		{
			delete pGap;
			pGap = 0;
		}
		if (mem)
		{
			GUI_MEMDEV_Delete(mem);
			mem = 0;
		}
		if (pSpeed)
		{
			delete pSpeed;
			pSpeed = 0;
		}
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

WM_HWIN Display_DAXDigitalGaugeScreen(void)
{
	// create the memory area to prevent flicker
	mem = GUI_MEMDEV_Create(0, 0, XSIZE_PHYS, YSIZE_PHYS);

	if ((hDAXGaugeScreen = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW, DAXDigitalCallback, 0)) <= 0)
		SystemError();

	if (trackData.finishLineLat || selectedRaceMode == DRAG_MODE)
	{
		if ((pPredictiveLap = new PredictiveLapTimer(DigitalPredictiveLapRect.x0, DigitalPredictiveLapRect.y0,
			DigitalPredictiveLapRect.x1 - DigitalPredictiveLapRect.x0, DigitalPredictiveLapRect.y1 - DigitalPredictiveLapRect.y0, GetPredictiveScale(), -GetPredictiveScale(), hDAXGaugeScreen)) <= 0)
				SystemError();

		if ((pGap = new DigitalStringOutput(300, 10, 170, 40, Left, hDAXGaugeScreen, GUI_FontD32, GUI_GREEN)) <= 0)
			SystemError();
	}

	if ((pSpeed = new DigitalStringOutput(0, 10, 120, 40, Left, hDAXGaugeScreen, GUI_FontD32, GUI_WHITE)) <= 0)
		SystemError();

	if ((pDistance = new DigitalStringOutput(155, 10, 120, 40, Left, hDAXGaugeScreen, GUI_FontD32, GUI_WHITE)) <= 0)
		SystemError();

	if ((pLapTime = new DigitalStringOutput(90, 110, 310, 90, Left, hDAXGaugeScreen, GUI_FontD80, GUI_WHITE)) <= 0)
		SystemError();

	WM_InvalidateWindow(hDAXGaugeScreen);

	hScreenTimer = WM_CreateTimer(hDAXGaugeScreen, 0, GAUGE_SCREEN_REFRESH_TIME, 0);

	return hDAXGaugeScreen;
}
