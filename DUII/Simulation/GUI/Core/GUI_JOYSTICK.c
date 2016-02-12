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
File        : GUI_JOYSTICK.c
Purpose     : Generic joystick routines
----------------------------------------------------------------------
*/

#include <string.h>           /* memcmp */

#include "GUI.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_JOYSTICK_StoreState
*/
void GUI_JOYSTICK_StoreState(const GUI_PID_STATE * pState) {
  static GUI_PID_STATE StatePrev;
  static int TimeAcc;
  int IsDifferent;

  IsDifferent = memcmp(pState, &StatePrev, sizeof(GUI_PID_STATE));
  /*
  * Handle dynamic pointer acceleration
  */
  if (IsDifferent == 0) {
    if (TimeAcc < 10) {
      TimeAcc++;
    }
  } else {
    TimeAcc = 1;
  }
  if ((pState->x || pState->y || pState->Pressed) || IsDifferent) {
    GUI_PID_STATE State;
    int Max;
    /*
    * Compute the new coordinates
    */
    GUI_PID_GetState(&State);
    if (pState->x < 0) {
      State.x -= TimeAcc;
    }
    if (pState->x > 0) {
      State.x += TimeAcc;
    }
    if (pState->y < 0) {
      State.y -= TimeAcc;
    }
    if (pState->y > 0) {
      State.y += TimeAcc;
    }
    State.Pressed = pState->Pressed;
    /*
    * Make sure coordinates are still in bounds
    */
    if (State.x < 0) {
      State.x = 0;
    }
    if (State.y < 0) {
      State.y = 0;
    }
    Max = LCD_GetXSize() - 1;
    if (State.x >= Max) {
      State.x = Max;
    }
    Max = LCD_GetYSize() - 1;
    if (State.y > Max) {
      State.y = Max;
    }
    /*
    * Inform emWin
    */
    GUI_PID_StoreState(&State);
    StatePrev = *pState;
  }
}

/*************************** End of file ****************************/
