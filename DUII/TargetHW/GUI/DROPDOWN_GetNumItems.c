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
File        : DROPDOWN_GetNumItems.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_GetNumItems
*/
int DROPDOWN_GetNumItems(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj* pObj;
  int r = 0;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    r = DROPDOWN__GetNumItems(pObj);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_GetNumItems_C(void);
  void DROPDOWN_GetNumItems_C(void) {}
#endif
