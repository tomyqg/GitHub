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
File        : GUI_ARRAY_DeleteItem.c
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
*       GUI_ARRAY_DeleteItem
*
* Purpose:
*   Eliminate one element in a GUI_ARRAY_OBJ.
*   This means:
*     - freeing the memory block
*     - moving the last item to the position of the deleted item
*     - possible reducing the size of the memory used for management (opt)
*
*/
void GUI_ARRAY_DeleteItem(GUI_ARRAY hArray, unsigned int Index) {
  GUI_ARRAY_OBJ * pThis;
  WM_HMEM   ha;
  WM_HMEM * pa;
  int i;

  pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
  if (Index < (unsigned)pThis->NumItems) {
    ha = pThis->haHandle;
    if (ha) {
      int NumItems;
      WM_LOCK();
      pa = (WM_HMEM *)GUI_LOCK_H(ha);
      /* Free the attached item */
      GUI_ALLOC_FreePtr(pa + Index);
      /* Move the last items to the position of the deleted item */
      NumItems = --pThis->NumItems;
      for (i = Index; i <= NumItems - 1; i++) {
        *(pa + i) = *(pa + i + 1);
      }
      GUI_UNLOCK_H(pa);
      WM_UNLOCK();
    }
  }
  GUI_UNLOCK_H(pThis);
}

#else

void GUI_ARRAY_DeleteItem(void) {}

#endif
