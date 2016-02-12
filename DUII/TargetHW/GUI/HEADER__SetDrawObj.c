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
File        : HEADER__SetBitmapObj.c
Purpose     : HEADER bitmap support, private function
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER.h"
#include "HEADER_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER__SetDrawObj
*/
void HEADER__SetDrawObj(HEADER_Handle hObj, unsigned Index, GUI_DRAW_HANDLE hDrawObj) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (Index <= GUI_ARRAY_GetNumItems(pObj->Columns)) {
      HEADER_COLUMN * pColumn;
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index);
      if (pColumn) {
        GUI_ALLOC_FreePtr(&pColumn->hDrawObj);
        pColumn->hDrawObj = hDrawObj;
      }
      GUI_UNLOCK_H(pColumn);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void HEADER__SetBitmapObj_C(void) {}
#endif
