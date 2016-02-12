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
File        : WM_GetInvalidRect.c
Purpose     : Windows manager, submodule
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       WM_GetInvalidRect
*
* Return value:
*   0 if window is valid (there is no invalid rectangle)
*   1 if there is an invalid rectangle
*   
*/
int WM_GetInvalidRect(WM_HWIN hWin, GUI_RECT * pRect) {
  int IsInvalid = 0;
  if (hWin) {
    WM_Obj * pWin;
    WM_LOCK();
    pWin = WM_HANDLE2PTR(hWin);
    if (pWin->Status & WM_SF_INVALID) {
      IsInvalid = 1;
      *pRect = pWin->InvalidRect;
    }
    WM_UNLOCK();
  }
  return IsInvalid;
}

#else
  void WM_GetInvalidRect_C(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
