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
File        : LISTBOX_Font.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "LISTBOX_Private.h"
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
*       LISTBOX_SetFont
*/
void LISTBOX_SetFont(LISTBOX_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  LISTBOX_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    pObj->Props.pFont = pFont;
    pObj->ContentSizeX = 0;
    GUI_UNLOCK_H(pObj);
    LISTBOX_InvalidateItem(hObj, LISTBOX_ALL_ITEMS);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_GetFont
*/
const GUI_FONT GUI_UNI_PTR * LISTBOX_GetFont(LISTBOX_Handle hObj) {
  const GUI_FONT GUI_UNI_PTR * pFont = NULL;
  LISTBOX_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    pFont = pObj->Props.pFont;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return pFont;
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_Font_C(void) {}
#endif
