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
#include "DataHandler.h"
#include "RaceSetupScreens.h"

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmSystemSetup;
extern GUI_CONST_STORAGE GUI_BITMAP bmGoRacing;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceSetup;
extern GUI_CONST_STORAGE GUI_BITMAP bmRaceReview;
extern GUI_CONST_STORAGE GUI_BITMAP bmBackground;
extern GUI_CONST_STORAGE GUI_BITMAP bmTraqmateLogo;
#ifdef	WITH_BANNER
extern GUI_CONST_STORAGE GUI_BITMAP bmBannerBack;
#endif
extern int suspendForHelp;


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
*       Local Defines
*
**********************************************************************
*/
#define	GORACING_BUTTON		GUI_ID_USER + 1
#define	RACEREVIEW_BUTTON	GUI_ID_USER + 2
#define	SETUP_BUTTON		GUI_ID_USER + 3
#define	SYSTEMSETUP_BUTTON	GUI_ID_USER + 4

/*********************************************************************
*
*       Local Valiables
*
**********************************************************************
*/
static WM_HWIN hWnd;
int glbErrorFlag = 0;
static int clickedOnThisScreen;

const GUI_RECT BannerRect = { 0, 0, XSIZE_PHYS, 35 };
const GUI_RECT InfoTimeRect = { 0, 0, 80, 35 };
const GUI_RECT BannerTimeRect = { 42, 9, 42+80, 9+22 };


//int ButtonIDs[] = {GO_RACING, REVIEW_SESSIONS, RACE_SETUP, SYSTEM_SETUP};

WM_HWIN GetGeneralConfinScreenWindowHandle(void)
{
	return hWnd;
}

static void _cbGeneralConfigCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;
	//int Sel;

	switch(pMsg->MsgId)
	{
	case WM_TIMER:
		if (!suspendForHelp)
			if (pMsg->Data.v == hScreenTimer)
			{
				WM_RestartTimer(hScreenTimer, SETUP_SCREEN_REFRESH_TIME);
				WM_InvalidateRect(pMsg->hWin, &BannerRect);
			}
		break;

	case WM_PAINT:
		if (!suspendForHelp)
		{
			/* Paint the background window */
			GUI_DrawBitmap(&bmBackground, 0, 0);
#ifdef	WITH_BANNER
			GUI_DrawBitmap(&bmBannerBack, 0, 0);
#endif
			GUI_DrawBitmap(&bmTraqmateLogo, 148, 6);
			DrawSignalStrength();
		}
		break;

	case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
      NCode = pMsg->Data.v;                 // Notification code
	   switch(NCode)
		{
			case WM_NOTIFICATION_CLICKED:
				clickedOnThisScreen = TRUE;
				break;

			case WM_NOTIFICATION_RELEASED:
				if (clickedOnThisScreen && !processingScreenTransition)
					switch (Id)
					{
						case GORACING_BUTTON:
							processingScreenTransition = true;		// This will be moved into DisplayNewScreenList when it is proven to work
							DisplayNewScreenList(CONFIG_SCREEN_TYPE, GO_RACING, hWnd);
							break;

						case RACEREVIEW_BUTTON:
							// Always initially display default screen (timing) when coming in "from the top"
							InitReviewDisplayTypes();
#ifdef _WINDOWS
							processingScreenTransition = true;
							DisplayNewScreenList(CONFIG_SCREEN_TYPE, REVIEW_SESSIONS, hWnd);
#else
							if (unsavedSysData.sdCardInserted)
							{
								processingScreenTransition = true;
								DisplayNewScreenList(CONFIG_SCREEN_TYPE, REVIEW_SESSIONS, hWnd);
							}
							else NotSupportedPopup(hWnd);
#endif							
							break;

						case SETUP_BUTTON:
							processingScreenTransition = true;
							DisplayNewScreenList(CONFIG_SCREEN_TYPE, RACE_SETUP, hWnd);
							break;

						case SYSTEMSETUP_BUTTON:
							processingScreenTransition = true;
							DisplayNewScreenList(CONFIG_SCREEN_TYPE, SYSTEM_SETUP, hWnd);
							break;
					}
				break;
	  };
	  break;

	case WM_DELETE:
		processingScreenTransition = false;
		break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       General Config Screen
*/

void Display_GeneralConfig(WM_HWIN hPrev)
{
BUTTON_Handle hButton;

	if (hPrev)
	{
		WM_DeleteWindow(hPrev);
	}

	screen_scrolling = HORIZONTAL_ONLY;

	GeneralConfigIgnoreNextKeyRelease();

	SetScreenType(GENERAL_CONFIG_SCREEN_TYPE);

	if ((hWnd = WM_CreateWindow(0, 0, XSIZE_PHYS, YSIZE_PHYS, WM_CF_SHOW | WM_CF_MEMDEV_ON_REDRAW, _cbGeneralConfigCallback, 0)) <= 0)
		SystemError();

	hButton = BUTTON_CreateEx(41, 38, 178, 115, hWnd, WM_CF_SHOW, 0, GORACING_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmGoRacing);

	hButton = BUTTON_CreateEx(261, 38, 178, 114, hWnd, WM_CF_SHOW, 0, RACEREVIEW_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRaceReview);

	hButton = BUTTON_CreateEx(41, 156, 178, 115, hWnd, WM_CF_SHOW, 0, SETUP_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmRaceSetup);

	hButton = BUTTON_CreateEx(261, 156, 178, 115, hWnd, WM_CF_SHOW, 0, SYSTEMSETUP_BUTTON);
	BUTTON_SetBitmap(hButton, BUTTON_BI_UNPRESSED, &bmSystemSetup);

	hScreenTimer = WM_CreateTimer(hWnd, 0, SETUP_SCREEN_REFRESH_TIME, 0);
	ResetNewScreen();				// Inform title bar that this is a new screen
}

void SystemError(void)
{
	glbErrorFlag = true;
}

void GeneralConfigIgnoreNextKeyRelease(void)
{
	clickedOnThisScreen = false;
}


/*************************** End of file ****************************/

