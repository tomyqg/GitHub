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
#include "FlashReadWrite.h"
#include "DataHandler.h"
#include "StandardList.h"
#include "TachSetupScreens.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hTachSetupScreens;
static LISTWHEEL_Handle hWarningRPMListWheel, hRedlineListWheel, hCylindersListWheel, hMaxRPMListWheel;
int rpmListStart;

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmGreenButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmWarningRPM;
extern GUI_CONST_STORAGE GUI_BITMAP bmRedlineRPM;
extern GUI_CONST_STORAGE GUI_BITMAP bmEngineCylinders;
extern GUI_CONST_STORAGE GUI_BITMAP bmTachStartRPM;
extern GUI_CONST_STORAGE GUI_BITMAP bmPowerBandEnd;
extern GUI_CONST_STORAGE GUI_BITMAP bmMaxRPM;


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen);
void DisplayWarningRPM(WM_HWIN);
void DisplayRedline(WM_HWIN);
void DisplayCylinders(WM_HWIN);
void DisplayMaxRPM(WM_HWIN);
void DisplayPowerEnd(WM_HWIN);
void WarningRPMPaint(WM_HWIN);
void RedlinePaint(WM_HWIN);
void CylindersPaint(WM_HWIN);
void MaxRPMPaint(WM_HWIN);
void PowerEndPaint(WM_HWIN);
void BuildRPMList(int rpm, int end, int increment);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
struct SCREEN_LIST TachSetupScreenList[NUM_TACH_SETUP_SCREENS+1] = {
	{ WARNING_RPM_SETUP, DisplayWarningRPM, WarningRPMPaint },
	{ REDLINE_RPM_SETUP, DisplayRedline, RedlinePaint },
	{ ENGINE_CYLINDERS_SETUP, DisplayCylinders, CylindersPaint },
	{ POWERBANDEND_RPM_SETUP, DisplayPowerEnd, PowerEndPaint },
	{ MAX_RPM_SETUP, DisplayMaxRPM, MaxRPMPaint },
	{ -1, NULL, NULL }
};

#define	WARNING_WHEEL		GUI_ID_LISTWHEEL0
#define	REDLINE_WHEEL		GUI_ID_LISTWHEEL1
#define	CYLINDERS_WHEEL		GUI_ID_LISTWHEEL2
#define	POWERSTART_WHEEL	GUI_ID_LISTWHEEL3
#define	MAX_RPM_WHEEL		GUI_ID_LISTWHEEL4
#define	POWEREND_WHEEL		GUI_ID_LISTWHEEL5

#define	RPMLIST_INCREMENT	50
#define	MAXRPM_INCREMENT	100

// List wheels limited to 171 (plus 1 for NULL) entries
#define	MAX_RPMLIST_STEPS	170
char RPMListData[MAX_RPMLIST_STEPS][6];
char *RPMListPtrs[MAX_RPMLIST_STEPS];

char * apCylinders[] = {	// list wheel index
		"2 stroke",			// 0
		"ECU / 1",			// 1
		"2",				// 2
		"3",				// 3
		"4",				// 4
		"5",				// 5
		"6",				// 6
		"8",				// 7
		"10",				// 8
		"12",				// 9
		"16",				// 10
		NULL
};


/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
char CylindersToIndex(char cyl)
{
	if (cyl <= 6)
		return cyl;
	else if (cyl == 8)
		return 7;
	else if (cyl == 10)
		return 8;
	else if (cyl == 12)
		return 9;
	else if (cyl == 16)
		return 10;
	else return 0;
}

char IndexToCylinders(char index)
{
	if (index <= 6)
		return index;
	else if (index == 7)
		return 8;
	else if (index == 8)
		return 10;
	else if (index == 9)
		return 12;
	else if (index == 10)
		return 16;
	else return 0;
}

char *GetCylindersText(void)
{
	return apCylinders[CylindersToIndex(vehicleData.engineCylinders)];
}


WM_HWIN GetTachSetupWindowHandle(void)
{
	return hTachSetupScreens;
}

void SetTachSetupWindowHandle(WM_HWIN hWin)
{
	hTachSetupScreens = hWin;
}

void TachSetupScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;
//	int results;
	int Sel;
//	WM_HWIN hDlg;

//	hDlg = pMsg->hWin;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_CREATE:
			TachSetupIgnoreNextKeyRelease();
			break;

		case WM_PAINT:
			if (GetScreenType() == TACH_SETUP_SCREENS_TYPE)
			{
				// Call paint method of center screen
				TachSetupScreenList[GetLastScreen(TACH_SETUP_SCREENS_TYPE)].paint(hTachSetupScreens);
				DrawSignalStrength();
			}
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				case WM_NOTIFICATION_SEL_CHANGED:
					if (abs(LISTWHEEL_GetVelocity(pMsg->hWinSrc)) <= 1)
					{
						switch(Id)
						{
						case WARNING_WHEEL:
							vehicleData.tach.greenEnd = vehicleData.tach.upperYellowStart = ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) + rpmListStart) * RPMLIST_INCREMENT;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case REDLINE_WHEEL:
							vehicleData.tach.upperRedStart = ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) + rpmListStart) * RPMLIST_INCREMENT;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case CYLINDERS_WHEEL:
							LISTWHEEL_SetSel(pMsg->hWinSrc, vehicleData.engineCylinders = IndexToCylinders(LISTWHEEL_GetPos(pMsg->hWinSrc)));
							break;
						case POWERSTART_WHEEL:
							vehicleData.tach.greenStart = ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) + rpmListStart) * RPMLIST_INCREMENT;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case POWEREND_WHEEL:
							vehicleData.tach.greenEnd = ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) + rpmListStart) * RPMLIST_INCREMENT;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						case MAX_RPM_WHEEL:
							vehicleData.tach.scaleEnd = ((Sel = LISTWHEEL_GetPos(pMsg->hWinSrc)) + rpmListStart) * MAXRPM_INCREMENT;
							LISTWHEEL_SetSel(pMsg->hWinSrc, Sel);
							break;
						}
					}
					break;

				case WM_NOTIFICATION_CLICKED:
