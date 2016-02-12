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
File        : DROPDOWN_SetAutoScroll.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"
#include "LISTBOX.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       DROPDOWN_SetAutoScroll
*/
void DROPDOWN_SetAutoScroll(DROPDOWN_Handle hObj, int OnOff) {
  if (hObj) {
    DROPDOWN_Obj * pObj;
    char Flags;
    WM_HWIN hListWin;

    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    hListWin = pObj->hListWin;
    Flags = pObj->Flags & (~DROPDOWN_SF_AUTOSCROLLBAR);
    if (OnOff) {
      Flags |= DROPDOWN_SF_AUTOSCROLLBAR;
    }
    if (pObj->Flags != Flags) {
      pObj->Flags = Flags;
      GUI_UNLOCK_H(pObj);
      if (hListWin) {
        LISTBOX_SetAutoScrollV(hListWin, (Flags & DROPDOWN_SF_AUTOSCROLLBAR) ? 1 : 0);
      }
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_SetAutoScroll_C(void);
  void DROPDOWN_SetAutoScroll_C(void) {}
#endif
