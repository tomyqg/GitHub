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
File        : CHECKBOX_SetTextColor.c
Purpose     : Implementation of CHECKBOX_SetTextColor
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
*       CHECKBOX_SetTextColor
*/
void CHECKBOX_SetTextColor(CHECKBOX_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    CHECKBOX_Obj * pObj;
    WM_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    if (pObj->Props.TextColor != Color) {
      pObj->Props.TextColor = Color;
      WM_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_SetTextColor_C(void);
  void CHECKBOX_SetTextColor_C(void) {}
#endif
