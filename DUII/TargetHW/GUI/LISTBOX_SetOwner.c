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
File        : LISTBOX_SetOwner.c
Purpose     : Implementation of LISTBOX_SetOwner
---------------------------END-OF-HEADER------------------------------
*/

#include "LISTBOX_Private.h"


#if GUI_WINSUPPORT


/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/

/*********************************************************************
*
*       LISTBOX_SetOwner
*/
void LISTBOX_SetOwner(LISTBOX_Handle hObj, WM_HWIN hOwner) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    pObj->hOwner = hOwner;
    GUI_UNLOCK_H(pObj);
    LISTBOX__InvalidateInsideArea(hObj);
    WM_UNLOCK();
  }
}



#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetOwner_C(void) {}
#endif
