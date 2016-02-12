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
File        : CHECKBOX_SetText.c
Purpose     : Implementation of CHECKBOX_SetText
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_SetText
*/
void CHECKBOX_SetText(CHECKBOX_Handle hObj, const char * s) {
  CHECKBOX_Obj * pObj;
  WM_HMEM hpText;
  WM_HMEM hpTextOld;
  if (hObj && s) {
    WM_LOCK();
    pObj = CHECKBOX_LOCK_H(hObj);
    hpText = hpTextOld = pObj->hpText;
    GUI_UNLOCK_H(pObj);
    if (GUI__SetText(&hpText, s)) {
      if (hpText != hpTextOld) {
        pObj = CHECKBOX_LOCK_H(hObj);
        pObj->hpText = hpText;
        GUI_UNLOCK_H(pObj);
      }
      WM_Invalidate(hObj);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void CHECKBOX_SetText_C(void);
  void CHECKBOX_SetText_C(void) {}
#endif
