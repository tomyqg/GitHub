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
File        : LISTBOX_InsertString.c
Purpose     : Implementation of LISTBOX_InsertString
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
*       LISTBOX_InsertString
*/
void LISTBOX_InsertString(LISTBOX_Handle hObj, const char* s, unsigned int Index) {
  if (hObj && s) {
    LISTBOX_Obj * pObj;
    unsigned int NumItems;
    GUI_ARRAY ItemArray;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    NumItems = LISTBOX__GetNumItems(pObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    if (Index < NumItems) {
      WM_HMEM hItem;
      hItem = GUI_ARRAY_InsertItem(ItemArray, Index, sizeof(LISTBOX_ITEM) + strlen(s));
      if (hItem) {
        LISTBOX_ITEM * pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
        pItem->Status = 0;
        pItem->ItemPosY = -1;
        strcpy(pItem->acText, s);
        GUI_UNLOCK_H(pItem);
        pObj = LISTBOX_LOCK_H(hObj);
        LISTBOX__AddSize(pObj, Index);
        GUI_UNLOCK_H(pObj);
        LISTBOX_InvalidateItem(hObj, Index);
      }
    } else {
      LISTBOX_AddString(hObj, s);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_InsertString_C(void) {}
#endif
