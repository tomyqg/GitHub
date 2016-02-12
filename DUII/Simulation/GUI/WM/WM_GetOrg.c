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
File        : WM_GetOrg.c
Purpose     : Implementation of WM_GetOrg and related functions
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
*       WM_GetWindowOrgX
*/
int WM_GetWindowOrgX(WM_HWIN hWin) {
  int r = 0;
  if (hWin) {
    WM_LOCK();
    r = WM_HANDLE2PTR(hWin)->Rect.x0;
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       WM_GetWindowOrgY
*/
int WM_GetWindowOrgY(WM_HWIN hWin) {
  int r = 0;
  if (hWin) {
    WM_LOCK();
    r = WM_HANDLE2PTR(hWin)->Rect.y0;
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       WM_GetOrgX
*/
int WM_GetOrgX(void) {
  return WM_GetWindowOrgX(GUI_Context.hAWin);
}

/*********************************************************************
*
*       WM_GetOrgY
*/
int WM_GetOrgY(void) {
  return WM_GetWindowOrgY(GUI_Context.hAWin);
}


#else
  void WM_GetOrg_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
