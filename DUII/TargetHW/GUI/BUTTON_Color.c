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
File        : BUTTON_Color.c
Purpose     : Implementation of button widget
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_GetBkColor  
*/
GUI_COLOR BUTTON_GetBkColor(BUTTON_Handle hObj,unsigned int Index) {
  GUI_COLOR Color = 0;
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
      Color = pObj->Props.aBkColor[Index];
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       BUTTON_SetBkColor
*/
void BUTTON_SetBkColor(BUTTON_Handle hObj,unsigned int Index, GUI_COLOR Color) {
  if (hObj) {
    BUTTON_Obj* pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
      pObj->Props.aBkColor[Index] = Color;
    }
    GUI_UNLOCK_H(pObj);
    BUTTON_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       BUTTON_GetTextColor  
*/
GUI_COLOR BUTTON_GetTextColor(BUTTON_Handle hObj,unsigned int Index) {
  GUI_COLOR Color = 0;
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      Color = pObj->Props.aTextColor[Index];
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       BUTTON_SetTextColor
*/
void BUTTON_SetTextColor(BUTTON_Handle hObj,unsigned int Index, GUI_COLOR Color) {
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      pObj->Props.aTextColor[Index] = Color;
    }
    GUI_UNLOCK_H(pObj);
    BUTTON_Invalidate(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       BUTTON_GetFrameColor
*/
GUI_COLOR BUTTON_GetFrameColor(BUTTON_Handle hObj) {
  GUI_COLOR Color = 0;
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    Color = pObj->Props.FrameColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       BUTTON_SetFrameColor
*/
void BUTTON_SetFrameColor(BUTTON_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    BUTTON_Obj * pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    pObj->Props.FrameColor = Color;
    GUI_UNLOCK_H(pObj);
    BUTTON_Invalidate(hObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_Color_C(void);
  void BUTTON_Color_C(void) {}
#endif /* GUI_WINSUPPORT */
