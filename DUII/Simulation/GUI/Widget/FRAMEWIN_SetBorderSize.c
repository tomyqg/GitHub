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
File        : FRAMEWIN.c
Purpose     : FRAMEWIN_SetBorderSize
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN_Private.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported code
*
**********************************************************************
*/

/*********************************************************************
*
*       FRAMEWIN_SetBorderSize
*/
void FRAMEWIN_SetBorderSize(FRAMEWIN_Handle hObj, unsigned Size) {
  GUI_LOCK();
  if (hObj) {
    GUI_RECT r;
    WM_Obj * pChild;
    int Diff, OldSize, OldHeight;
    WM_HWIN hChild;
    WM_HWIN hNext;
    FRAMEWIN_Obj * pObj;
    pObj      = FRAMEWIN_LOCK_H(hObj);
    OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
    OldSize   = pObj->Props.BorderSize;
    Diff      = Size - OldSize;
    for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = hNext) {
      pChild = (WM_Obj *)GUI_LOCK_H(hChild);
      r = pChild->Rect;
      GUI_MoveRect(&r, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
      if ((r.y0 == pObj->Props.BorderSize) && ((r.y1 - r.y0 + 1) == OldHeight)) {
        if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
          WM_MoveWindow(hChild, -Diff, Diff);
        } else {
          WM_MoveWindow(hChild, Diff, Diff);
        }
      }
      hNext = pChild->hNext;
      GUI_UNLOCK_H(pChild);
    }
    pObj->Props.BorderSize   = Size;
    GUI_UNLOCK_H(pObj);
    FRAMEWIN__UpdatePositions(hObj);
    FRAMEWIN_Invalidate(hObj);
  }
  GUI_UNLOCK();
}

#else
  void FRAMEWIN_SetBorderSize_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
