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
File        : EDIT_SetpfUpdateBuffer.c
Purpose     : Implementation of edit widget
---------------------------END-OF-HEADER------------------------------
*/

#include "EDIT.h"
#include "EDIT_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*             Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       EDIT_SetpfUpdateBuffer
*/
void EDIT_SetpfUpdateBuffer(EDIT_Handle hObj, tEDIT_UpdateBuffer * pfUpdateBuffer) {
  if (hObj) {
    EDIT_Obj * pObj;
    WM_LOCK();
    pObj = EDIT_LOCK_H(hObj);
    pObj->pfUpdateBuffer = pfUpdateBuffer;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */

void EDIT_SetpfUpdateBuffer_C(void);
void EDIT_SetpfUpdateBuffer_C(void) {}

#endif /* GUI_WINSUPPORT */


