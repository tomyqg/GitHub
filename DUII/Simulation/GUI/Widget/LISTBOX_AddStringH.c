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
File        : LISTBOX_AddStringH.c
Purpose     : Implementation of listbox widget
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
*       LISTBOX_AddStringH
*/
void LISTBOX_AddStringH(LISTBOX_Handle hObj, WM_HMEM hString) {
  LISTBOX_Obj * pObj;
  LISTBOX_ITEM Item = {0};
  LISTBOX_ITEM * pItem;
  const char * s;
  unsigned ItemIndex;
  int Len;
  GUI_ARRAY ItemArray;

  if (hObj && hString) {
    WM_LOCK();
    s = (const char *)GUI_LOCK_H(hString);
    Len = strlen(s);
    GUI_UNLOCK_H(s);
    pObj = LISTBOX_LOCK_H(hObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    if (GUI_ARRAY_AddItem(ItemArray, &Item, sizeof(LISTBOX_ITEM) + Len) == 0) {
      ItemIndex = GUI_ARRAY_GetNumItems(ItemArray) - 1;
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
      s = (const char *)GUI_LOCK_H(hString);
      pItem->ItemPosY = -1;
      strcpy(pItem->acText, s);
      GUI_UNLOCK_H(pItem);
      GUI_UNLOCK_H(s);
      pObj = LISTBOX_LOCK_H(hObj);
      LISTBOX__InvalidateItemSize(pObj, ItemIndex);
      LISTBOX__AddSize(pObj, ItemIndex);
      GUI_UNLOCK_H(pObj);
      LISTBOX_UpdateScrollers(hObj);
      LISTBOX__InvalidateItem(hObj, ItemIndex);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_AddStringH_C(void);
  void LISTBOX_AddStringH_C(void) {}
#endif

/*************************** End of file ****************************/
