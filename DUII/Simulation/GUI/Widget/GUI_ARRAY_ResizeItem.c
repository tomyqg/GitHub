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
File        : GUI_ARRAY_ResizeItem.c
Purpose     : Array handling routines
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_ARRAY_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_ARRAY_ResizeItemLocked
*
* Purpose:
*   Resizes one element in a GUI_ARRAY_OBJ.
* Return value:
*   Handle of allocated memory block if successful
*   0 if failed
*
*/
void * GUI_ARRAY_ResizeItemLocked(GUI_ARRAY hArray, unsigned int Index, int Len) {
  GUI_ARRAY_OBJ * pThis;
  void * r;
  WM_HMEM hNew;
  
  r     = NULL;
  hNew  = GUI_ALLOC_AllocZero(Len);
  if (hNew) {
    void * pOld;
    void * pNew;
    pThis = (GUI_ARRAY_OBJ *)GUI_LOCK_H(hArray);
    pOld = GUI_ARRAY__GetpItemLocked(pThis, Index);
    pNew = GUI_LOCK_H(hNew);
    GUI_MEMCPY(pNew, pOld, Len);
    if (GUI_ARRAY__SethItem(pThis, Index, hNew)) {
      GUI_ALLOC_FreePtr(&hNew);    /* Free on error */
    } else {
      r = pNew;
    }
    GUI_UNLOCK_H(pThis);
    GUI_UNLOCK_H(pOld);
  }
  return r;
}

#else  /* avoid empty object files */

void GUI_ARRAY_ResizeItem_C(void);
void GUI_ARRAY_ResizeItem_C(void){}

#endif
