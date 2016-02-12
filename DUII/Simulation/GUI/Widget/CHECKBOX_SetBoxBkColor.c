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
File        : CHECKBOX_SetBoxBkColor.c
Purpose     : Implementation of CHECKBOX_SetBoxBkColor
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_SetBoxBkColor
*/
GUI_COLOR CHECKBOX_SetBoxBkColor(CHECKBOX_Handle hObj, GUI_COLOR Color, int Index) {
  GUI_COLOR OldColor = 0;
  if (hObj) {
    CHECKBOX_Obj * pObj;
    WM_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBkColorBox)) {
      if (Color != pObj->Props.aBkColorBox[Index]) {
        OldColor = pObj->Props.aBkColorBox[Index];
        pObj->Props.aBkColorBox[Index] = Color;
        WM_InvalidateWindow(hObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return OldColor;
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_SetBoxBkColor_C(void);
  void CHECKBOX_SetBoxBkColor_C(void) {}
#endif
