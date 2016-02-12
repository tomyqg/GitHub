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
File        : PROGBAR_SetTextAlign.c
Purpose     : Implementation of progbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_SetTextAlign
*/
void PROGBAR_SetTextAlign(PROGBAR_Handle hObj, int Align) {
  PROGBAR_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = PROGBAR_LOCK_H(hObj);
    pObj->TextAlign = Align;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */
  void PROGBAR_SetTextAlign_C(void);
  void PROGBAR_SetTextAlign_C(void) {}
#endif
