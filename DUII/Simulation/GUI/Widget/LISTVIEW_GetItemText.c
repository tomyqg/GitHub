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
File        : LISTVIEW_GetItemText.c
Purpose     : Implementation of LISTVIEW_GetItemText
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
*       LISTVIEW_GetItemText
*/
void LISTVIEW_GetItemText(LISTVIEW_Handle hObj, unsigned Column, unsigned Row, char* pBuffer, unsigned MaxSize) {
  memset(pBuffer, 0, MaxSize);
  if (hObj && MaxSize) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if ((Column < LISTVIEW__GetNumColumns(pObj)) && (Row < LISTVIEW__GetNumRows(pObj))) {
      LISTVIEW_ROW  * pRow;
      LISTVIEW_CELL * pCell;
      pRow  = (LISTVIEW_ROW  *)GUI_ARRAY_GetpItemLocked(pObj->RowArray,  Row);
      pCell = (LISTVIEW_CELL *)GUI_ARRAY_GetpItemLocked(pRow->CellArray, Column);
      strncpy(pBuffer, pCell->acText, MaxSize);
      pBuffer[MaxSize - 1] = 0;
      GUI_UNLOCK_H(pCell);
      GUI_UNLOCK_H(pRow);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_GetItemText_C(void);
  void LISTVIEW_GetItemText_C(void) {}
#endif

/*************************** End of file ****************************/
