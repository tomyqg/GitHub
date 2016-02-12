/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2010  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.06 - Graphical user interface for embedded applications **
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUITOUCH_StoreUnstable.C
Purpose     : Implementation of GUITOUCH_StoreUnstable
----------------------------------------------------------------------
*/

#include <stdlib.h>

#include "GUI.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

static int _x, _y;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TOUCH_StoreUnstable
*/
void GUI_TOUCH_StoreUnstable(int x, int y) {
  int xDiff, yDiff;
  xDiff = abs (x - _x);
  yDiff = abs (y - _y);
  if (xDiff + yDiff > 2) {
    _x = x;
    _y = y;
    GUI_TOUCH_StoreState(x, y);
  }
}

/*********************************************************************
*
*       GUI_TOUCH_GetUnstable
*/
void GUI_TOUCH_GetUnstable(int* px, int* py) {
  *px = _x;
  *py = _y;
}

/*************************** End of file ****************************/
