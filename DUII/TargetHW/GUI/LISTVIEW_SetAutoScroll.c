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
File        : LISTVIEW_SetAutoScroll.c
Purpose     : Implementation of LISTVIEW_SetAutoScrollH() and LISTVIEW_SetAutoScrollV()
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI_ARRAY.h"
#include "LISTVIEW_Private.h"
#include "SCROLLBAR.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetAutoScroll
*/
static void _SetAutoScroll(LISTVIEW_Handle hObj, int OnOff, int HV_Flag) {
  if (hObj) {
    LISTVIEW_Obj * pObj;
    char Flags, FlagsOld;
    WM_LOCK();
    pObj = LISTVIEW_LOCK_H(hObj);
    FlagsOld = pObj->Flags;
    Flags = FlagsOld & (~HV_Flag);
    if (OnOff) {
      Flags |= HV_Flag;
    }
    if (Flags != FlagsOld) {
      pObj->Flags = Flags;
    }
    GUI_UNLOCK_H(pObj);
    if (Flags != FlagsOld) {
      LISTVIEW__UpdateScrollParas(hObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTVIEW_SetAutoScrollH
*/
void LISTVIEW_SetAutoScrollH(LISTVIEW_Handle hObj, int State) {
  _SetAutoScroll(hObj, State, LISTVIEW_SF_AUTOSCROLLBAR_H);
}

/*********************************************************************
*
*       LISTVIEW_SetAutoScrollV
*/
void LISTVIEW_SetAutoScrollV(LISTVIEW_Handle hObj, int State) {
  _SetAutoScroll(hObj, State, LISTVIEW_SF_AUTOSCROLLBAR_V);
}

#else                            /* Avoid problems with empty object modules */
  void LISTVIEW_SetAutoScroll_C(void) {}
#endif
