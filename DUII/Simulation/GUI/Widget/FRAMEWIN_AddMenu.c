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
File        : FRAMEWIN_AddMenu.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "MENU.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_AddMenu
*/
void FRAMEWIN_AddMenu(FRAMEWIN_Handle hObj, WM_HWIN hMenu) {
  if (hObj) {
    int TitleHeight, BorderSize, IBorderSize = 0;
    int x0, y0, xSize;
    FRAMEWIN_Obj * pObj;

    WM_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
    BorderSize = pObj->Props.BorderSize;
    if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
      IBorderSize = pObj->Props.IBorderSize;
    }
    x0     = BorderSize;
    y0     = BorderSize + TitleHeight + IBorderSize;
    xSize  = WM__GetWindowSizeX(&pObj->Widget.Win);
    xSize -= BorderSize * 2;
    pObj->hMenu = hMenu;
    if (pObj->cb) {
      MENU_SetOwner(hMenu, pObj->hClient);
    }
    MENU_Attach(hMenu, hObj, x0, y0, xSize, 0, 0);
    WM_SetStayOnTop(hMenu, 0);
    WM_SetAnchor(hMenu, WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT);
    FRAMEWIN__UpdatePositions(hObj);
    FRAMEWIN_Invalidate(hObj);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else
  void FRAMEWIN_AddMenu_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
