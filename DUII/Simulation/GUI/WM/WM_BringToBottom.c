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
File        : WM_BringToBottom.c
Purpose     : Window manager routine
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */
#include "GUI_Debug.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_BringToBottom
*/
void WM_BringToBottom(WM_HWIN hWin) {
  WM_HWIN hParent;
  WM_HWIN hPrev;
  WM_Obj* pWin;
  WM_Obj* pPrev;
  WM_Obj* pParent;
  WM_LOCK();
  if (hWin) {
    pWin = WM_H2P(hWin);
    hPrev = WM__GetPrevSibling(hWin);
    if (hPrev) {                   /* If there is no previous one, there is nothing to do ! */
      hParent = WM_GetParent(hWin);
      /* Invalidate window area */
      if (pWin->Status & WM_SF_ISVIS) {
        WM__InvalidateRectEx(&pWin->Rect, hParent, pWin->hNext);
      }
      pParent = WM_H2P(hParent);
      /* Unlink hWin */
      pPrev = WM_H2P(hPrev);
      pPrev->hNext = pWin->hNext;
      /* Link from parent (making it the first child) */
      pWin->hNext = pParent->hFirstChild;
      pParent->hFirstChild = hWin;
    }
  }
  WM_UNLOCK();
}

#else
  void WM_BringToBottom_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
