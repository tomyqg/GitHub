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
File        : FRAMEWIN_IsMinMax.c
Purpose     : Add. framewin routines
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "WIDGET.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_IsMinimized
*/
int FRAMEWIN_IsMinimized(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    r = (pObj->Flags & FRAMEWIN_SF_MINIMIZED) ? 1 : 0;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       FRAMEWIN_IsMaximized
*/
int FRAMEWIN_IsMaximized(FRAMEWIN_Handle hObj) {
  int r = 0;
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    r = (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) ? 1 : 0;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else
  void FRAMEWIN_IsMinMax_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
