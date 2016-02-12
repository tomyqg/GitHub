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
File        : MENU_Popup.c
Purpose     : Implementation of menu widget
---------------------------END-OF-HEADER------------------------------
*/

#include "MENU.h"
#include "MENU_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU_Popup
*/
void MENU_Popup(MENU_Handle hObj, WM_HWIN hDestWin, int x, int y, int xSize, int ySize, int Flags) {
  GUI_USE_PARA(Flags);
  if (hObj && hDestWin) {
    MENU_Obj * pObj;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    pObj->Flags  |= MENU_SF_POPUP;
    pObj->Width   = ((xSize > 0) ? xSize : 0);
    pObj->Height  = ((ySize > 0) ? ySize : 0);
    GUI_UNLOCK_H(pObj);
    x            += WM_GetWindowOrgX(hDestWin);
    y            += WM_GetWindowOrgY(hDestWin);
    MENU_SetOwner(hObj, hDestWin);
    WM_AttachWindowAt(hObj, WM_HBKWIN, x, y);
    MENU__SendMenuMessage(hDestWin, hObj, MENU_ON_OPEN, 0);
    MENU__SendMenuMessage(hObj, hDestWin, MENU_ON_INITMENU, 0);
    WM_UNLOCK();
  }
}

#else  /* avoid empty object files */
  void MENU_Popup_C(void) {}
#endif

/*************************** End of file ****************************/
