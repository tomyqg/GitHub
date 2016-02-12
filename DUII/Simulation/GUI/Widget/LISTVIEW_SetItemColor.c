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
File        : LISTVIEW_SetItemColor.c
Purpose     : Implementation of LISTVIEW_SetItemTextColor and LISTVIEW_SetItemBkColor
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "LISTVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetpCellInfo
*
* Purpose:
*   Returns a pointer of a LISTVIEW_CELL_INFO structure in locked state.
*   The pointer needs to be unlocked after use.
*/
static LISTVIEW_CELL_INFO * _GetpCellInfo(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index) {
  LISTVIEW_CELL_INFO * pCellInfo = 0;
  if (hObj) {
    GUI_ARRAY RowArray;
    GUI_ARRAY CellArray;
    unsigned NumColumns, NumRows;
    LISTVIEW_Obj * pObj;
    LISTVIEW_ROW  * pRow;
    LISTVIEW_CELL * pCell;
    WM_HMEM hCellInfo;
    int i;

    pObj = LISTVIEW_LOCK_H(hObj);
    NumColumns = LISTVIEW__GetNumColumns(pObj);
    NumRows    = LISTVIEW__GetNumRows(pObj);
    RowArray   = pObj->RowArray;
    GUI_UNLOCK_H(pObj);
    if (Index < GUI_COUNTOF(pCellInfo->aTextColor)) {
      if ((Column < NumColumns) && (Row < NumRows)) {
        pRow  = (LISTVIEW_ROW  *)GUI_ARRAY_GetpItemLocked(RowArray,  Row);
        if (pRow) {
          CellArray = pRow->CellArray;
          GUI_UNLOCK_H(pRow);
          pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(CellArray, Column);
          if (pCell) {
            hCellInfo = pCell->hCellInfo;
            GUI_UNLOCK_H(pCell);
            if (hCellInfo == 0) {
              hCellInfo = GUI_ALLOC_AllocZero(sizeof(LISTVIEW_CELL_INFO));
              if (hCellInfo) {
                pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(CellArray, Column);
                pCell->hCellInfo = hCellInfo;
                GUI_UNLOCK_H(pCell);
                pCellInfo = (LISTVIEW_CELL_INFO *)GUI_LOCK_H(hCellInfo);
                for (i = 0; i < GUI_COUNTOF(pCellInfo->aTextColor); i++) {
                  pCellInfo->aTextColor[i] = LISTVIEW_GetTextColor(hObj, i);
                  pCellInfo->aBkColor[i]   = LISTVIEW_GetBkColor  (hObj, i);
                }
              }
            } else {
              pCellInfo = (LISTVIEW_CELL_INFO *)GUI_LOCK_H(hCellInfo);
            }
          }
        }
      }
    }
  }
  return pCellInfo;
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_SetItemTextColor
*/
void LISTVIEW_SetItemTextColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, GUI_COLOR Color) {
  LISTVIEW_Obj * pObj;
  LISTVIEW_CELL_INFO * pCellInfo;
  if (hObj) {
    WM_LOCK();
    pCellInfo = _GetpCellInfo(hObj, Column, Row, Index);
    if (pCellInfo) {
      pObj = LISTVIEW_LOCK_H(hObj);
      pCellInfo->aTextColor[Index] = Color;
      pCellInfo->Flags |= LISTVIEW_CELL_INFO_COLORS;
      LISTVIEW__InvalidateRow(hObj, pObj, Row);
      GUI_UNLOCK_H(pObj);
    }
    if (pCellInfo) {
      GUI_UNLOCK_H(pCellInfo);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_SetItemBkColor
*/
void LISTVIEW_SetItemBkColor(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, unsigned int Index, GUI_COLOR Color) {
  LISTVIEW_Obj * pObj;
  LISTVIEW_CELL_INFO * pCellInfo;
  if (hObj) {
    WM_LOCK();
    pCellInfo = _GetpCellInfo(hObj, Column, Row, Index);
    if (pCellInfo) {
      pObj = LISTVIEW_LOCK_H(hObj);
      pCellInfo->aBkColor[Index] = Color;
      pCellInfo->Flags |= LISTVIEW_CELL_INFO_COLORS;
      LISTVIEW__InvalidateRow(hObj, pObj, Row);
      GUI_UNLOCK_H(pObj);
    }
    if (pCellInfo) {
      GUI_UNLOCK_H(pCellInfo);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetItemColor_C(void);
  void LISTVIEW_SetItemColor_C(void) {}
#endif

/*************************** End of file ****************************/
