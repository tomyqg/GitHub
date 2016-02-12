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
File        : WM__GetFirstSibling.c
Purpose     : Implementation of WM__GetFirstSibling
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
*       WM__GetFirstSibling

  Return value: Handle of parent, 0 if none
*/
WM_HWIN WM__GetFirstSibling(WM_HWIN hWin) {
  hWin = WM_GetParent(hWin);
  return (hWin) ? WM_HANDLE2PTR(hWin)->hFirstChild : 0;
}

#else
  void WM__GetFirstSibling_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
