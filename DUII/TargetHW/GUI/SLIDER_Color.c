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
File        : SLIDER_Color.c
Purpose     : Implementation of slider widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SLIDER_Private.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SLIDER_SetBkColor
*/
void SLIDER_SetBkColor(SLIDER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    pObj->Props.BkColor = Color;
    GUI_UNLOCK_H(pObj);
    #if SLIDER_SUPPORT_TRANSPARENCY
      if (Color == GUI_INVALID_COLOR) {
        WM_SetHasTrans(hObj);
      } else {
        WM_ClrHasTrans(hObj);
      }
    #endif
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_GetBkColor
*/
GUI_COLOR SLIDER_GetBkColor(SLIDER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    Color = pObj->Props.BkColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       SLIDER_SetBarColor
*/
void SLIDER_SetBarColor(SLIDER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    pObj->Props.BarColor = Color;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_GetBarColor
*/
GUI_COLOR SLIDER_GetBarColor(SLIDER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    Color = pObj->Props.BarColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       SLIDER_SetTickColor
*/
void SLIDER_SetTickColor(SLIDER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    pObj->Props.TickColor = Color;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SLIDER_GetTickColor
*/
GUI_COLOR SLIDER_GetTickColor(SLIDER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    Color = pObj->Props.TickColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       SLIDER_SetFocusColor
*/
GUI_COLOR SLIDER_SetFocusColor(SLIDER_Handle hObj, GUI_COLOR Color) {
  GUI_COLOR OldColor = 0;
  if (hObj) {
    SLIDER_Obj* pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
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

/*********************************************************************
*
*       SLIDER_GetFocusColor
*/
GUI_COLOR SLIDER_GetFocusColor(SLIDER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    SLIDER_Obj * pObj;
    WM_LOCK();
    pObj = SLIDER_LOCK_H(hObj);
    Color = pObj->Props.FocusColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

#else /* avoid empty object files */

void SLIDER_Color_C(void);
void SLIDER_Color_C(void){}

#endif  /* #if GUI_WINSUPPORT */
