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
File        : LISTBOX_SetString.c
Purpose     : Implementation of LISTBOX_SetString
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
*       LISTBOX_SetString
*/
void LISTBOX_SetString(LISTBOX_Handle hObj, const char* s, unsigned int Index) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    unsigned NumItems;
    GUI_ARRAY ItemArray;
    LISTBOX_ITEM * pItem;
    WM_LOCK();

    pObj = LISTBOX_LOCK_H(hObj);
    NumItems = LISTBOX__GetNumItems(pObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    if (Index < NumItems) {
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_ResizeItemLocked(ItemArray, Index, sizeof(LISTBOX_ITEM) + strlen(s));
      if (pItem) {
        strcpy(pItem->acText, s);
        GUI_UNLOCK_H(pItem);
        pObj = LISTBOX_LOCK_H(hObj);
        pObj->ContentSizeX = 0;
        LISTBOX__InvalidateItemSize(pObj, Index);
        GUI_UNLOCK_H(pObj);
        LISTBOX_UpdateScrollers(hObj);
        LISTBOX__InvalidateItem(hObj, Index);
      }
    }
    WM_UNLOCK();
  }
}


#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetString_C(void) {}
#endif
