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
File        : BUTTON_SelfDraw.c
Purpose     : Button self draw support
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON.h"
#include "BUTTON_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_SetSelfDrawEx
*/
void BUTTON_SetSelfDrawEx(BUTTON_Handle hObj,unsigned int Index, GUI_DRAW_SELF_CB* pDraw, int x, int y) {
  BUTTON__SetDrawObj(hObj, Index, GUI_DRAW_SELF_Create(pDraw, x, y));
}

/*********************************************************************
*
*       BUTTON_SetSelfDraw
*/
void BUTTON_SetSelfDraw(BUTTON_Handle hObj,unsigned int Index, GUI_DRAW_SELF_CB* pDraw) {
  BUTTON_SetSelfDrawEx(hObj, Index, pDraw, 0, 0);
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_SelfDraw_C(void) {}
#endif
