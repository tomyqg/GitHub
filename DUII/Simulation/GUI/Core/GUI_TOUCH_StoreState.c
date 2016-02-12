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
File        : GUITOUCH_StoreState.C
Purpose     : Implementation of GUITOUCH_StoreState
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static GUI_PID_STATE _State;  /* Static so we retain coordinates when touch is released */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_TOUCH_StoreState
*
* NOTE:
*   This is the older version. Prefer GUI_TOUCH_StoreStateEx in new code
*/
void GUI_TOUCH_StoreState(int x, int y) {
  if ((x >= 0) && (y >= 0)) {
    _State.Pressed = 1;
    _State.x = x;
    _State.y = y;
  } else {
    _State.Pressed = 0;
  }
  GUI_TOUCH_StoreStateEx(&_State);
}

/*********************************************************************
*
*       GUI_TOUCH_SetLayer
*/
void GUI_TOUCH_SetLayer(int Layer) {
  _State.Layer = Layer;
}

/*************************** End of file ****************************/
