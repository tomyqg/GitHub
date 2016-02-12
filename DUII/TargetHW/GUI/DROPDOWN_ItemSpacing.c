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
File        : DROPDOWN_ItemSpacing.c
Purpose     : Implementation of DROPDOWN_SetItemSpacing & DROPDOWN_GetItemSpacing
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
*       DROPDOWN_SetItemSpacing
*/
void DROPDOWN_SetItemSpacing(DROPDOWN_Handle hObj, unsigned Value) {
  if (hObj) {
    DROPDOWN_Obj * pObj;
    WM_HWIN hListWin;
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    hListWin = pObj->hListWin;
    pObj->ItemSpacing = Value;
    GUI_UNLOCK_H(pObj);
    if (hListWin) {
      LISTBOX_SetItemSpacing(hListWin, Value);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       DROPDOWN_GetItemSpacing
*/
unsigned DROPDOWN_GetItemSpacing(DROPDOWN_Handle hObj) {
  unsigned Value = 0;
  if (hObj) {
    DROPDOWN_Obj * pObj;
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    Value = pObj->ItemSpacing;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Value;
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_ItemSpacing_C(void);
  void DROPDOWN_ItemSpacing_C(void) {}
#endif
