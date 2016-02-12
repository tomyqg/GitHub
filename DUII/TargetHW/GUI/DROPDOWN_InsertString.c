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
File        : DROPDOWN_InsertString.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "DROPDOWN.h"
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
*       DROPDOWN_InsertString
*/
void DROPDOWN_InsertString(DROPDOWN_Handle hObj, const char * s, unsigned int Index) {
  if (hObj && s) {
    DROPDOWN_Obj * pObj;
    unsigned int NumItems;
    GUI_ARRAY Handles;
    WM_HWIN hListWin;
    WM_LOCK();
    NumItems = DROPDOWN_GetNumItems(hObj);
    pObj = DROPDOWN_LOCK_H(hObj);
    Handles  = pObj->Handles;
    hListWin = pObj->hListWin;
    GUI_UNLOCK_H(pObj);
    if (Index < NumItems) {
      WM_HMEM hItem;
      hItem = GUI_ARRAY_InsertItem(Handles, Index, strlen(s) + 1);
      if (hItem) {
        char * pBuffer = (char *)GUI_LOCK_H(hItem);
        strcpy(pBuffer, s);
        GUI_UNLOCK_H(s);
      }
      WM_InvalidateWindow(hObj);
      if (hListWin) {
        LISTBOX_InsertString(hListWin, s, Index);
      }
    } else {
      DROPDOWN_AddString(hObj, s);
      if (hListWin) {
        LISTBOX_AddString(hListWin, s);
      }
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_InsertString_C(void);
  void DROPDOWN_InsertString_C(void) {}
#endif
