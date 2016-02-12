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
File        : DROPDOWN_SetBkColor.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX.h"
#include "DROPDOWN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_SetBkColor
*/
void DROPDOWN_SetBkColor(DROPDOWN_Handle hObj, unsigned int Index, GUI_COLOR Color) {
  DROPDOWN_Obj * pObj;
  WM_HWIN hListWin;
  if (hObj) {
    if (Index < GUI_COUNTOF(pObj->Props.aBackColor)) {
      WM_LOCK();
      pObj = DROPDOWN_LOCK_H(hObj);
      hListWin = pObj->hListWin;
      if (pObj->Props.aBackColor[Index] != Color) {
        pObj->Props.aBackColor[Index] = Color;
        GUI_UNLOCK_H(pObj);
        DROPDOWN_Invalidate(hObj);
        if (hListWin) {
          LISTBOX_SetBkColor(hListWin, Index, Color);
        }
      } else {
        GUI_UNLOCK_H(pObj);
      }
      WM_UNLOCK();
    }
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_SetBkColor_C(void);
  void DROPDOWN_SetBkColor_C(void) {}
#endif
