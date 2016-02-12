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
File        : WM_GetScrollState.c
Purpose     : Implementation of WM_GetScrollState
----------------------------------------------------------------------
*/

#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       WM_GetScrollState
*/
void WM_GetScrollState(WM_HWIN hObj, WM_SCROLL_STATE* pScrollState) {
  WM_MESSAGE Msg;
  Msg.MsgId  = WM_GET_SCROLL_STATE;
  Msg.Data.p = pScrollState;
  WM_SendMessage(hObj, &Msg);
}

#else
  void WM_GetScrollState_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
