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
File        : WM_GetDesktopWindow.c
Purpose     : Implementation of WM_GetDesktopWindow
----------------------------------------------------------------------
*/

#include "WM_Intern_ConfDep.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_GetDesktopWindow
*/
WM_HWIN WM_GetDesktopWindow(void) {
  #if GUI_NUM_LAYERS == 1
    return WM__ahDesktopWin[0];
  #else
    return WM__ahDesktopWin[GUI_Context.SelLayer];
  #endif
}

#else
  void WM_GetDesktopWindow_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
