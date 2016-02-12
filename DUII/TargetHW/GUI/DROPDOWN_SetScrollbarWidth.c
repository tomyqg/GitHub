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
File        : DROPDOWN_SetScrollbarWidth.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX.h"
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
*       DROPDOWN_SetScrollbarWidth
*/
void DROPDOWN_SetScrollbarWidth(DROPDOWN_Handle hObj, unsigned Width) {
  DROPDOWN_Obj * pObj;
  U8 ScrollbarWidth;
  WM_HWIN hListWin;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    ScrollbarWidth = pObj->ScrollbarWidth;
    hListWin = pObj->hListWin;
    if (Width != (unsigned)ScrollbarWidth) {
      pObj->ScrollbarWidth = Width;
      GUI_UNLOCK_H(pObj);
      if (hListWin) {
        LISTBOX_SetScrollbarWidth(hListWin, Width);
      }
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }  
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_SetScrollbarWidth_C(void);
  void DROPDOWN_SetScrollbarWidth_C(void) {}
#endif
