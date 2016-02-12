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
File        : FRAMEWIN__UpdateButtons.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

#if GUI_WINSUPPORT


/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN__UpdateButtons
*
* Purpose:
*   Adjust button positions & size.
*   This is required after resizing the title bar
*/
void FRAMEWIN__UpdateButtons(FRAMEWIN_Handle hObj, int OldHeight, int OldBorderSizeL, int OldBorderSizeR, int OldBorderSizeT) {
  FRAMEWIN_Obj * pObj;
  WM_HWIN hChild;
  WM_Obj * pChild;
  int Height, BorderSizeL, BorderSizeR, BorderSizeT, Diff, nr, nl;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;

  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  Height = FRAMEWIN__CalcTitleHeight(pObj);
  GUI_UNLOCK_H(pObj);
  //
  // Get current values
  //
  BorderSizeL = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_L);
  BorderSizeR = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_R);
  BorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
  //
  // Check if something to do
  //
  if ((Height      != OldHeight) || 
      (BorderSizeL != OldBorderSizeL) || 
      (BorderSizeR != OldBorderSizeR) || 
      (BorderSizeT != OldBorderSizeT)) {
    Diff = Height - OldHeight;
    nr = nl = 0;
    hChild = WM_GetFirstChild(hObj);
    while (hChild) {
      pChild = (WM_Obj *)GUI_LOCK_H(hChild);
      if (((pChild->Status & WM_SF_ANCHOR_RIGHT) && ((pChild->Status & WM_SF_ANCHOR_LEFT ) == 0)) ||
          ((pChild->Status & WM_SF_ANCHOR_LEFT ) && ((pChild->Status & WM_SF_ANCHOR_RIGHT) == 0))) {
        if (Diff) {
          WM_ResizeWindow(hChild, Diff, Diff);
        }
        if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
          WM_MoveWindow(hChild, OldBorderSizeR - BorderSizeR
                                - Diff * nr++,
                                BorderSizeT - OldBorderSizeT);
        }
        if (pChild->Status & WM_SF_ANCHOR_LEFT) {
          WM_MoveWindow(hChild, BorderSizeL - OldBorderSizeL
                                + Diff * nl++,
                                BorderSizeT - OldBorderSizeT);
        }
      }
      GUI_UNLOCK_H(pChild);
      hChild = WM_GetNextSibling(hChild);
    }
  }
}

#else
  void FRAMEWIN__UpdateButtons_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
