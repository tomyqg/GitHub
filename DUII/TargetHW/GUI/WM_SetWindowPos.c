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
File        : WM_SetWindowPos.c
Purpose     : Implementation of WM_SetWindowPos
----------------------------------------------------------------------
*/

#include "WM.h"

#if GUI_WINSUPPORT    /* If 0, WM will not generate any code */

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_SetWindowPos
*/
void WM_SetWindowPos(WM_HWIN hWin, int xPos, int yPos, int xSize, int ySize) {
  WM_SetSize(hWin, xSize, ySize);
  WM_MoveTo(hWin, xPos, yPos);
}

#else
  void WM_SetWindowPos_C(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
