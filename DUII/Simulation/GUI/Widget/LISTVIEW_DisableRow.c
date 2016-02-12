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
File        : LISTVIEW_DisableRow.c
Purpose     : Implementation of LISTVIEW_DisableRow
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "LISTVIEW_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetRowDisabled
*/
static void _SetRowDisabled(LISTVIEW_Handle hObj, unsigned Row, int OnOff) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (Row < LISTVIEW__GetNumRows(pObj)) {
      LISTVIEW_ROW * pRow;
      pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(pObj->RowArray, Row);
      pRow->Disabled = OnOff;
      GUI_UNLOCK_H(pRow);
      LISTVIEW__InvalidateRow(hObj, pObj, Row);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
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
*       LISTVIEW_DisableRow
*/
void LISTVIEW_DisableRow(LISTVIEW_Handle hObj, unsigned Row) {
  _SetRowDisabled(hObj, Row, 1);
}

/*********************************************************************
*
*       LISTVIEW_EnableRow
*/
void LISTVIEW_EnableRow(LISTVIEW_Handle hObj, unsigned Row) {
  _SetRowDisabled(hObj, Row, 0);
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_DisableRow_C(void);
  void LISTVIEW_DisableRow_C(void) {}
#endif

/*************************** End of file ****************************/
