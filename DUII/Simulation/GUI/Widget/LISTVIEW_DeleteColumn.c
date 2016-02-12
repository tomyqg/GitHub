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
File        : LISTVIEW_DeleteColumn.c
Purpose     : Implementation of LISTVIEW_DeleteColumn
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_DeleteColumn
*/
void LISTVIEW_DeleteColumn(LISTVIEW_Handle hObj, unsigned Index) {
  if (hObj) {
    LISTVIEW_Obj * pObj;
    LISTVIEW_ROW * pRow;
    unsigned NumRows, i;
    LISTVIEW_CELL * pCell;
    GUI_ARRAY CellArray;
    WM_HMEM hCellInfo;

    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (Index < LISTVIEW__GetNumColumns(pObj)) {
      HEADER_DeleteItem(pObj->hHeader, Index);
      GUI_ARRAY_DeleteItem(pObj->ColumnArray, Index);
      NumRows = LISTVIEW__GetNumRows(pObj);
      for (i = 0; i < NumRows; i++) {
        pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(pObj->RowArray, i);
        CellArray = pRow->CellArray;
        GUI_UNLOCK_H(pRow);
        /* Delete attached info items */
        pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(CellArray, Index);
        hCellInfo = pCell->hCellInfo;
        GUI_UNLOCK_H(pCell);
        if (hCellInfo) {
          GUI_ALLOC_Free(hCellInfo);
        }
        /* Delete cell */
        GUI_ARRAY_DeleteItem(CellArray, Index);
      }
      GUI_UNLOCK_H(pObj);
      LISTVIEW__UpdateScrollParas(hObj);
      pObj = LISTVIEW_LOCK_H(hObj);
      LISTVIEW__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_DeleteColumn_C(void);
  void LISTVIEW_DeleteColumn_C(void) {}
#endif

/*************************** End of file ****************************/
