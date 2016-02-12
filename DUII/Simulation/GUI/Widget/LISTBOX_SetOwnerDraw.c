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
File        : LISTBOX_SetOwnerDraw.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_SetOwnerDraw
*/
void LISTBOX_SetOwnerDraw(LISTBOX_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawItem) {
  LISTBOX_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    if (pfDrawItem) {
      pObj->pfDrawItem = pfDrawItem;
    }else {
      pObj->pfDrawItem = LISTBOX_OwnerDraw;
    }
    pObj->ContentSizeX = 0;
    GUI_UNLOCK_H(pObj);
    LISTBOX_InvalidateItem(hObj, LISTBOX_ALL_ITEMS);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetOwnerDraw_C(void) {}
#endif

/*************************** End of file ****************************/

