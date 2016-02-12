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
File        : WM_GetFlags.c
Purpose     : Windows manager, add. module
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
*       WM_GetFlags
*/
U16 WM_GetFlags(WM_HWIN hWin) {
  U16 r = 0;
  if (hWin) {
    WM_LOCK();
    r = WM_H2P(hWin)->Status;
    WM_UNLOCK();
  }
  return r;
}

#else                                       /* Avoid empty object files */
  void WM_GetFlags_C(void) {}
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
