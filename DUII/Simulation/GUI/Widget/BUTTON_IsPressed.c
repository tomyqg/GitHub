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
File        : BUTTON_IsPressed.c
Purpose     : Button widget, various (optional) Get routines
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include "BUTTON.h"
#include "BUTTON_Private.h"
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
*       BUTTON_IsPressed
*/
unsigned BUTTON_IsPressed(BUTTON_Handle hObj) {
  unsigned r = 0;
  if (hObj) {
    BUTTON_Obj* pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    r = (pObj->Widget.State & BUTTON_STATE_PRESSED) ? 1 : 0;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON_IsPressed_C(void) {}
#endif
