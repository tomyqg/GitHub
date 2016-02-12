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
File        : MENU_SetTextColor.c
Purpose     : Implementation of menu widget
---------------------------END-OF-HEADER------------------------------
*/

#include "MENU.h"
#include "MENU_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU_SetTextColor
*/
void MENU_SetTextColor(MENU_Handle hObj, unsigned ColorIndex, GUI_COLOR Color) {
  if (hObj) {
    MENU_Obj * pObj;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    if (pObj) {
      if (ColorIndex < GUI_COUNTOF(pObj->Props.aTextColor)) {
        if (Color != pObj->Props.aTextColor[ColorIndex]) {
          pObj->Props.aTextColor[ColorIndex] = Color;
          WM_InvalidateWindow(hObj);
        }
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */
  void MENU_SetTextColor_C(void) {}
#endif

/*************************** End of file ****************************/
