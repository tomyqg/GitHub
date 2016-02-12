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
File        : FRAMEWIN_MinMaxRest.c
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
*           Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _InvalidateButton
*/
static void _InvalidateButton(FRAMEWIN_Obj * pObj, int Id) {
  WM_HWIN hChild;
  WM_HWIN hNext;
  WM_Obj * pChild;
  for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = hNext) {
    pChild = (WM_Obj *)GUI_LOCK_H(hChild);
    if (WM_GetId(hChild) == Id) {
      WM_InvalidateWindow(hChild);
    }
    hNext = pChild->hNext;
    GUI_UNLOCK_H(pChild);
  }
}

/*********************************************************************
*
*       _RestoreMinimized
*/
static void _RestoreMinimized(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  pObj = FRAMEWIN_LOCK_H(hObj);
  /* When window was minimized, restore it */
  if (pObj->Flags & FRAMEWIN_SF_MINIMIZED) {
    int OldHeight;
    int NewHeight;
    OldHeight = 1 + pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0;
    NewHeight = 1 + pObj->rRestore.y1 - pObj->rRestore.y0;
    GUI_UNLOCK_H(pObj);
    WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
    pObj = FRAMEWIN_LOCK_H(hObj);
    WM_ShowWindow(pObj->hClient);
    WM_ShowWindow(pObj->hMenu);
    FRAMEWIN__UpdatePositions(hObj);
    pObj->Flags &= ~FRAMEWIN_SF_MINIMIZED;
    _InvalidateButton(pObj, GUI_ID_MINIMIZE);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _RestoreMaximized
*/
static void _RestoreMaximized(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  pObj = FRAMEWIN_LOCK_H(hObj);
  /* When window was maximized, restore it */
  if (pObj->Flags & FRAMEWIN_SF_MAXIMIZED) {
    GUI_RECT r = pObj->rRestore;
    WM_MoveTo(hObj, r.x0, r.y0);
    WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
    FRAMEWIN__UpdatePositions(hObj);
    pObj->Flags &= ~FRAMEWIN_SF_MAXIMIZED;
    _InvalidateButton(pObj, GUI_ID_MAXIMIZE);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MinimizeFramewin
*/
static void _MinimizeFramewin(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  pObj = FRAMEWIN_LOCK_H(hObj);
  _RestoreMaximized(hObj);
  /* When window is not minimized, minimize it */
  if ((pObj->Flags & FRAMEWIN_SF_MINIMIZED) == 0) {
    int OldHeight = pObj->Widget.Win.Rect.y1 - pObj->Widget.Win.Rect.y0 + 1;
    int NewHeight = FRAMEWIN__CalcTitleHeight(pObj) + pObj->Widget.pEffect->EffectSize * 2 + 2;    
    pObj->rRestore = pObj->Widget.Win.Rect;
    WM_HideWindow(pObj->hClient);
    WM_HideWindow(pObj->hMenu);
    pObj->Flags |= FRAMEWIN_SF_MINIMIZED;
    _InvalidateButton(pObj, GUI_ID_MINIMIZE);
    GUI_UNLOCK_H(pObj);
    WM_ResizeWindow(hObj, 0, NewHeight - OldHeight);
    FRAMEWIN__UpdatePositions(hObj);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _MaximizeFramewin
*/
static void _MaximizeFramewin(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  pObj = FRAMEWIN_LOCK_H(hObj);
  _RestoreMinimized(hObj);
  /* When window is not maximized, maximize it */
  if ((pObj->Flags & FRAMEWIN_SF_MAXIMIZED) == 0) {
    WM_HWIN hParent;
    WM_Obj * pParent;
    GUI_RECT r;
    hParent = pObj->Widget.Win.hParent;
    pParent = (WM_Obj *)GUI_LOCK_H(hParent);
    r = pParent->Rect;
    if (pParent->hParent == 0) {
      r.x1 = LCD_GetXSize();
      r.y1 = LCD_GetYSize();
    }
    GUI_UNLOCK_H(pParent);
    pObj->rRestore = pObj->Widget.Win.Rect;
    pObj->Flags |= FRAMEWIN_SF_MAXIMIZED;
    _InvalidateButton(pObj, GUI_ID_MAXIMIZE);
    GUI_UNLOCK_H(pObj);
    WM_MoveTo(hObj, r.x0, r.y0);
    WM_SetSize(hObj, r.x1 - r.x0 + 1, r.y1 - r.y0 + 1);
    FRAMEWIN__UpdatePositions(hObj);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_Minimize
*/
void FRAMEWIN_Minimize(FRAMEWIN_Handle hObj) {
  if (hObj) {
    WM_LOCK();
    _MinimizeFramewin(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_Maximize
*/
void FRAMEWIN_Maximize(FRAMEWIN_Handle hObj) {
  if (hObj) {
    WM_LOCK();
    _MaximizeFramewin(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_Restore
*/
void FRAMEWIN_Restore(FRAMEWIN_Handle hObj) {
  if (hObj) {
    WM_LOCK();
    _RestoreMinimized(hObj);
    _RestoreMaximized(hObj);
    WM_UNLOCK();
  }
}

#else
  void FRAMEWIN_MinMaxRest_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
