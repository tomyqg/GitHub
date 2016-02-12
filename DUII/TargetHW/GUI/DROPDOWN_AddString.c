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
File        : DROPDOWN_AddString.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
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
*       DROPDOWN_AddString
*/
void DROPDOWN_AddString(DROPDOWN_Handle hObj, const char * s) {
  DROPDOWN_Obj * pObj;
  GUI_ARRAY Handles;
  if (hObj && s) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    Handles = pObj->Handles;
    GUI_UNLOCK_H(pObj);
    GUI_ARRAY_AddItem(Handles, s, strlen(s) + 1);
    DROPDOWN_Invalidate(hObj);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_AddString_C(void);
  void DROPDOWN_AddString_C(void) {}
#endif
