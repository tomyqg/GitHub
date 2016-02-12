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
File        : FRAMEWIN_SetTitleVis.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*           Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _ShowHideButtons
*/
static void _ShowHideButtons(FRAMEWIN_Obj * pObj) {
  WM_HWIN hChild;
  WM_HWIN hNext;
  WM_Obj * pChild;
  int y0;
  for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = hNext) {
    pChild = (WM_Obj *)GUI_LOCK_H(hChild);
    hNext = pChild->hNext;
    y0 = pChild->Rect.y0 - pObj->Widget.Win.Rect.y0;
    GUI_UNLOCK_H(pChild);
    if ((y0 == pObj->Props.BorderSize) && (hChild != pObj->hClient)) {
      if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
        WM_ShowWindow(hChild);
      } else {
        WM_HideWindow(hChild);
      }
    }
  }
}

/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/

/*********************************************************************
*
*       FRAMEWIN_SetTitleVis
*/
void FRAMEWIN_SetTitleVis(FRAMEWIN_Handle hObj, int Show) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    int State;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    State = pObj->Widget.State;
    if (Show) {
      State |= FRAMEWIN_SF_TITLEVIS;
    } else {
      State &= ~FRAMEWIN_SF_TITLEVIS;
    }
    if (pObj->Widget.State != State) {
      pObj->Widget.State = State;
      FRAMEWIN__UpdatePositions(hObj);
      _ShowHideButtons(pObj);
      if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
        if (State & FRAMEWIN_SF_TITLEVIS) {
          WM_ShowWindow(hObj);
        } else {
          WM_HideWindow(hObj);
        }
      }
      FRAMEWIN_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else
  void FRAMEWIN_SetTitleVis_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
