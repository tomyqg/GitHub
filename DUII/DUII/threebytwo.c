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

#include "GUI.h"
#include "BUTTON.h"
#include "commondefs.h"
#include "LCDConf.h"
#include "DIALOG.h"
#include "threebytwo.h"

/*********************************************************************
*
*       External data and code
*
**********************************************************************
*/

extern int ShowDialog(const char *sTitle, const char *sText, int, int);

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
void StartAutoLand(void);
void ProcessScreenMovement(void);
//void ProcessAutoLand(void);


#define	NOT_MOVING	0
#define MOVING_IN_X	1
#define	MOVING_IN_Y	2


int buttoncount;
static signed int x_origin = 0;
static signed int y_origin = 0;
static int previous_touch_state;
static signed int x_increment = 0;
static signed int y_increment = 0;
static signed int x_target = 0;
static signed int y_target = 0;
static int glb_results;
static int KMCDialogSemaphone = 0;
static int current_movement = NOT_MOVING;

static void _cbMainCallback(WM_MESSAGE * pMsg) {
	int NCode, Id;
	int results;

	switch(pMsg->MsgId)
	{
	case WM_PAINT:
		/* Paint the background window */
		GUI_SetBkColor(GUI_BLACK);
		GUI_Clear();
		GUI_SetColor(GUI_GREEN);
		GUI_SetFont(&GUI_Font24_ASCII);

		GUI_DispStringAt("Screen 1", 0, 0);
		GUI_DispStringAt("Screen 2", XSIZE_PHYS, 0);
		GUI_DispStringAt("Screen 3", XSIZE_PHYS*2, 0);
		GUI_DispStringAt("Screen 4", 0, YSIZE_PHYS);
		GUI_DispStringAt("Screen 5", XSIZE_PHYS, YSIZE_PHYS);
		GUI_DispStringAt("Screen 6", XSIZE_PHYS*2, YSIZE_PHYS);
		break;

	case WM_NOTIFY_PARENT:
      Id    = WM_GetId(pMsg->hWinSrc);      // Id of widget
      NCode = pMsg->Data.v;                 // Notification code
	  switch(NCode)
	  {
		case WM_NOTIFICATION_CLICKED:
		  switch (Id) {
			case GUI_ID_BUTTON1:
				GUI_SetOrg(x_origin = XSIZE_PHYS, y_origin = 0);
				break;

			case GUI_ID_BUTTON2:
				GUI_SetOrg(x_origin = XSIZE_PHYS*2, y_origin = 0);
				break;

			case GUI_ID_BUTTON3:
				GUI_SetOrg(x_origin = 0, y_origin = YSIZE_PHYS);
				break;

			case GUI_ID_BUTTON4:
				GUI_SetOrg(x_origin = XSIZE_PHYS, y_origin = YSIZE_PHYS);
				break;

			case GUI_ID_BUTTON5:
				GUI_SetOrg(x_origin = XSIZE_PHYS*2, y_origin = YSIZE_PHYS);
				break;

			case GUI_ID_BUTTON6:
				GUI_SetOrg(x_origin = 0, y_origin = 0);
				break;

			//case GUI_ID_BUTTON7:
			//	if (!KMCDialogSemaphone)
			//	{
			//		KMCDialogSemaphone = 1;

			//		results = ShowDialog("KMC TEST", "Continue", x_origin, y_origin);
			//		if (results == 1)		// 1 == YES but I'm not including DU_2.h yet (keeping test case simple)
			//			glb_results = 1;
			//		else glb_results = 2;

			//		KMCDialogSemaphone = 0;
			//	}
			//	break;
		  };
	  };
	  break;
 	default:
		  WM_DefaultProc(pMsg);
	};
}

void StartAutoLand(void)
{
	if (current_movement == MOVING_IN_X)
//	if (x_origin % XSIZE_PHYS)
	{
		if (x_origin <= 0)
		{
			x_origin = 0;
			x_increment = 0;
		}
		else if (x_origin <= XSIZE_PHYS/2)
		{
			x_increment = -5;
			x_target = 0;
		}
		else if ((x_origin >= XSIZE_PHYS/2) && (x_origin <= XSIZE_PHYS))
		{
			x_increment = 5;
			x_target = XSIZE_PHYS;
		}
		else if ((x_origin >= XSIZE_PHYS) && (x_origin <= XSIZE_PHYS + XSIZE_PHYS/2))
		{
			x_increment = -5;
			x_target = XSIZE_PHYS;
		}
		else if ((x_origin >= XSIZE_PHYS + XSIZE_PHYS/2) && (x_origin <= XSIZE_PHYS*2))
		{
			x_increment = 5;
			x_target = XSIZE_PHYS*2;
		}
		else
		{
			x_increment = 0;
		}
	}
	else
	{
		x_increment = 0;
	}

	if (current_movement == MOVING_IN_Y)
//	if (y_origin % YSIZE_PHYS)
	{
		if (y_origin < YSIZE_PHYS/2)
		{
			y_increment = -5;
			y_target = 0;
		}
		else if ((y_origin >= YSIZE_PHYS/2) && (y_origin <= YSIZE_PHYS))
		{
			y_increment = 5;
			y_target = YSIZE_PHYS;
		}
		else
		{
			y_increment = 0;
		}
	}
	else
	{
		y_increment = 0;
	}
}

