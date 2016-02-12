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
File        : LISTVIEW_SetTextColor.c
Purpose     : Implementation of LISTVIEW_SetTextColor
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_SetTextColor
*/
void LISTVIEW_SetTextColor(LISTVIEW_Handle hObj, unsigned int Index, GUI_COLOR Color) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      WM_LOCK();
      pObj = LISTVIEW_LOCK_H(hObj);
      if (Color != pObj->Props.aTextColor[Index]) {
        pObj->Props.aTextColor[Index] = Color;
        LISTVIEW__InvalidateInsideArea(hObj);
      }
      GUI_UNLOCK_H(pObj);
      WM_UNLOCK();
    }
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetTextColor_C(void);
  void LISTVIEW_SetTextColor_C(void) {}
#endif

/*************************** End of file ****************************/
