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
File        : FRAMEWIN_ButtonMin.c
Purpose     : 
---------------------------END-OF-HEADER------------------------------
*/


#include <stdlib.h>
#include <string.h>
#include "GUI_Private.h"
#include "FRAMEWIN_Private.h"
#include "BUTTON.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       Callback
*
* This is the overwritten callback routine for the button.
* The primary reason for overwriting it is that we define the default
* action of the Framewindow here.
* It works as follows:
* - User clicks and releases the button
*   -> BUTTON sends WM_NOTIFY_PARENT to FRAMEWIN
*     -> FRAMEWIN either a) reacts or b)sends WM_NOTIFY_PARENT_REFLECTION back
*       In case of a) This module reacts !
*/
static void _cbMin(WM_MESSAGE * pMsg) {
  if (pMsg->MsgId == WM_NOTIFY_PARENT_REFLECTION) {
    WM_HWIN hObj;
    FRAMEWIN_Obj * pObj;
    U16 Flags;
    hObj = pMsg->hWinSrc;
    pObj = FRAMEWIN_LOCK_H(hObj);
    Flags = pObj->Flags;
    GUI_UNLOCK_H(pObj);
    if (Flags & FRAMEWIN_SF_MINIMIZED) {
      FRAMEWIN_Restore(hObj);
    } else {
      FRAMEWIN_Minimize(hObj);
    }
    return;                                       /* We are done ! */
  }
  BUTTON_Callback(pMsg);
}

/*********************************************************************
*
*       _DrawMin
*/
static void _DrawMin(WM_HWIN hObj) {
  GUI_RECT r;
  int i, Size;
  WM_GetInsideRectEx(hObj, &r);
  WM_ADDORG(r.x0, r.y0);
  WM_ADDORG(r.x1, r.y1);
  Size = (r.x1 - r.x0 + 1) >> 1;
  WM_ITERATE_START(&r); {
    for (i = 1; i < Size; i++) {
      LCD_DrawHLine(r.x0 + i, r.y1 - i - (Size >> 1), r.x1 - i);
    }
  } WM_ITERATE_END();
}

/*********************************************************************
*
*       _DrawRestore
*/
static void _DrawRestore(WM_HWIN hObj) {
  GUI_RECT r;
  int i, Size;
  WM_GetInsideRectEx(hObj, &r);
  WM_ADDORG(r.x0, r.y0);
  WM_ADDORG(r.x1, r.y1);
  Size = (r.x1 - r.x0 + 1) >> 1;
  WM_ITERATE_START(&r); {
    for (i = 1; i < Size; i++) {
      LCD_DrawHLine(r.x0 + i, r.y0 + i + (Size >> 1), r.x1 - i);
    }
  } WM_ITERATE_END();
}

/*********************************************************************
*
*       _Draw
*/
static void _Draw(WM_HWIN hObj) {
  FRAMEWIN_Obj * pFrame;
  U16 Flags;
  WM_HWIN hFrame;

  hFrame = WM_GetParent(hObj);
  pFrame = FRAMEWIN_LOCK_H(hFrame);
  Flags = pFrame->Flags;
  GUI_UNLOCK_H(pFrame);
  GUI_SetColor(GUI_BLACK);
  if (Flags & FRAMEWIN_SF_MINIMIZED) {
    _DrawRestore(hObj);
  } else {
    _DrawMin(hObj);
  }
}

/*********************************************************************
*
*        Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_AddMinButton
*/
WM_HWIN FRAMEWIN_AddMinButton(FRAMEWIN_Handle hObj, int Flags, int Off) {
  WM_HWIN hButton;
  hButton = FRAMEWIN_AddButton(hObj, Flags, Off, GUI_ID_MINIMIZE);
  BUTTON_SetSelfDraw(hButton, 0, &_Draw);
  WM_SetCallback(hButton, _cbMin);
  return hButton;
}

#else
  void FRAMEWIN_ButtonMin_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
