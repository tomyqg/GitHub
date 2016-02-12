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
File        : LISTVIEW_SetItemText.c
Purpose     : Implementation of LISTVIEW_SetItemText
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
*       LISTVIEW_SetItemText
*/
void LISTVIEW_SetItemText(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, const char * s) {
  if (hObj && s) {
    GUI_ARRAY RowArray;
    GUI_ARRAY CellArray;
    unsigned NumColumns, NumRows;
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    NumColumns = LISTVIEW__GetNumColumns(pObj);
    NumRows    = LISTVIEW__GetNumRows(pObj);
    RowArray   = pObj->RowArray;
    GUI_UNLOCK_H(pObj);
    if ((Column < NumColumns) && (Row < NumRows)) {
      LISTVIEW_ROW  * pRow;
      LISTVIEW_CELL * pCell;
      int NumBytes;
      NumBytes = GUI__strlen(s) + 1;
      pRow  = (LISTVIEW_ROW  *)GUI_ARRAY_GetpItemLocked(RowArray, Row);
      CellArray = pRow->CellArray;
      GUI_UNLOCK_H(pRow);
      pCell = (LISTVIEW_CELL *)GUI_ARRAY_ResizeItemLocked(CellArray, Column, sizeof(LISTVIEW_CELL) + NumBytes);
      if (pCell) {
        pObj = LISTVIEW_LOCK_H(hObj);
        strcpy(pCell->acText, s);
        GUI_UNLOCK_H(pCell);
        pObj->IsPresorted = 0;
        LISTVIEW__InvalidateRow(hObj, pObj, Row);
        GUI_UNLOCK_H(pObj);
      }
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetItemText_C(void);
  void LISTVIEW_SetItemText_C(void) {}
#endif

/*************************** End of file ****************************/
