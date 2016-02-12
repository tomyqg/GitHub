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
File        : HEADER.c
Purpose     : Implementation of header widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "HEADER_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetTextColor
*/
void HEADER_SetTextColor(HEADER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    pObj->Props.TextColor = Color;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetBkColor
*/
void HEADER_SetBkColor(HEADER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    pObj->Props.BkColor = Color;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetArrowColor
*/
void HEADER_SetArrowColor(HEADER_Handle hObj, GUI_COLOR Color) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    pObj->Props.ArrowColor = Color;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_GetTextColor
*/
GUI_COLOR HEADER_GetTextColor(HEADER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Color = pObj->Props.TextColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       HEADER_GetBkColor
*/
GUI_COLOR HEADER_GetBkColor(HEADER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Color = pObj->Props.BkColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

/*********************************************************************
*
*       HEADER_GetArrowColor
*/
GUI_COLOR HEADER_GetArrowColor(HEADER_Handle hObj) {
  GUI_COLOR Color;
  Color = 0;
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Color = pObj->Props.ArrowColor;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Color;
}

#else /* avoid empty object files */

void HEADER_Color_C(void);
void HEADER_Color_C(void){}

#endif  /* #if GUI_WINSUPPORT */
