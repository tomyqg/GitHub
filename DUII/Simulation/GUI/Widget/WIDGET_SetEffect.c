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
File        : WIDGET_SetEffect.c
Purpose     : Effect routines
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI.h"
#include "WIDGET.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       WIDGET_SetEffect
*/
void WIDGET_SetEffect(WM_HWIN hObj, const WIDGET_EFFECT* pEffect) {
  WM_MESSAGE Msg;
  Msg.hWinSrc = 0;
  Msg.MsgId = WM_WIDGET_SET_EFFECT;
  Msg.Data.p = (const void*)pEffect;
  WM_SendMessage(hObj, &Msg);
}

#else
  void WIDGET_SetEffect_c(void) {} /* Avoid problems with empty object modules */
#endif /* GUI_WINSUPPORT */




