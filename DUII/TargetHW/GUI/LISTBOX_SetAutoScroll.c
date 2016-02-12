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
File        : LISTBOX_SetAutoScroll.c
Purpose     : Implementation of LISTBOX_SetAutoScroll
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI_ARRAY.h"
#include "LISTBOX_Private.h"
#include "SCROLLBAR.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_SetAutoScrollH
*/
void LISTBOX_SetAutoScrollH(LISTBOX_Handle hObj, int State) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    char Flags, OldFlags;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    OldFlags = pObj->Flags;
    GUI_UNLOCK_H(pObj);
    Flags = OldFlags & (~LISTBOX_SF_AUTOSCROLLBAR_H);
    if (State) {
      Flags |= LISTBOX_SF_AUTOSCROLLBAR_H;
    } else {
      WM_SetScrollbarH(hObj, 0);
    }
    if (OldFlags != Flags) {
      pObj = LISTBOX_LOCK_H(hObj);
      pObj->Flags = Flags;
      GUI_UNLOCK_H(pObj);
      LISTBOX_UpdateScrollers(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_SetAutoScrollV
*/
void LISTBOX_SetAutoScrollV(LISTBOX_Handle hObj, int State) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    char Flags, OldFlags;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    OldFlags = pObj->Flags;
    GUI_UNLOCK_H(pObj);
    Flags = OldFlags & (~LISTBOX_SF_AUTOSCROLLBAR_V);
    if (State) {
      Flags |= LISTBOX_SF_AUTOSCROLLBAR_V;
    } else {
      WM_SetScrollbarV(hObj, 0);
    }
    if (OldFlags != Flags) {
      pObj = LISTBOX_LOCK_H(hObj);
      pObj->Flags = Flags;
      GUI_UNLOCK_H(pObj);
      LISTBOX_UpdateScrollers(hObj);
    }
    WM_UNLOCK();
  }
}


#else                            /* Avoid problems with empty object modules */
  void LISTBOX_SetAutoScroll_C(void) {}
#endif
