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
File        : WM_GetId.c
Purpose     : Implementation of WM_GetId
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
*       WM_GetId
*/
int WM_GetId(WM_HWIN hObj) {
  WM_MESSAGE Msg = {0};
  Msg.MsgId  = WM_GET_ID;
  WM_SendMessage(hObj, &Msg);
  return Msg.Data.v;
}

#else
  void WM_GetId_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
