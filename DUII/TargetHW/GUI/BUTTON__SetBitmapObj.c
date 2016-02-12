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
File        : BUTTON__SetBitmapObj.c
Purpose     : Button bitmap support, private function
---------------------------END-OF-HEADER------------------------------
*/

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
*       BUTTON__SetDrawObj
*/
void BUTTON__SetDrawObj(BUTTON_Handle hObj, int Index, GUI_DRAW_HANDLE hDrawObj) {
  if (hObj) {
    BUTTON_Obj* pObj;
    WM_LOCK();
    pObj = BUTTON_LOCK_H(hObj);
    if ((unsigned int)Index <= GUI_COUNTOF(pObj->ahDrawObj)) {
      GUI_ALLOC_FreePtr(&pObj->ahDrawObj[Index]);
      pObj->ahDrawObj[Index] = hDrawObj;
      BUTTON_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void BUTTON__SetBitmapObj_C(void) {}
#endif
