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
File        : LISTVIEW_DeleteRow.c
Purpose     : Implementation of LISTVIEW_DeleteRow
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW__InvalidateRowAndBelow
*/
void LISTVIEW__InvalidateRowAndBelow(LISTVIEW_Handle hObj, LISTVIEW_Obj* pObj, int Sel) {
  if (Sel >= 0) {
    GUI_RECT Rect;
    int HeaderHeight, RowDistY;
    HeaderHeight = HEADER_GetHeight(pObj->hHeader);
    RowDistY     = LISTVIEW__GetRowDistY(pObj);
    WM_GetInsideRectExScrollbar(hObj, &Rect);
    Rect.y0 += HeaderHeight + (Sel - pObj->ScrollStateV.v) * RowDistY;
    WM_InvalidateRect(hObj, &Rect);
  }
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_DeleteRow
*/
void LISTVIEW_DeleteRow(LISTVIEW_Handle hObj, unsigned Index) {
  LISTVIEW_Obj * pObj;
  unsigned NumRows;
  int ScrollChanged;
  unsigned NumColumns, i;
  LISTVIEW_ROW * pRow;
  GUI_ARRAY CellArray;
  if (hObj) {
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    NumRows = LISTVIEW__GetNumRows(pObj);
    if (Index < NumRows) {
      pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(pObj->RowArray, Index);
      CellArray = pRow->CellArray;
      GUI_UNLOCK_H(pRow);
      /* Delete attached info items */
      NumColumns = LISTVIEW__GetNumColumns(pObj);
      for (i = 0; i < NumColumns; i++) {
        LISTVIEW_CELL* pCell;
        pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(CellArray, i);
        if (pCell) {
          if (pCell->hCellInfo) {
            GUI_ALLOC_Free(pCell->hCellInfo);
          }
          GUI_UNLOCK_H(pCell);
        }
      }
      /* Delete row */
      GUI_ARRAY_Delete(CellArray);
      GUI_ARRAY_DeleteItem(pObj->RowArray, Index);
      /* Adjust properties */
      if (pObj->Sel == (int)Index) {
        pObj->Sel = -1;
      } else if (pObj->SortIndex < 0) {
        if (pObj->Sel > (int)Index) {
          pObj->Sel--;
        }
      } else {
        while (pObj->Sel >= (int)LISTVIEW__GetNumRows(pObj)) {
          pObj->Sel = -1;
        }
      }
      pObj->IsPresorted = 0;
      GUI_UNLOCK_H(pObj);
      ScrollChanged = LISTVIEW__UpdateScrollParas(hObj);
      pObj = LISTVIEW_LOCK_H(hObj);
      if (ScrollChanged) {
        LISTVIEW__InvalidateInsideArea(hObj);
      } else {
        LISTVIEW__InvalidateRowAndBelow(hObj, pObj, Index);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_DeleteRow_C(void);
  void LISTVIEW_DeleteRow_C(void) {}
#endif

/*************************** End of file ****************************/
