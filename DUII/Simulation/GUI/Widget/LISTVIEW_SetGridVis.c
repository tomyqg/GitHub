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
File        : LISTVIEW_SetGridVis.c
Purpose     : Implementation of LISTVIEW_SetGridVis
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
*       LISTVIEW_SetGridVis
*/
int LISTVIEW_SetGridVis(LISTVIEW_Handle hObj, int Show) {
  int ShowGrid = 0;
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    ShowGrid = pObj->ShowGrid;
    if (Show != ShowGrid) {
      pObj->ShowGrid = Show;
      LISTVIEW__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    if (Show != ShowGrid) {
      LISTVIEW__UpdateScrollParas(hObj);
    }
    WM_UNLOCK();
  }
  return ShowGrid;
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetGridVis_C(void);
  void LISTVIEW_SetGridVis_C(void) {}
#endif

/*************************** End of file ****************************/
