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
File        : WM_ResizeWindow.C
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include <stddef.h>
#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */
#include "GUI_Debug.h"
#define WM_DEBUG_LEVEL 1

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_ResizeWindow
*/
void WM_ResizeWindow(WM_HWIN hWin, int dx, int dy) {
  GUI_RECT rOld, rNew, rMerge;
  WM_Obj* pWin;
  if (((dx | dy) == 0) || (hWin == 0)){ /* Early out if there is nothing to do */
    return;
  }
  WM_LOCK();
  pWin = WM_HANDLE2PTR(hWin);
  rOld = pWin->Rect;
  rNew = rOld;
  if (dx) {
    if ((pWin->Status & WM_SF_ANCHOR_RIGHT) && (!(pWin->Status & WM_SF_ANCHOR_LEFT))) {
      rNew.x0 -= dx;
    } else {
      rNew.x1 += dx;
    }
  }
  if (dy) {
    if ((pWin->Status & WM_SF_ANCHOR_BOTTOM) && (!(pWin->Status & WM_SF_ANCHOR_TOP))) {
      rNew.y0 -= dy;
    } else {
      rNew.y1 += dy;
    }
  }
  if (pWin->Status & WM_CF_STATIC) {
    WM__InvalidateDrawAndDescs(hWin);
  }
  pWin->Rect = rNew;
  if (pWin->Status & WM_SF_ISVIS) {
    GUI_MergeRect(&rMerge, &rOld, &rNew);
    WM__InvalidateRectEx(&rMerge, pWin->hParent, pWin->hNext);
  }
  WM__UpdateChildPositions(pWin, rNew.x0 - rOld.x0, rNew.y0 - rOld.y0, rNew.x1 - rOld.x1, rNew.y1 - rOld.y1);
  GUI__IntersectRect(&pWin->InvalidRect, &pWin->Rect); /* Make sure invalid area is not bigger than window itself */
  WM__SendMsgNoData(hWin, WM_SIZE);                    /* Send size message to the window */
  WM_UNLOCK();
}

#else
  void WM_ResizeWindow(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
