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
File        : HEADER_SetDragLimit.c
Purpose     : Implementation of HEADER_SetDragLimit
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetDragLimit
*/
void HEADER_SetDragLimit(HEADER_Handle hObj, unsigned OnOff) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (OnOff) {
      pObj->DragLimit = 1;
    } else {
      pObj->DragLimit = 0;
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void HEADER_SetDragLimit_C(void);
  void HEADER_SetDragLimit_C(void) {}
#endif
