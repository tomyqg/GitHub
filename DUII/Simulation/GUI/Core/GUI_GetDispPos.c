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
File        : GUI_GetDispPos.c
Purpose     : Implementation of optional routines
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetDispPosX
*
* Purpose:
*   Return X-component of current display position
*/
int GUI_GetDispPosX(void) {
  int r;
  GUI_LOCK();
  r = GUI_Context.DispPosX;
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_GetDispPosY
*
* Purpose:
*   Return Y-component of current display position
*/
int GUI_GetDispPosY(void) {
  int r;
  GUI_LOCK();
  r = GUI_Context.DispPosY;
  GUI_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
