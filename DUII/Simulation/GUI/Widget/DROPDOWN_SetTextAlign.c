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
File        : DROPDOWN_SetTextAlign.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       DROPDOWN_SetTextAlign
*/
void DROPDOWN_SetTextAlign(DROPDOWN_Handle hObj, int Align) {
  DROPDOWN_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    pObj->Props.Align = Align;
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_SetTextAlign_C(void);
  void DROPDOWN_SetTextAlign_C(void) {}
#endif
