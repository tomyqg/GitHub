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
File        : WM_GetBkColor.c
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
*       WM_GetBkColor
*
  Purpose:
    Return the clients background color.
    If a window does not define a background color, the default
    procedure returns GUI_INVALID_COLOR
*/
GUI_COLOR WM_GetBkColor(WM_HWIN hObj) {
  if (hObj) {
    WM_MESSAGE Msg;
    Msg.MsgId  = WM_GET_BKCOLOR;
    WM_SendMessage(hObj, &Msg);
    return Msg.Data.Color;
  }
  return GUI_INVALID_COLOR;
}

#else
  void WM_GetBkColor_C(void) {} /* avoid empty object files */
#endif   /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
