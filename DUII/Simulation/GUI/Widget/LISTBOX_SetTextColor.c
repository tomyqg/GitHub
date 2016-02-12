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
File        : LISTBOX_SetTextColor.c
Purpose     : Implementation of LISTBOX_SetTextColor
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "LISTBOX_Private.h"


#if GUI_WINSUPPORT


/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       LISTBOX_SetTextColor
*/
GUI_COLOR LISTBOX_SetTextColor(LISTBOX_Handle hObj, unsigned int Index, GUI_COLOR Color) {
  GUI_COLOR r = GUI_INVALID_COLOR;
  if (hObj) {
    LISTBOX_Obj * pObj;
    if (Index < GUI_COUNTOF(pObj->Props.aBackColor)) {
      WM_LOCK();
      pObj = LISTBOX_LOCK_H(hObj);
      pObj->Props.aTextColor[Index] = Color;
      r = pObj->Props.aTextColor[Index];
      GUI_UNLOCK_H(pObj);
      LISTBOX__InvalidateInsideArea(hObj);
      WM_UNLOCK();
    }
  }
  return r;
}



#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetTextColor_C(void) {}
#endif
