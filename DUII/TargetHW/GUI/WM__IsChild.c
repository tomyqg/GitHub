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
File        : WM__IsChild.c
Purpose     : Implementation of WM__IsChild
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
*       WM__IsChild
*/
int WM__IsChild(WM_HWIN hWin, WM_HWIN hParent) {
  int r = 0;
  if (hWin) {
    WM_Obj *pObj = WM_H2P(hWin);
    if (pObj) {
      if (pObj->hParent == hParent) {
        r = 1;
      }
    }
  }
  return r;
}

#else
  void WM__IsChild_c(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
