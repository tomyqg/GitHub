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
File        : MENU__FindItem.c
Purpose     : Implementation of menu widget
---------------------------END-OF-HEADER------------------------------
*/

#include "MENU.h"
#include "MENU_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU__FindItem
*/
int MENU__FindItem(MENU_Handle hObj, U16 ItemId, MENU_Handle * phMenu) {
  int ItemIndex = -1;
  MENU_Obj * pObj;
  if (hObj) {
    GUI_ARRAY ItemArray;
    unsigned NumItems, i;
    pObj = MENU_LOCK_H(hObj);
    NumItems = MENU__GetNumItems(pObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    for (i = 0; (i < NumItems) && (ItemIndex < 0); i++) {
      MENU_ITEM * pItem;
      MENU_Handle hSubmenu;
      U16         Id;
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i);
      hSubmenu = pItem->hSubmenu;
      Id       = pItem->Id;
      GUI_UNLOCK_H(pItem);
      if (Id == ItemId) {
        *phMenu   = hObj;
        ItemIndex = i;
      } else if (hSubmenu) {
        ItemIndex = MENU__FindItem(hSubmenu, ItemId, phMenu);
      }
    }
  }
  return ItemIndex;
}

#else  /* avoid empty object files */
  void MENU__FindItem_C(void) {}
#endif

/*************************** End of file ****************************/
