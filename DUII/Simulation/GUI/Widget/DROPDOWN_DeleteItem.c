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
File        : DROPDOWN_DeleteItem.c
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
*       DROPDOWN_DeleteItem
*/
void DROPDOWN_DeleteItem(DROPDOWN_Handle hObj, unsigned int Index) {
  if (hObj) {
    DROPDOWN_Obj * pObj;
    unsigned int NumItems;
    WM_HWIN hListWin;

    NumItems = DROPDOWN_GetNumItems(hObj);
    if (Index < NumItems) {
      WM_LOCK();
      pObj = DROPDOWN_LOCK_H(hObj);
      hListWin = pObj->hListWin;
      GUI_ARRAY_DeleteItem(pObj->Handles, Index);
      GUI_UNLOCK_H(pObj);
      WM_InvalidateWindow(hObj);
      if (hListWin) {
        LISTBOX_DeleteItem(hListWin, Index);
      }
      WM_UNLOCK();
    }
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_DeleteItem_C(void);
  void DROPDOWN_DeleteItem_C(void) {}
#endif
