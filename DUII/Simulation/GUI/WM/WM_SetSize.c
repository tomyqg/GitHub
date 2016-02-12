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
File        : WM_SetSize.c
Purpose     : Windows manager, add. module
----------------------------------------------------------------------
*/

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
*       WM_SetSize
*/
void WM_SetSize(WM_HWIN hWin, int xSize, int ySize) {
  WM_Obj* pWin;
  int dx, dy;
  if (hWin) {
    WM_LOCK();
    pWin = WM_H2P(hWin);
    dx = xSize - (pWin->Rect.x1 - pWin->Rect.x0 + 1);
    dy = ySize - (pWin->Rect.y1 - pWin->Rect.y0 + 1);
    WM_ResizeWindow(hWin, dx, dy);
    WM_UNLOCK();
  }
}

#else
  void WM_SetSize_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
