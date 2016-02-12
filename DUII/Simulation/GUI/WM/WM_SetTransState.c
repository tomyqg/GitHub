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
File        : WM_SetTransState.c
Purpose     : Implementation of WM_SetTransState
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT && WM_SUPPORT_TRANSPARENCY

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

void WM_SetTransState(WM_HWIN hWin, unsigned State) {
  WM_Obj *pWin;
  WM_LOCK();
  if (hWin) {
    pWin = WM_H2P(hWin);
    if (State & WM_CF_HASTRANS) {
      WM_SetHasTrans(hWin);
    } else {
      WM_ClrHasTrans(hWin);
    }
    if (State & WM_CF_CONST_OUTLINE) {
      if (!(pWin->Status & WM_CF_CONST_OUTLINE)) {
        pWin->Status |= WM_CF_CONST_OUTLINE;
        WM_InvalidateWindow(hWin);
      }
    } else {
      if (pWin->Status & WM_CF_CONST_OUTLINE) {
        pWin->Status &= ~WM_CF_CONST_OUTLINE;
        WM_InvalidateWindow(hWin);
      }
    }
  }
  WM_UNLOCK();
}

#else
  void WM_SetTransState_c(void);
  void WM_SetTransState_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/

