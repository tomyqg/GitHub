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
File        : RADIO_SetFocusColor.c
Purpose     : Implementation of RADIO_SetFocusColor
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_SetFocusColor
*/
GUI_COLOR RADIO_SetFocusColor(RADIO_Handle hObj, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  if (hObj) {
    RADIO_Obj* pObj;
    WM_LOCK();
    pObj = RADIO_LOCK_H(hObj);
    if (Color != pObj->Props.FocusColor) {
      OldColor = pObj->Props.FocusColor;
      pObj->Props.FocusColor = Color;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return OldColor;
}

#else                            /* Avoid problems with empty object modules */
  void RADIO_SetFocusColor_C(void);
  void RADIO_SetFocusColor_C(void) {}
#endif
