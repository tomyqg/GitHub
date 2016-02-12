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
File        : LISTVIEW_SetColumnWidth.c
Purpose     : Implementation of LISTVIEW_SetColumnWidth
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
*       LISTVIEW_SetColumnWidth
*/
void LISTVIEW_SetColumnWidth(LISTVIEW_Handle hObj, unsigned int Index, int Width) {
  if (hObj) {
    LISTVIEW_Obj * pObj;
    HEADER_Handle hHeader;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    hHeader = pObj->hHeader;
    GUI_UNLOCK_H(pObj);
    HEADER_SetItemWidth(hHeader, Index, Width);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetColumnWidth_C(void);
  void LISTVIEW_SetColumnWidth_C(void) {}
#endif

/*************************** End of file ****************************/
