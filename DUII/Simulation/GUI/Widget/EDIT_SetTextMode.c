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
File        : EDIT_SetTextMode
Purpose     : Implementation of EDIT_SetTextMode
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
*       EDIT_SetTextMode
*/
void EDIT_SetTextMode(EDIT_Handle hEdit) {
  EDIT_Obj * pObj;
  WM_LOCK();
  if (hEdit) {
    pObj = EDIT_LOCK_H(hEdit);
    pObj->pfAddKeyEx    = NULL;
    pObj->pfUpdateBuffer= NULL;
    pObj->CurrentValue  = 0;
    pObj->CursorPos     = 0;
    pObj->Min           = 0;
    pObj->Max           = 0;
    pObj->NumDecs       = 0;
    pObj->Flags         = 0;
    pObj->EditMode      = GUI_EDIT_MODE_INSERT;
    EDIT_SetText(hEdit, "");
    EDIT__SetCursorPos(hEdit, pObj->CursorPos);
    GUI_UNLOCK_H(pObj);
    WM_Invalidate(hEdit);
  }
  WM_UNLOCK();
}

#else  /* avoid empty object files */

void EDIT_SetTextMode_C(void);
void EDIT_SetTextMode_C(void){}

#endif /* GUI_WINSUPPORT */