//					clickedOnThisScreen = true;
					switch (Id)
					{
						case WARNING_WHEEL:
						case REDLINE_WHEEL:
						case CYLINDERS_WHEEL:
						case POWERSTART_WHEEL:
						case POWEREND_WHEEL:
						case MAX_RPM_WHEEL:
						//case LAP_METER_WHEEL:
							pressingbutton_semaphore = 1;
							break;

						default:
							WM_DefaultProc(pMsg);
					}
					break;

				case WM_NOTIFICATION_RELEASED:
					pressingbutton_semaphore = 0;
					break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplayWarningRPM(WM_HWIN hParent)
{
	int rpm, end;

	if ((rpm = vehicleData.tach.greenStart) == 0)
		rpm = vehicleData.tach.upperYellowStart - 2000;

	if (((vehicleData.tach.upperRedStart - rpm) / RPMLIST_INCREMENT) > MAX_RPMLIST_STEPS)
	{
		rpmListStart = (rpm = vehicleData.tach.upperYellowStart - 1000) / RPMLIST_INCREMENT;
		end = vehicleData.tach.upperYellowStart + 1000;
	}
	else
	{
		rpmListStart = rpm / RPMLIST_INCREMENT;
		end = vehicleData.tach.upperRedStart;
	}
	BuildRPMList(rpm, end, RPMLIST_INCREMENT);

	DisplayStdListScreen(hParent, RPMListPtrs, WARNING_WHEEL, &hWarningRPMListWheel,
		((vehicleData.tach.upperYellowStart / RPMLIST_INCREMENT) - rpmListStart) , TEXT_ONLY, "Select RPM");
}

void BuildRPMList(int rpm, int end, int increment)
{
	int i;

	for (i = 0; i < MAX_RPMLIST_STEPS; i++)
	{
		if (rpm > end)
			break;
		else
		{
			sprintf(RPMListData[i], "%d", rpm);
			RPMListPtrs[i] = RPMListData[i];
			rpm += increment;
		}
	}
	RPMListPtrs[i] = 0;
}

void DisplayRedline(WM_HWIN hParent)
{
	int rpm, end;

	rpmListStart = (rpm = vehicleData.tach.upperYellowStart) / RPMLIST_INCREMENT;
	end = vehicleData.tach.scaleEnd;
	BuildRPMList(rpm, end, RPMLIST_INCREMENT);

	DisplayStdListScreen(hParent, RPMListPtrs, REDLINE_WHEEL, &hRedlineListWheel,
		((vehicleData.tach.upperRedStart / RPMLIST_INCREMENT) - rpmListStart) , TEXT_ONLY, "Select RPM");
}

void DisplayCylinders(WM_HWIN hParent)
{
	DisplayStdListScreen(hParent, apCylinders, CYLINDERS_WHEEL, &hCylindersListWheel,
			CylindersToIndex(vehicleData.engineCylinders), TEXT_ONLY, "Select Cylinders");
}


void DisplayPowerEnd(WM_HWIN hParent)
{
}

void DisplayMaxRPM(WM_HWIN hParent)
{
	int rpm, end;

	rpmListStart = (rpm = ((vehicleData.tach.upperRedStart % 100) * 100)) / MAXRPM_INCREMENT;
	end = 20000;
	BuildRPMList(rpm, end, MAXRPM_INCREMENT);

	DisplayStdListScreen(hParent, RPMListPtrs, MAX_RPM_WHEEL, &hMaxRPMListWheel,
		((vehicleData.tach.scaleEnd / MAXRPM_INCREMENT) - rpmListStart) , TEXT_ONLY, "Select RPM");
}


void WarningRPMPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmWarningRPM, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(WARNING_RPM_SETUP), 0, 229);
}

void RedlinePaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmRedlineRPM, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(REDLINE_RPM_SETUP), 0, 229);
}

void CylindersPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmEngineCylinders, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(ENGINE_CYLINDERS_SETUP), 0, 229);
}

void PowerEndPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmPowerBandEnd, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(POWERBANDEND_RPM_SETUP), 0, 229);
}

void MaxRPMPaint(WM_HWIN hParent)
{
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	GUI_DrawBitmap(&bmSingleSpinner, SPINNER_START, 42);
	GUI_DrawBitmap(&bmMaxRPM, 60, 0);
	GUI_DrawBitmap(GetNavigationArrow(MAX_RPM_SETUP), 0, 229);
}

static GUI_CONST_STORAGE GUI_BITMAP *GetNavigationArrow(int screen)
{
	return &bmleftarrow;
}

void TachSetupIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}


/*************************** End of file ****************************/

