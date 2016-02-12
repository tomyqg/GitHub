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
File        : LISTBOX_ItemSpacing.c
Purpose     : Implementation of LISTBOX_SetItemSpacing & LISTBOX_GetItemSpacing
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "LISTBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       LISTBOX_SetItemSpacing
*/
void LISTBOX_SetItemSpacing(LISTBOX_Handle hObj, unsigned Value) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    pObj->ItemSpacing = Value;
    GUI_UNLOCK_H(pObj);
    LISTBOX_InvalidateItem(hObj, LISTBOX_ALL_ITEMS);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_GetItemSpacing
*/
unsigned LISTBOX_GetItemSpacing(LISTBOX_Handle hObj) {
  unsigned Value = 0;
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    Value = pObj->ItemSpacing;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Value;
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetItemSpacing_C(void);
  void LISTBOX_SetItemSpacing_C(void) {}
#endif
