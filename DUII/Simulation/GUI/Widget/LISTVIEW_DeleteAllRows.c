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
File        : LISTVIEW_DeleteAllRows.c
Purpose     : Implementation of LISTVIEW_DeleteAllRows
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
*       LISTVIEW_DeleteAllRows
*/
void LISTVIEW_DeleteAllRows(LISTVIEW_Handle hObj) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    unsigned NumRows;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    NumRows = LISTVIEW__GetNumRows(pObj);
    GUI_UNLOCK_H(pObj);
    while (NumRows--) {
      LISTVIEW_DeleteRow(hObj, NumRows);
    }
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_DeleteAllRows_C(void);
  void LISTVIEW_DeleteAllRows_C(void) {}
#endif

/*************************** End of file ****************************/
