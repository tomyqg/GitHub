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
File        : EDIT_GetCursorPixelPos.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_GetCursorPixelPos
*/
void EDIT_GetCursorPixelPos(EDIT_Handle hObj, int * pxPos, int * pyPos) {
  if (hObj) {
    int NumChars;
    EDIT_Obj * pObj;
    const char GUI_UNI_PTR * pText;
    GUI_RECT rInside, rText;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    if (pObj->hpText) {
      GUI_SetFont(pObj->Props.pFont);
      WIDGET__GetInsideRect(&pObj->Widget, &rInside);
      pText       = (const char *)GUI_LOCK_H(pObj->hpText);
      NumChars    = GUI__GetNumChars(pText);
      rInside.x0 += pObj->Props.Border + EDIT_XOFF;
      rInside.x1 -= pObj->Props.Border + EDIT_XOFF;
      GUI__CalcTextRect(pText, &rInside, &rText, pObj->Props.Align);
      rText.x0 += GUI__GetCursorPosX(pText, pObj->CursorPos, NumChars);
      *pxPos = rText.x0;
      *pyPos = rText.y0;
      GUI_UNLOCK_H(pText);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */

void EDIT_GetCursorPixelPos_C(void);
void EDIT_GetCursorPixelPos_C(void) {}

#endif /* GUI_WINSUPPORT */


