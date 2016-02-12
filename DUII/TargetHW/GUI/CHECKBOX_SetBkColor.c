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
File        : CHECKBOX_SetBkColor.c
Purpose     : Implementation of CHECKBOX_SetBkColor
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
*       CHECKBOX_SetBkColor
*/
void CHECKBOX_SetBkColor(CHECKBOX_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    CHECKBOX_Obj* pObj;
    WM_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    if (Color != pObj->Props.BkColor) {
      pObj->Props.BkColor = Color;
      #if WM_SUPPORT_TRANSPARENCY
        if (Color <= 0xFFFFFF) {
          WM_SetTransState(hObj, 0);
        } else {
          WM_SetTransState(hObj, WM_CF_HASTRANS);
        }
      #endif
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_SetBkColor_C(void);
  void CHECKBOX_SetBkColor_C(void) {}
#endif
