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
File        : LISTVIEW_SetItemBitmap.c
Purpose     : Implementation of LISTVIEW_SetItemBitmap
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
static LISTVIEW_CELL_INFO * _GetpCellInfo(LISTVIEW_Handle hObj, unsigned Column, unsigned Row) {
  LISTVIEW_CELL_INFO * pCellInfo = 0;
  if (hObj) {
    GUI_ARRAY RowArray;
    GUI_ARRAY CellArray;
    unsigned NumColumns, NumRows;
    LISTVIEW_Obj * pObj;
    LISTVIEW_ROW  * pRow;
    LISTVIEW_CELL * pCell;
    WM_HMEM hCellInfo;

    pObj = LISTVIEW_LOCK_H(hObj);
    NumColumns = LISTVIEW__GetNumColumns(pObj);
    NumRows    = LISTVIEW__GetNumRows(pObj);
    RowArray   = pObj->RowArray;
    GUI_UNLOCK_H(pObj);
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
              GUI_MEMSET((U8 *)pCellInfo, 0, sizeof(LISTVIEW_CELL_INFO));
            }
          } else {
            pCellInfo = (LISTVIEW_CELL_INFO *)GUI_LOCK_H(hCellInfo);
          }
        }
      }
    }
  }
  return pCellInfo;
}

/*********************************************************************
*
*       _Draw
*/
static void _Draw(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, GUI_RECT * pRect) {
  LISTVIEW_Obj * pObj;
  LISTVIEW_CELL_INFO * pCellInfo;
  const GUI_BITMAP GUI_UNI_PTR * pBitmap;

  if (hObj) {
    pCellInfo = _GetpCellInfo(hObj, Column, Row);
    if (pCellInfo) {
      if (pCellInfo->pData) {
        pBitmap = (const GUI_BITMAP GUI_UNI_PTR *)pCellInfo->pData;
        WM_SetUserClipRect(pRect);
        GUI_DrawBitmap(pBitmap, pRect->x0 + pCellInfo->xOff, pRect->y0 + pCellInfo->yOff);
        WM_SetUserClipRect(NULL);
      }
      GUI_UNLOCK_H(pCellInfo);
      pObj = LISTVIEW_LOCK_H(hObj);
      LISTVIEW__InvalidateRow(hObj, pObj, Row);
      GUI_UNLOCK_H(pObj);
    }
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
*       LISTVIEW_SetItemBitmap
*/
void LISTVIEW_SetItemBitmap(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, int xOff, int yOff, const GUI_BITMAP GUI_UNI_PTR * pBitmap) {
  LISTVIEW_Obj * pObj;
  LISTVIEW_CELL_INFO * pCellInfo;

  if (hObj) {
    WM_LOCK();
    pCellInfo = _GetpCellInfo(hObj, Column, Row);
    if (pCellInfo) {
      pObj = LISTVIEW_LOCK_H(hObj);
      pCellInfo->pData  = (void *)pBitmap;
      pCellInfo->pfDraw = _Draw;
      pCellInfo->Flags |= LISTVIEW_CELL_INFO_BITMAP;
      pCellInfo->xOff = (I16)xOff;
      pCellInfo->yOff = (I16)yOff;
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
  void LISTVIEW_SetItemBitmap_C(void);
  void LISTVIEW_SetItemBitmap_C(void) {}
#endif

/*************************** End of file ****************************/
