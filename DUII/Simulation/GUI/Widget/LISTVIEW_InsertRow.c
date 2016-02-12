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
File        : LISTVIEW_InsertRow.c
Purpose     : Implementation of LISTVIEW_InsertRow
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

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
*       LISTVIEW_InsertRow
*/
int LISTVIEW_InsertRow(LISTVIEW_Handle hObj, unsigned Index, const GUI_ConstString * ppText) {
  int r = 0;
  if (hObj) {
    GUI_ARRAY RowArray;
    GUI_ARRAY CellArray;
    unsigned NumItems;
    int NumColumns;
    int ScrollChanged;
    LISTVIEW_Obj * pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    NumItems   = GUI_ARRAY_GetNumItems(pObj->RowArray);
    NumColumns = LISTVIEW__GetNumColumns(pObj);
    RowArray   = pObj->RowArray;
    GUI_UNLOCK_H(pObj);
    if (Index < NumItems) {
      /* Insert new row */
      if (GUI_ARRAY_InsertItem(RowArray, Index, sizeof(LISTVIEW_ROW))) {
        LISTVIEW_ROW * pRow;
        int i, NumBytes;
        const char * s;
        CellArray = GUI_ARRAY_Create();
        if (CellArray) {
          pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(RowArray, Index);
          pRow->CellArray = CellArray;
          GUI_UNLOCK_H(pRow);
          /* Add columns for the new row */
          for (i = 0; i < NumColumns; i++) {
            LISTVIEW_CELL * pCell;
            s = (ppText) ? *ppText++ : 0;
            if (s == 0) {
              ppText = 0;
            }
            NumBytes = GUI__strlen(s) + 1;
            if (GUI_ARRAY_AddItem(CellArray, NULL, sizeof(LISTVIEW_CELL) + NumBytes)) {
              r = 1;
              break;
            }
            pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(CellArray, i);
            if (NumBytes > 1) {
              strcpy(pCell->acText, s);
            }
            GUI_UNLOCK_H(pCell);
          }
          pObj = LISTVIEW_LOCK_H(hObj);
          if (pObj->SortIndex < 0) {
            if ((int)Index < LISTVIEW__GetSel(pObj)) {
              LISTVIEW_IncSel(hObj);
            }
          }
          pObj->IsPresorted = 0;
          GUI_UNLOCK_H(pObj);
          ScrollChanged = LISTVIEW__UpdateScrollParas(hObj);
          pObj = LISTVIEW_LOCK_H(hObj);
          if (ScrollChanged) {
            LISTVIEW__InvalidateInsideArea(hObj);
          } else {
            if (pObj->hSort && (pObj->SortIndex >= 0)) {
              LISTVIEW__InvalidateInsideArea(hObj);
            } else {
              LISTVIEW__InvalidateRowAndBelow(hObj, pObj, Index);
            }
          }
          GUI_UNLOCK_H(pObj);
        }
      } else {
        r = 1;
      }
    } else {
      /* Append new row */
      r = LISTVIEW_AddRow(hObj, ppText);
    }
    WM_UNLOCK();
  }
  return r;
}


#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_InsertRow_C(void);
  void LISTVIEW_InsertRow_C(void) {}
#endif

/*************************** End of file ****************************/
