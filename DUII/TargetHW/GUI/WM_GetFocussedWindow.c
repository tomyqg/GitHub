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
File        : WM_GetFocussedWindow.c
Purpose     : Implementation of said function
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_GetFocussedWindow
*/
WM_HWIN WM_GetFocussedWindow(void) {
  WM_HWIN r;
  WM_LOCK();
  r = WM__ahWinFocus[WM__TOUCHED_LAYER] ;
  WM_UNLOCK();
  return r;
}

#else
  void WM_GetFocussedWindow_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
