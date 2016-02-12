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
File        : EDIT_SetFocussable.c
Purpose     : Implementation of EDIT_SetFocussable
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       EDIT_SetFocussable
*/
void EDIT_SetFocussable(EDIT_Handle hObj, int State) {
  if (State) {
    WIDGET_OrState(hObj, WIDGET_STATE_FOCUSSABLE);
  } else {
    WIDGET_AndState(hObj, WIDGET_STATE_FOCUSSABLE);
  }
}

#else  /* avoid empty object files */

void EDIT_SetFocussable_C(void);
void EDIT_SetFocussable_C(void){}

#endif /* GUI_WINSUPPORT */