void ProcessScreenMovement(void)
{
	GUI_PID_STATE TouchState;
	signed int xPhys, yPhys, x, y, i, j;
	static int signed lastx, lasty;

	GUI_TOUCH_GetState(&TouchState);  /* Get the touch position in pixel */

	if (TouchState.Pressed)			// If touch pressed
	{
		xPhys = GUI_TOUCH_GetxPhys();     /* Get the A/D mesurement result in x */
		yPhys = GUI_TOUCH_GetyPhys();     /* Get the A/D mesurement result in y */

		/* Display the new position */
		GUI_SetColor(GUI_RED);
		GUI_SetFont(&GUI_Font32_ASCII);

		// WARNING - carriage returns WRAP to y=0 - we do not want this if we are displaying
		// on virtual screens
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 3; j++)
			{
				GUI_DispStringAt("Position:", 10 + (j * XSIZE_PHYS), 140 + (i * YSIZE_PHYS));
				GUI_DispString("x:");
				GUI_DispDec(x = TouchState.x,4);
				GUI_DispString(", y:");
				GUI_DispDec(y = TouchState.y,4);
			}
		}
		x = TouchState.x;
		y = TouchState.y;

		// If any movement since last pass
		if ((x != lastx) || (y != lasty))
		{
			// If this is a valid position (ie all positive coordinates)
			if ((x > 0) && (y > 0) && (lastx > 0) && (lasty > 0))
			{
				if (current_movement == NOT_MOVING)
				{
					// See if we are moving more in the X or Y direction
					if (abs(lastx - x) > abs(lasty - y))
					{
						current_movement = MOVING_IN_X;
					}
					else
					{
						current_movement = MOVING_IN_Y;
					}
				}

				if (current_movement == MOVING_IN_X)
				{
					x_origin += lastx - x;
					if (x_origin < 0)
					{
						x_origin = 0;
						current_movement = NOT_MOVING;
					}
					else if (x_origin > XSIZE_PHYS*2)
					{
						x_origin = XSIZE_PHYS*2;
						current_movement = NOT_MOVING;
					}
				}
				else
				{
					y_origin += lasty - y;
					if (y_origin < 0)
					{
						y_origin = 0;
						current_movement = NOT_MOVING;
					}
					else if (y_origin > YSIZE_PHYS)
					{
						y_origin = YSIZE_PHYS;
						current_movement = NOT_MOVING;
					}
				}

				GUI_SetOrg(x_origin,y_origin);
			}
		}
		lastx = x;
		lasty = y;

		previous_touch_state = 1;
	}

	// Else touch screen not pressed
	else
	{
		if (previous_touch_state == 1)	// if were just touching
		{
			previous_touch_state = 0;	// Calculate position for auto continue
			lastx = lasty = 0;
			StartAutoLand();
		}
	}

	// Check for and process auto movement
	if (x_increment)
	{
		x_origin += x_increment;
		if (abs(x_origin - x_target) < abs(x_increment))
		{
			x_origin = x_target;
			x_increment = 0;
			current_movement = NOT_MOVING;
		}
		GUI_SetOrg(x_origin,y_origin);
	}

	if (y_increment)
	{
		y_origin += y_increment;
		if (abs(y_origin - y_target) < abs(y_increment))
		{
			y_origin = y_target;
			y_increment = 0;
			current_movement = NOT_MOVING;
		}
		GUI_SetOrg(x_origin,y_origin);
	}
}

/*********************************************************************
*
*       MainTask
*/
void Display_3by2(void) {
WM_HWIN hBtn;
//int count = 0;

	WM_SetCallback( WM_GetDesktopWindow(), _cbMainCallback);

	hBtn = BUTTON_CreateEx( 50, 80, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON1);
	BUTTON_SetText(hBtn, "Go To Page 2");

	hBtn = BUTTON_CreateEx( 50 + XSIZE_PHYS, 80, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON2);
	BUTTON_SetText(hBtn, "Go To Page 3");

	hBtn = BUTTON_CreateEx( 50 + XSIZE_PHYS*2, 80, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON3);
	BUTTON_SetText(hBtn, "Go To Page 4");

	hBtn = BUTTON_CreateEx( 50, 80 + YSIZE_PHYS, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON4);
	BUTTON_SetText(hBtn, "Go To Page 5");

	hBtn = BUTTON_CreateEx( 50 + XSIZE_PHYS, 80 + YSIZE_PHYS, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON5);
	BUTTON_SetText(hBtn, "Go To Page 6");

	hBtn = BUTTON_CreateEx( 50 + XSIZE_PHYS*2, 80 + YSIZE_PHYS, 100, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON6);
	BUTTON_SetText(hBtn, "Go To Page 1");

	// Put another button to show a popup on page 5
	hBtn = BUTTON_CreateEx( 50 + XSIZE_PHYS, 180 + YSIZE_PHYS, 200, 30, 0, WM_CF_SHOW, 0, GUI_ID_BUTTON7);
	BUTTON_SetText(hBtn, "Pop up a dialog box");

	BUTTON_SetDefaultBkColor(GUI_BLACK, BUTTON_CI_UNPRESSED);
	BUTTON_SetDefaultTextColor(GUI_WHITE, BUTTON_CI_UNPRESSED);
	
	FRAMEWIN_SetDefaultBarColor(1, GUI_BLACK);
	FRAMEWIN_SetDefaultBorderSize(2);
	FRAMEWIN_SetDefaultClientColor(GUI_BLACK);
	FRAMEWIN_SetDefaultTextAlign(GUI_TA_HCENTER);
	FRAMEWIN_SetDefaultTitleHeight(20);
	FRAMEWIN_SetDefaultFont(&GUI_Font16_ASCII);

	GUI_SetOrg(0,0);
}

/*************************** End of file ****************************/

