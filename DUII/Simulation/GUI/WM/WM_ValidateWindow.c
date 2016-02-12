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
File        : WM_ValidateWindow.c
Purpose     : Implementation of WM_ValidateWindow
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
*       WM_ValidateWindow
*/
void WM_ValidateWindow(WM_HWIN hWin) {
  WM_Obj* pWin;
  WM_LOCK();
  if (hWin) {
    pWin = WM_HANDLE2PTR(hWin);
    if (pWin->Status & WM_SF_INVALID) {
      pWin->Status &= ~WM_SF_INVALID;
      WM__NumInvalidWindows--;
    }
  }
  WM_UNLOCK();
}

#else
  void WM_ValidateWindow_C(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
