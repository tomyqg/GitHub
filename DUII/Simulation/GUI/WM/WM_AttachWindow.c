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
File        : WM_AttachWindow.c
Purpose     : Windows manager routines
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*         Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_DetachWindow
*/
void WM_DetachWindow(WM_HWIN hWin) {
  if (hWin) {
    WM_HWIN hParent;
    WM_Obj* pWin;
    WM_LOCK();
    pWin = WM_H2P(hWin);
    hParent = pWin->hParent;
    if (hParent) {
      WM_Obj* pParent;
      WM__DetachWindow(hWin);
      pParent = WM_H2P(hParent);
      WM_MoveWindow(hWin, -pParent->Rect.x0,  -pParent->Rect.y0);   /* Convert screen coordinates -> parent coordinates */
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       WM_AttachWindow
*/
void WM_AttachWindow(WM_HWIN hWin, WM_HWIN hParent) {
  WM_LOCK();
  if (hParent && (hParent != hWin)) {
    WM_Obj* pWin    = WM_H2P(hWin);
    WM_Obj* pParent = WM_H2P(hParent);
    if (pWin->hParent != hParent) {
      WM_DetachWindow(hWin);
      WM__InsertWindowIntoList(hWin, hParent);
      WM_MoveWindow(hWin, pParent->Rect.x0,  pParent->Rect.y0);    /* Convert parent coordinates -> screen coordinates */
      WM_InvalidateWindow(hWin);                                   /* Because WM_MoveWindow(0, 0) does not invalidate the window! */
    }
  }
  WM_UNLOCK();
}

/*********************************************************************
*
*       WM_AttachWindowAt
*/
void WM_AttachWindowAt(WM_HWIN hWin, WM_HWIN hParent, int x, int y) {
  WM_DetachWindow(hWin);
  WM_MoveTo(hWin, x, y);
  WM_AttachWindow(hWin, hParent);
}


#else
  void WM_AttachWindow_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/

