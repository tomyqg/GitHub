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
File        : WM_GetDesktopWindowEx.c
Purpose     : Implementation of WM_GetDesktopWindowEx
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
*       WM_GetDesktopWindowEx
*/
WM_HWIN WM_GetDesktopWindowEx(unsigned int LayerIndex) {
  WM_HWIN r = WM_HWIN_NULL;
  if (LayerIndex < GUI_NUM_LAYERS) {
    r = WM__ahDesktopWin[LayerIndex];
  }
  return r;
}


#else
  void WM_GetDesktopWindowEx_c(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
