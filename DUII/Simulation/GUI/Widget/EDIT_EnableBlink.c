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
File        : EDIT_EnableBlink.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_EnableBlink
*/
void EDIT_EnableBlink(EDIT_Handle hObj, int Period, int OnOff) {
  if (hObj) {
    WM_HTIMER hTimer;
    EDIT_Obj * pObj;
    WM_LOCK();
    if (OnOff) {
      hTimer = WM_CreateTimer(hObj, 0, Period, 0);
      pObj = EDIT_LOCK_H(hObj);
      pObj->hTimer = hTimer;
      GUI_UNLOCK_H(pObj);
    } else {
      pObj = EDIT_LOCK_H(hObj);
      WM_DeleteTimer(pObj->hTimer);
      pObj->hTimer = 0;
      WM_Invalidate(hObj);
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */

void EDIT_EnableBlink_C(void);
void EDIT_EnableBlink_C(void) {}

#endif /* GUI_WINSUPPORT */
