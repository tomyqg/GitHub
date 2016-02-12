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
File        : DROPDOWN_SetColor.c
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
*       DROPDOWN_SetColor
*/
void DROPDOWN_SetColor(DROPDOWN_Handle hObj, unsigned int Index, GUI_COLOR Color) {
  DROPDOWN_Obj * pObj;
  if (hObj) {
    if (Index < GUI_COUNTOF(pObj->Props.aColor)) {
      WM_LOCK();
      pObj = DROPDOWN_LOCK_H(hObj);
      if (pObj->Props.aColor[Index] != Color) {
        pObj->Props.aColor[Index] = Color;
        GUI_UNLOCK_H(pObj);
        DROPDOWN_Invalidate(hObj);
      } else {
        GUI_UNLOCK_H(pObj);
      }
      WM_UNLOCK();
    }
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_SetColor_C(void);
  void DROPDOWN_SetColor_C(void) {}
#endif
