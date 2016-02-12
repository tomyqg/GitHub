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
File        : WM_EnableWindow.c
Purpose     : Implementation of WM_EnableWindow, WM_DisableWindow
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
*       WM_SetEnableState
*/
void WM_SetEnableState(WM_HWIN hWin, int State) {
  if (hWin) {
    WM_Obj* pWin;
    U16 Status;
    WM_LOCK();
    pWin = WM_H2P(hWin);
    Status = pWin->Status;
    if (State) {
      Status &= ~WM_SF_DISABLED;
    } else {
      Status |=  WM_SF_DISABLED;
    }
    if (pWin->Status != Status) {
      WM_MESSAGE Msg;
      pWin->Status = Status;
      Msg.MsgId  = WM_NOTIFY_ENABLE;
      Msg.Data.v = State;
      WM__SendMessage(hWin, &Msg);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       WM_EnableWindow
*/
void WM_EnableWindow(WM_HWIN hWin) {
  WM_SetEnableState(hWin, 1);
}

/*********************************************************************
*
*       WM_DisableWindow
*/
void WM_DisableWindow(WM_HWIN hWin) {
  WM_SetEnableState(hWin, 0);
}
#else
  void WM_EnableWindow_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
