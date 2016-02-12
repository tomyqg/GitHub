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
File        : WM_MoveChildTo.C
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public API code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_MoveChildTo
*/
void WM_MoveChildTo(WM_HWIN hWin, int x, int y) {
  if (hWin) {
    WM_HWIN hParent;
    WM_LOCK();
    hParent = WM_GetParent(hWin);
    if (hParent) {
      WM_Obj * pParent, * pWin;
      pParent = WM_HANDLE2PTR(hParent);
      pWin    = WM_HANDLE2PTR(hWin);
      x -= pWin->Rect.x0 - pParent->Rect.x0;
      y -= pWin->Rect.y0 - pParent->Rect.y0;
      WM__MoveWindow(hWin, x, y);
    }
    WM_UNLOCK();
  }
}

#else
  void WM_MoveChildTo_c(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
