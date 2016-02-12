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
File        : FRAMEWIN_SetOwnerDraw.c
Purpose     : Setting of optional drawing routine
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_SetOwnerDraw
*/
void FRAMEWIN_SetOwnerDraw(FRAMEWIN_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawItem) {
  GUI_LOCK();
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    pObj = FRAMEWIN_LOCK_H(hObj);
    pObj->pfDrawItem = pfDrawItem;
    GUI_UNLOCK_H(pObj);
    FRAMEWIN_Invalidate(hObj);
  }
  GUI_UNLOCK();
}

#else
  void FRAMEWIN_SetOwnerDraw_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

