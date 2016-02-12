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
File        : BUTTON_SetTextAlign.c
Purpose     : Implementation of BUTTON_SetTextAlign
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_SetTextAlign
*/
void BUTTON_SetTextAlign(BUTTON_Handle hObj, int Align) {
  if (hObj) {
    BUTTON_Obj* pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    pObj->Props.Align = Align;
    GUI_UNLOCK_H(pObj);
    BUTTON_Invalidate(hObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_SetTextAlign_C(void);
  void BUTTON_SetTextAlign_C(void) {}
#endif
