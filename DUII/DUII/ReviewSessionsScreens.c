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
#include "commondefs.h"
#include "DIALOG.h"
#include "ConfigScreens.h"
#include "GeneralConfig.h"
#include "FlashReadWrite.h"
#include "DataHandler.h"
#include "ReviewSessionsScreens.h"
#include "StandardList.h"
#include "RaceSetupScreens.h"
#include "SystemSettingsScreens.h"


/*********************************************************************
*
*       Public Variables
*
**********************************************************************
*/

/*********************************************************************
*
*       Local enumerations
*
**********************************************************************
*/
enum DISPLAY_TYPES {
	DT_TIMING,
	DT_GENERAL,
	DT_PERFORMANCE,
	DT_INPUTS,
	DT_CONFIG
};


/*********************************************************************
*
*       Local Prototypes
*
**********************************************************************
*/
void SessionDetailsPaint(WM_HWIN);
void SetReviewTypeButtonText(void);
void DisplayDataPanel(WM_HWIN hParent);
MULTIEDIT_HANDLE CreateTextBox(WM_HWIN hParent, char *pText);


/*********************************************************************
*
*       Tables and Constants
*
**********************************************************************
*/
const char *pDisplayText[] = {
	"TIMING",
	"GENERAL",
	"PERFORMANCE",
	"INPUTS",
	"CONFIG"
};

#define	LAP_BUTTON			GUI_ID_USER + 1
#define	YES_BUTTON			GUI_ID_USER + 2
#define	NO_BUTTON			GUI_ID_USER + 3
#define	REVIEW_TYPE_BUTTON	GUI_ID_USER + 4
#define	LAP_TIME_LIST		GUI_ID_LISTVIEW0
#define	TEXT_BOX			GUI_ID_TEXT0
#define LEFT_CENTER			140

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmthreewayarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmleftarrow;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmSingleSpinner;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackgroundWithPanel;
extern GUI_CONST_STORAGE GUI_BITMAP bmBanner;
extern GUI_CONST_STORAGE GUI_BITMAP bmSessionDetails;
extern GUI_CONST_STORAGE GUI_BITMAP bmyes;
extern GUI_CONST_STORAGE GUI_BITMAP bmno;
extern GUI_CONST_STORAGE GUI_BITMAP bmBlueButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmTimeSlip;

/*********************************************************************
*
*       Local Variables
*
**********************************************************************
*/
static WM_HWIN hReviewSessionsScreens;
LISTWHEEL_Handle hLapsListWheel;
BUTTON_Handle hYes, hNo, hReviewTypeBtn;
char BestLapText[20];
enum DISPLAY_TYPES displayType;
LISTVIEW_Handle hLapTimes;
MULTIEDIT_HANDLE hTextBox;
char **pLapList;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
WM_HWIN GetReviewSessionsWindowHandle(void)
{
	return hReviewSessionsScreens;
}

void SetReviewSessionsWindowHandle(WM_HWIN hWin)
{
	hReviewSessionsScreens = hWin;
}

void ReviewSessionsScreensCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
		case WM_TIMER:
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
			break;

		case WM_PAINT:
			SessionDetailsPaint(hReviewSessionsScreens);
			DrawSignalStrength();
			break;

		case WM_NOTIFY_PARENT:
			Id = WM_GetId(pMsg->hWinSrc);		// Id of widget
			NCode = pMsg->Data.v;				// Notification code
			switch (NCode)
			{
				//case WM_NOTIFICATION_SEL_CHANGED:
			case WM_NOTIFICATION_RELEASED:
				switch (Id)
				{
				case REVIEW_TYPE_BUTTON:
					if (++displayType > DT_CONFIG)
					{
						if ((sysData.selectedSessionType == ROADRACE_TYPE) || (sysData.selectedSessionType == QUARTERMILE_TYPE)
								|| (sysData.selectedSessionType == AUTOCROSS_TYPE))
							displayType = DT_TIMING;
						else displayType = DT_GENERAL;
					}
					else if ((sysData.selectedSessionType == QUARTERMILE_TYPE) && (displayType == DT_PERFORMANCE))
						displayType = DT_INPUTS;

					SetReviewTypeButtonText();
					DisplayDataPanel(hReviewSessionsScreens);
					break;
				}
				break;
			}
			break;

		default:
			WM_DefaultProc(pMsg);
	}
}

