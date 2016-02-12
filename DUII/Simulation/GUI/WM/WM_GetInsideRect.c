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
File        : WM_GetInsideRect.c
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
*       WM_GetInsideRectEx
*
  Purpose:
    Return the inside rectangle in client coordinates.
    The inside rectangle is the client rectangle minus the effect,
    which typically reduces the rectangle by 0 - 3 pixels on either side
    (2 for the standard 3D effect).
*/        
void WM_GetInsideRectEx(WM_HWIN hWin, GUI_RECT* pRect) {
  WM_MESSAGE Msg;
  Msg.Data.p = pRect;
  Msg.MsgId  = WM_GET_INSIDE_RECT;
  WM_SendMessage(hWin, &Msg);
}

/*********************************************************************
*
*       WM_GetInsideRect
*/
void WM_GetInsideRect(GUI_RECT* pRect) {
  WM_GetInsideRectEx(GUI_Context.hAWin, pRect);
}

#else
  void WM_GetInsideRect_C(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
