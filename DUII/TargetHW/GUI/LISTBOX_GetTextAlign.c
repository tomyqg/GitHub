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
File        : LISTBOX_GetTextAlign.c
Purpose     : Implementation of LISTBOX_GetTextAlign
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_SetTextAlign
*/
int LISTBOX_GetTextAlign(LISTBOX_Handle hObj) {
  int Align = 0;
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    Align = pObj->Props.Align;
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hObj);
    WM_UNLOCK();
  }
  return Align;
}

#else  /* avoid empty object files */

void LISTBOX_GetTextAlign_c(void);
void LISTBOX_GetTextAlign_c(void){}

#endif  /* #if GUI_WINSUPPORT */