void DisplaySessionDetailsScreen(void)
{
	SetLastScreen(REVIEW_SESSIONS_SCREENS_TYPE, CHECK_TIMES_SCREEN);
	SetScreenScrollingHOnly();		// Still needed for navigation purposes - but no scrolling as actually going on

	hReviewSessionsScreens = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, ReviewSessionsScreensCallback, 0);

	hLapTimes = hTextBox = 0;
	displayType = DT_TIMING;			// Always start with timing screens
	pLapList = CreateLapsList();
	DisplayDataPanel(hReviewSessionsScreens);

	hReviewTypeBtn = BUTTON_CreateEx(BUTTON_LEFT, 218, BUTTON_WIDTH, BUTTON_HEIGHT, hReviewSessionsScreens, WM_CF_SHOW, 0, REVIEW_TYPE_BUTTON);
	BUTTON_SetFont(hReviewTypeBtn, &GUI_Font24_ASCII);
	BUTTON_SetBitmap(hReviewTypeBtn, BUTTON_BI_UNPRESSED, &bmBlueButton);
	BUTTON_SetTextColor(hReviewTypeBtn, BUTTON_BI_UNPRESSED, GUI_WHITE);
	BUTTON_SetTextColor(hReviewTypeBtn, BUTTON_BI_DISABLED, GUI_WHITE);
	SetReviewTypeButtonText();

	hScreenTimer = WM_CreateTimer(hReviewSessionsScreens, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

void ReturnFromReviewSessionsScreen(void)
{
	sysData.navigatedToReviewAfterRace = false;			// Leaving review sessions - clear entry flag
	// "Returning" to the REVIEW_SESSIONS screen by hitting the BACK button
	if (!GetConfigWindowHandle())
	{
		// First time after powering directly up to the CHECK_TIMES_SCREEN screen - create REVIEW_SESSIONS screen from scratch
		DisplayNewScreenList(CONFIG_SCREEN_TYPE, REVIEW_SESSIONS, hReviewSessionsScreens);
	}
	else
	{
		// Hitting BACK to return to the REVIEW_SESSIONS screen in normal circumstances - the screen already exists underneath
		// the CHECK_TIMES_SCREEN screen - reset screen origination and navigation variables
		SetLastScreen(CONFIG_SCREEN_TYPE, REVIEW_SESSIONS);
		WM_Invalidate(GetConfigWindowHandle());
		WM_DeleteWindow(hReviewSessionsScreens);
		hScreenTimer = WM_CreateTimer(GetConfigWindowHandle(), 0, SETUP_SCREEN_REFRESH_TIME, 0);
		ResetNewScreen();				// Inform title bar that this is a new screen
	}
}


void DisplayDataPanel(WM_HWIN hParent)
{
	HEADER_Handle hHeader;
	char **pLaps;
	int i;

	if (hLapTimes)
	{
		WM_DeleteWindow(hLapTimes);
		hLapTimes = 0;
	}

	if ((sysData.selectedSessionType != ROADRACE_TYPE) && (sysData.selectedSessionType != QUARTERMILE_TYPE) && (sysData.selectedSessionType != AUTOCROSS_TYPE)
			&& (displayType == DT_TIMING))
		displayType = DT_GENERAL;
	
	switch (displayType)
	{
	case DT_TIMING:
		if (hTextBox)
		{
			WM_DeleteWindow(hTextBox);
			hTextBox = 0;
		}

		if (sysData.selectedSessionType == ROADRACE_TYPE)
		{
			// Display road race timing data
			hLapTimes = LISTVIEW_CreateEx(32, 50, 215, 205, hParent, WM_CF_SHOW, 0, LAP_TIME_LIST);
			WM_SetFocus(hLapTimes);
			LISTVIEW_AddColumn(hLapTimes, 60, "Lap", GUI_TA_HCENTER | GUI_TA_VCENTER);
			LISTVIEW_AddColumn(hLapTimes, 155, "Time", GUI_TA_HCENTER | GUI_TA_VCENTER);
			LISTVIEW_SetAutoScrollV(hLapTimes, 1);
			LISTVIEW_SetFont(hLapTimes, &GUI_Font24B_ASCII);
			HEADER_SetFont(hHeader = LISTVIEW_GetHeader(hLapTimes), &GUI_Font24B_ASCII);
			LISTVIEW_SetRowHeight(hLapTimes, FINGER_WIDTH);

			i = 1;
			pLaps = pLapList;
			while (*pLaps)
			{
				char *pDest[2];
				char lapNumber[5];

				sprintf(lapNumber, "%d", i++);
				pDest[0] = lapNumber;
				pDest[1] = *pLaps++;
				LISTVIEW_AddRow(hLapTimes, pDest);
			}
			// If more than 5 items displayed then we will display a scroll bar, reset the column width to account for
			// scroll bar width so that column heading and highlight look correct
			if (i > 6)
			{
				LISTVIEW_SetColumnWidth(hLapTimes, 1, 155 - FINGER_WIDTH);
				SCROLLBAR_SetWidth(WM_GetScrollbarV(hLapTimes), FINGER_WIDTH);
			}

			i = GetBestLapNumber(selectedSessionUnsorted) - 1;
			LISTVIEW_SetItemBkColor(hLapTimes, 0, i, LISTVIEW_CI_UNSEL, GUI_MEDGREEN);
			LISTVIEW_SetItemBkColor(hLapTimes, 1, i, LISTVIEW_CI_UNSEL, GUI_MEDGREEN);

			HEADER_SetHeight(hHeader, FINGER_WIDTH);
			LISTVIEW_SetCompareFunc(hLapTimes, 0, &LISTVIEW_CompareDec);
			LISTVIEW_SetCompareFunc(hLapTimes, 1, &LISTVIEW_CompareText);
			LISTVIEW_EnableSort(hLapTimes);
		}
		else
		{
			// Display drag race time slip
		}
		break;

	case DT_GENERAL:
		if (!hTextBox)
			hTextBox = CreateTextBox(hParent, GetSessionText(ST_GENERAL));
		else
		{
			MULTIEDIT_SetText(hTextBox, GetSessionText(ST_GENERAL));
			SCROLLBAR_SetWidth(WM_GetScrollbarV(hTextBox), FINGER_WIDTH);
		}
		break;

	case DT_PERFORMANCE:
		if (!hTextBox)
			hTextBox = CreateTextBox(hParent, GetSessionText(ST_PERFORMANCE));
		else
		{
			MULTIEDIT_SetText(hTextBox, GetSessionText(ST_PERFORMANCE));
			SCROLLBAR_SetWidth(WM_GetScrollbarV(hTextBox), FINGER_WIDTH);
		}
		break;

	case DT_INPUTS:
		if (!hTextBox)
			hTextBox = CreateTextBox(hParent, GetSessionText(ST_INPUTS));
		else
		{
			MULTIEDIT_SetText(hTextBox, GetSessionText(ST_INPUTS));
			SCROLLBAR_SetWidth(WM_GetScrollbarV(hTextBox), FINGER_WIDTH);
		}
		break;

	case DT_CONFIG:
		if (!hTextBox)
			hTextBox = CreateTextBox(hParent, GetSessionText(ST_CONFIG));
		else
		{
			MULTIEDIT_SetText(hTextBox, GetSessionText(ST_CONFIG));
			SCROLLBAR_SetWidth(WM_GetScrollbarV(hTextBox), FINGER_WIDTH);
		}
		break;
	}
}

MULTIEDIT_HANDLE CreateTextBox(WM_HWIN hParent, char *pText)
{
	MULTIEDIT_HANDLE hTextBox;

	hTextBox = MULTIEDIT_CreateEx(32, 50, 215, 205, hParent, WM_CF_SHOW, 0, TEXT_BOX, 200, pText);
	MULTIEDIT_SetFont(hTextBox,&GUI_Font20_ASCII);
	MULTIEDIT_SetAutoScrollV(hTextBox, 1);
	MULTIEDIT_SetAutoScrollH(hTextBox, 1);
	MULTIEDIT_SetReadOnly(hTextBox, 1);
	MULTIEDIT_SetBkColor(hTextBox, MULTIEDIT_CI_READONLY, GUI_WHITE);
	SCROLLBAR_SetWidth(WM_GetScrollbarV(hTextBox), FINGER_WIDTH);
	MULTIEDIT_SetWrapWord(hTextBox);
	return hTextBox;
}


void SetReviewTypeButtonText(void)
{
	BUTTON_SetText(hReviewTypeBtn, pDisplayText[displayType]);
}


void SessionDetailsPaint(WM_HWIN hParent)
{
	GUI_SetFont(&GUI_Font24_ASCII);
	GUI_SetColor(GUI_WHITE);
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);

	// draw remaining session notes
	GUI_DrawBitmap(&bmBackgroundWithPanel, 0, 0);
	if (sysData.selectedSessionType == ROADRACE_TYPE) {
		GUI_DispStringHCenterAt(NameStruct.track, BUTTON_LEFT + (BUTTON_WIDTH / 2), 52);
		GUI_DispStringHCenterAt(NameStruct.date, BUTTON_LEFT + (BUTTON_WIDTH / 2), 85);
		GUI_DispStringHCenterAt(NameStruct.time, BUTTON_LEFT + (BUTTON_WIDTH / 2), 118);
	}
	else {
		GUI_DispStringHCenterAt(NameStruct.track, BUTTON_LEFT + (BUTTON_WIDTH / 2), 52);
		GUI_DispStringHCenterAt(NameStruct.driver, BUTTON_LEFT + (BUTTON_WIDTH / 2), 85);
		GUI_DispStringHCenterAt(NameStruct.vehicle, BUTTON_LEFT + (BUTTON_WIDTH / 2), 118);
		GUI_DispStringHCenterAt(NameStruct.date, BUTTON_LEFT + (BUTTON_WIDTH / 2), 151);
		GUI_DispStringHCenterAt(NameStruct.time, BUTTON_LEFT + (BUTTON_WIDTH / 2), 184);
	}

	if (displayType == DT_TIMING)
	{
		if (sysData.selectedSessionType == ROADRACE_TYPE)
		{
			int itmp = LISTVIEW_GetNumRows(hLapTimes);
			if (itmp <= 5) {		// search for last non-blank list element
				char timebuf[16];

				for (; itmp > 0; itmp--) {					
					LISTVIEW_GetItemText(hLapTimes, 1, itmp-1, timebuf, sizeof(timebuf));
					if (0 != strlen(timebuf)) break;
				} // for
			} // if
				
			GUI_SetColor(GUI_GREEN);
			GetBestLap(2, selectedSessionUnsorted, BestLapText);
			
			sprintf(BestLapText + strlen(BestLapText), " of %d", itmp);
			GUI_DispStringHCenterAt(BestLapText, BUTTON_LEFT + (BUTTON_WIDTH / 2), 151);
			GetBestLap(1, selectedSessionUnsorted, BestLapText);		
			GUI_DispStringHCenterAt(BestLapText, BUTTON_LEFT + (BUTTON_WIDTH / 2), 184);
		}
		else
		{
			char scratch[50];

			// Display drag race time slip
			GUI_DrawBitmap(&bmTimeSlip, 29, 49);
			GUI_SetFont(&GUI_Font20B_ASCII);
			GUI_SetColor(GUI_BLACK);

#define SESS_DETAIL_START		67
#define SESS_DETAIL_SPACING		25

			if (sysData.selectedSessionType == AUTOCROSS_TYPE)
			{
				GUI_DispStringHCenterAt("Autocross / Rally",  LEFT_CENTER, SESS_DETAIL_START + (2*SESS_DETAIL_SPACING));
				GUI_DispStringHCenterAt("Run Time",  LEFT_CENTER, SESS_DETAIL_START + (3*SESS_DETAIL_SPACING));
				GUI_DispStringHCenterAt(GetBestLapAutocross(scratch),  LEFT_CENTER, SESS_DETAIL_START + (4*SESS_DETAIL_SPACING));	// Run time
			}
			else
			{
				GUI_DispStringHCenterAt(GetSessionDataString(1, ST_PERFORMANCE, scratch),  LEFT_CENTER, SESS_DETAIL_START + (0*SESS_DETAIL_SPACING));	// Max Accel
				GUI_DispStringHCenterAt(GetSessionDataString(2, ST_PERFORMANCE, scratch),  LEFT_CENTER, SESS_DETAIL_START + (1*SESS_DETAIL_SPACING));	// 0-60, 100
				GUI_DispStringHCenterAt(GetSessionDataString(3, ST_PERFORMANCE, scratch),  LEFT_CENTER, SESS_DETAIL_START + (2*SESS_DETAIL_SPACING));	// 60'
				GUI_DispStringHCenterAt(GetSessionDataString(4, ST_PERFORMANCE, scratch),  LEFT_CENTER, SESS_DETAIL_START + (3*SESS_DETAIL_SPACING));	// 330'
				GUI_DispStringHCenterAt(GetSessionDataString(5, ST_PERFORMANCE, scratch),  LEFT_CENTER, SESS_DETAIL_START + (4*SESS_DETAIL_SPACING));	// 1/8mi
				GUI_DispStringHCenterAt(GetSessionDataString(6, ST_PERFORMANCE, scratch), LEFT_CENTER, SESS_DETAIL_START + (5*SESS_DETAIL_SPACING));	// 1000'
				GUI_DispStringHCenterAt(GetSessionDataString(7, ST_PERFORMANCE, scratch), LEFT_CENTER, SESS_DETAIL_START + (6*SESS_DETAIL_SPACING));	// 1/4mi
			}
#undef SESS_DETAIL_START
#undef SESS_DETAIL_SPACING
		}
	}

	GUI_DrawBitmap(&bmSessionDetails, 70, 0);
	GUI_DrawBitmap(&bmleftarrow, 0, 229);
}


void ReviewSessionsIgnoreNextKeyRelease(void)
{
//	clickedOnThisScreen = false;
}



/*************************** End of file ****************************/

