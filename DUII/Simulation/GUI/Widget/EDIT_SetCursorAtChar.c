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
File        : EDIT_SetCursorAtChar.c
Purpose     : Implementation of EDIT_SetCursorAtChar
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "EDIT.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
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
*       EDIT_SetCursorAtChar
*/
void EDIT_SetCursorAtChar(EDIT_Handle hObj, int Pos) {
  WM_LOCK();
  if (hObj) {
    EDIT__SetCursorPos(hObj, Pos);
    EDIT_Invalidate(hObj);
  }
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void EDIT_SetCursorAtChar_C(void);
void EDIT_SetCursorAtChar_C(void){}

#endif /* GUI_WINSUPPORT */
