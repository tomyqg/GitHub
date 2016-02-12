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
File        : GUI_ARRAY_InsertItem.c
Purpose     : Array handling routines
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_ARRAY_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ARRAY_InsertBlankItem
*
* Purpose:
*   Inserts a blank element in a GUI_ARRAY_OBJ.
*
* Parameters:
*   Index   Index of the element to insert before
*           0 means: Insert before first element
*           1 means: Insert before second element
*
* Return value:
*   1 if successful
*   0 if failed
*
* Notes:
*   (1) Index changes
*       The index of all items after the one inserted will change
*       (Increment by 1)
*/
char GUI_ARRAY_InsertBlankItem(GUI_ARRAY hArray, unsigned int Index) {
  GUI_ARRAY_OBJ * pThis;
  U16 NumItems;
  WM_HMEM haHandle;
  
  if (hArray) {
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    NumItems = pThis->NumItems;
    haHandle = pThis->haHandle;
    GUI_UNLOCK_H(pThis);
    if (Index >= (unsigned)NumItems) {
      GUI_DEBUG_ERROROUT("GUI_ARRAY_InsertBlankItem: Illegal index");
    } else {
      WM_HMEM hNewBuffer;
      hNewBuffer = GUI_ALLOC_AllocZero(sizeof(WM_HMEM) * (NumItems + 1));
      if (hNewBuffer == 0) {
        GUI_DEBUG_ERROROUT("GUI_ARRAY_InsertBlankItem: Failed to alloc buffer");
      } else {
        WM_HMEM * pOldBuffer;
        WM_HMEM * pNewBuffer;
        pNewBuffer = (WM_HMEM *)GUI_LOCK_H(hNewBuffer);
        pOldBuffer = (WM_HMEM *)GUI_LOCK_H(haHandle);
        GUI_MEMCPY(pNewBuffer, pOldBuffer, Index * sizeof(WM_HMEM));
        GUI_MEMCPY(pNewBuffer + (Index + 1), pOldBuffer + Index, (NumItems - Index) * sizeof(WM_HMEM));
        GUI_UNLOCK_H(pNewBuffer);
        GUI_UNLOCK_H(pOldBuffer);
        GUI_ALLOC_Free(haHandle);
        pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
        pThis->haHandle = hNewBuffer;
        pThis->NumItems++;
        GUI_UNLOCK_H(pThis);
        return 1;               /* Successfull */
      }
    }
  }
  return 0;                   /* Failed */
}

/*********************************************************************
*
*       GUI_ARRAY_InsertItem
*
* Purpose:
*   Inserts an element in a GUI_ARRAY_OBJ.
*
*
* Parameters:
*   Index   Index of the element to insert before
*           0 means: Insert before first element
*           1 means: Insert before second element
*
* Return value:
*   Handle of allocated memory block if successful
*   0 if failed
*
* Notes:
*   (1) Index changes
*       The index of all items after the one inserted will change
*       (Increment by 1)
*/
WM_HMEM GUI_ARRAY_InsertItem(GUI_ARRAY hArray, unsigned int Index, int Len) {
  WM_HMEM hNewBuffer = 0;
  if (GUI_ARRAY_InsertBlankItem(hArray, Index)) {
    hNewBuffer = GUI_ARRAY_SetItem(hArray, Index, 0, Len);
  }
  return hNewBuffer;
}

#else  /* avoid empty object files */

void GUI_ARRAY_InsertItem_C(void);
void GUI_ARRAY_InsertItem_C(void){}

#endif
