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
File        : LISTVIEW_SetLBorder.c
Purpose     : Implementation of LISTVIEW_SetLBorder
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
*       LISTVIEW_SetLBorder
*/
void LISTVIEW_SetLBorder(LISTVIEW_Handle hObj, unsigned BorderSize) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (pObj->LBorder != BorderSize) {
      pObj->LBorder = BorderSize;
      LISTVIEW__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTVIEW_SetLFreeBorder - KMC
*/
void LISTVIEW_SetLFreeBorder(LISTVIEW_Handle hObj, int BorderSize) {
  if (hObj) {
    LISTVIEW_Obj* pObj;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    if (pObj->LFreeBorder != BorderSize) {
      pObj->LFreeBorder = BorderSize;
      LISTVIEW__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetLBorder_C(void);
  void LISTVIEW_SetLBorder_C(void) {}
  void LISTVIEW_SetLFreeBorder_C(void);
  void LISTVIEW_SetLFreeBorder_C(void) {}
#endif

/*************************** End of file ****************************/
