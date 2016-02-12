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
File        : LISTBOX_MultiSel.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI_ARRAY.h"
#include "LISTBOX_Private.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_SetMulti
*/
void LISTBOX_SetMulti(LISTBOX_Handle hObj, int Mode) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    if (Mode) {
      if (!(pObj->Flags & LISTBOX_SF_MULTISEL)) {
        pObj->Flags |= LISTBOX_SF_MULTISEL;
        LISTBOX__InvalidateInsideArea(hObj);
      }
    } else {
      if (pObj->Flags & LISTBOX_SF_MULTISEL) {
        pObj->Flags &= ~LISTBOX_SF_MULTISEL;
        LISTBOX__InvalidateInsideArea(hObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_GetMulti
*/
int LISTBOX_GetMulti(LISTBOX_Handle hObj) {
  int Multi = 0;
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    if (!(pObj->Flags & LISTBOX_SF_MULTISEL)) {
      Multi = 0;
    } else {
      Multi = 1;
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Multi;
}

/*********************************************************************
*
*       LISTBOX_GetItemSel
*/
int LISTBOX_GetItemSel(LISTBOX_Handle hObj, unsigned Index) {
  int Ret = 0;
  if (hObj) {
    unsigned NumItems;
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    NumItems = LISTBOX__GetNumItems(pObj);
    if ((Index < NumItems) && (pObj->Flags & LISTBOX_SF_MULTISEL)) {
      WM_HMEM hItem = GUI_ARRAY_GethItem(pObj->ItemArray, Index);
      if (hItem) {
        LISTBOX_ITEM * pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
        if (pItem->Status & LISTBOX_ITEM_SELECTED) {
          Ret = 1;
        }
        GUI_UNLOCK_H(pItem);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Ret;
}

/*********************************************************************
*
*       LISTBOX_SetItemSel
*/
void LISTBOX_SetItemSel(LISTBOX_Handle hObj, unsigned Index, int OnOff) {
  if (hObj) {
    unsigned NumItems;
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    NumItems = LISTBOX__GetNumItems(pObj);
    if ((Index < NumItems) && (pObj->Flags & LISTBOX_SF_MULTISEL)) {
      WM_HMEM hItem = GUI_ARRAY_GethItem(pObj->ItemArray, Index);
      if (hItem) {
        LISTBOX_ITEM * pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
        if (OnOff) {
          if (!(pItem->Status & LISTBOX_ITEM_SELECTED)) {
            pItem->Status |= LISTBOX_ITEM_SELECTED;
            LISTBOX__InvalidateItem(hObj, Index);
          }
        } else {
          if (pItem->Status & LISTBOX_ITEM_SELECTED) {
            pItem->Status &= ~LISTBOX_ITEM_SELECTED;
            LISTBOX__InvalidateItem(hObj, Index);
          }
        }
        GUI_UNLOCK_H(pItem);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_MultiSel_C(void) {}
#endif
