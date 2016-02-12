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
File        : WM__IsEnabled.c
Purpose     : Windows manager function
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM__IsEnabled
*/
int WM__IsEnabled(WM_HWIN hWin) {
  int r = 1;
  if ((WM_H2P(hWin)->Status) & WM_SF_DISABLED) {
    r = 0;
  }
  return r;
}

#else
  void WM__IsEnabled_c(void) {} /* avoid empty object files */
#endif  /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
