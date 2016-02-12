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
File        : MENU_GetItemText.c
Purpose     : Implementation of menu widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

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
*       MENU_GetItemText
*/
void MENU_GetItemText(MENU_Handle hObj, U16 ItemId, char * pBuffer, unsigned BufferSize) {
  if (hObj && pBuffer) {
    int Index;
    WM_LOCK();
    Index = MENU__FindItem(hObj, ItemId, &hObj);
    if (Index >= 0) {
      MENU_Obj *  pObj;
      MENU_ITEM * pItem;
      pObj  = MENU_LOCK_H(hObj);
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
      strncpy(pBuffer, pItem->acText, BufferSize);
      GUI_UNLOCK_H(pItem);
      GUI_UNLOCK_H(pObj);
      pBuffer[BufferSize - 1] = 0;
    }
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */
  void MENU_GetItemText_C(void) {}
#endif

/*************************** End of file ****************************/
