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
File        : WM__Screen2Client.c
Purpose     : Implementation of WM__Screen2Client
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
*       WM__Screen2Client
*/
void WM__Screen2Client(const WM_Obj* pWin, GUI_RECT *pRect) {
  GUI_MoveRect(pRect, -pWin->Rect.x0, -pWin->Rect.y0);
}

#else
  void WM__Screen2Client_c(void);
  void WM__Screen2Client_c(void) {} /* avoid empty object files */
#endif

/*************************** End of file ****************************/
