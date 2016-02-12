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
File        : LISTVIEW_UserData.c
Purpose     : Implementation of LISTVIEW_SetUserData() and LISTVIEW_GetUserData()
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
*       LISTVIEW_SetUserData
*/
void LISTVIEW_SetUserData(LISTVIEW_Handle hObj, unsigned Row, U32 UserData) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (Row < LISTVIEW__GetNumRows(pObj)) {
      LISTVIEW_ROW * pRow;
      pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(pObj->RowArray, Row);
      pRow->UserData = UserData;
      GUI_UNLOCK_H(pRow);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_GetUserData
*/
U32 LISTVIEW_GetUserData(LISTVIEW_Handle hObj, unsigned Row) {
  U32 UserData;
  UserData = 0;
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (Row < LISTVIEW__GetNumRows(pObj)) {
      LISTVIEW_ROW * pRow;
      pRow = (LISTVIEW_ROW *)GUI_ARRAY_GetpItemLocked(pObj->RowArray, Row);
      UserData = pRow->UserData;
      GUI_UNLOCK_H(pRow);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return UserData;
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_UserData_C(void);
  void LISTVIEW_UserData_C(void) {}
#endif

/*************************** End of file ****************************/
