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
File        : FRAMEWIN_SetColors.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Exported routines:  Set Properties
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_SetBarColor
*/
void FRAMEWIN_SetBarColor(FRAMEWIN_Handle hObj, unsigned Index, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBarColor)) {
      pObj->Props.aBarColor[Index] = Color;
      FRAMEWIN_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetTextColor
*/
void FRAMEWIN_SetTextColor(FRAMEWIN_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    int i;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
      pObj->Props.aTextColor[i] = Color;
    }
    GUI_UNLOCK_H(pObj);
    FRAMEWIN_Invalidate(hObj);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetTextColorEx
*/
void FRAMEWIN_SetTextColorEx(FRAMEWIN_Handle hObj, unsigned Index, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      pObj->Props.aTextColor[Index] = Color;
      FRAMEWIN_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetClientColor
*/
void FRAMEWIN_SetClientColor(FRAMEWIN_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (pObj->Props.ClientColor != Color) {
      pObj->Props.ClientColor = Color;
      FRAMEWIN_Invalidate(pObj->hClient);
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

#else
  void FRAMEWIN_SetColors_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
