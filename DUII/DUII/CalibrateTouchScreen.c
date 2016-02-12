
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
#include "commondefs.h"
#include "DUII.h"
#include "GUI.h"
#include "CalibrateTouchScreen.h"
#include "FlashReadWrite.h"
#if	_WINDOWS
#include "SIMConf.h"
#else
#include "LCDConf.h"
#endif


/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/
extern GUI_CONST_STORAGE GUI_BITMAP bmtarget;

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
void MoveToNextPos(void);
void TouchScreenCallback(WM_MESSAGE * pMsg);

/*********************************************************************
*
*       Local Valiables
*
**********************************************************************
*/

int PositionTable[5][2] = {
	{ 45, 19 },
	{ 405, 19 },
	{ 405, 223 },
	{ 45, 223 },
	{ 0, 0 }
};

static int position = 0;
static int done = 0;
static WM_HWIN hBtn = 0;

void MoveToNextPos(void)
{
	if (hBtn)
		WM_DeleteWindow(hBtn);

	GUI_Clear();
	hBtn = BUTTON_CreateEx( PositionTable[position][0], PositionTable[position][1], 31, 31, 0, WM_CF_SHOW, 0, GUI_ID_USER);
	WIDGET_SetEffect(hBtn, &WIDGET_Effect_None);
	BUTTON_SetBitmap( hBtn, BUTTON_BI_UNPRESSED, &bmtarget);
		GUI_DispStringHCenterAt("Touch the target", XSIZE_PHYS/2, YSIZE_PHYS/2);
}

void TouchScreenCallback(WM_MESSAGE * pMsg)
{
	int NCode, Id;

	switch(pMsg->MsgId)
	{
	case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
      NCode = pMsg->Data.v;                 // Notification code
	   switch(NCode)
		{
		case WM_NOTIFICATION_CLICKED:
			switch (Id)
			{
				case GUI_ID_USER:
					if (PositionTable[++position][0] != 0)
						MoveToNextPos();
					else
					{
						done = TRUE;
						sysData.touchScreenCalibrated = TRUE;
						WriteDataStructures(SYSTEM_DATA);
					}
					break;
		  };
	  };
	  break;

	default:
		  WM_DefaultProc(pMsg);
	};
}

/*********************************************************************
*
*       General Config Screen
*/

void CalibrateTouchScreen(void)
{
#ifndef	_WINDOWS
	// Put touch screen controller in Calibration Mode
	if (StartTouchCalibration() == -1)
		return;		// Failed to put chip in calibration mode
#endif
	
	GUI_SetFont(&GUI_Font24_ASCII);
	MoveToNextPos();

	WM_SetCallback( WM_GetDesktopWindow(), TouchScreenCallback);

	while (!done)
	{
#ifndef	_WINDOWS
		int retval;
		
	// Wait for valid touch
		if ((retval = GetTouchPoint()) == -1)
			return;		// Error when talking to touch screen - error out
		if (retval == 1)		// If valid touch detected
		{
			if (PositionTable[++position][0] != 0)
				MoveToNextPos();
			else
			{
				sysData.touchScreenCalibrated = TRUE;
				WriteDataStructures(SYSTEM_DATA);
				return;			// Exit application
			}
		}
#endif

		GUI_Delay(10);
#ifdef _WINDOWS
		if (SIM_Tick())
			return;
#endif
	}
}

/*************************** End of file ****************************/

