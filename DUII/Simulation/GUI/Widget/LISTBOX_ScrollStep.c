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
File        : LISTBOX_ScrollStep.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "LISTBOX_Private.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       LISTBOX_SetScrollStepH
*/
void LISTBOX_SetScrollStepH(LISTBOX_Handle hObj, int Value) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    pObj->Props.ScrollStepH = Value;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_GetScrollStepH
*/
int LISTBOX_GetScrollStepH(LISTBOX_Handle hObj) {
  int Value = 0;
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    Value = pObj->Props.ScrollStepH;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Value;
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_ScrollStep_C(void) {}
#endif
