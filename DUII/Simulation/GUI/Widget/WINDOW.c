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
File        : WINDOW.c
Purpose     : Window routines
---------------------------END-OF-HEADER------------------------------
*/


#include <stdlib.h>
#include <string.h>
#include "DIALOG.h"
#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

#ifndef WINDOW_BKCOLOR_DEFAULT
  #define WINDOW_BKCOLOR_DEFAULT 0xC0C0C0
#endif

/*********************************************************************
*
*       Object definition
*
**********************************************************************
*/

typedef struct {
  WIDGET Widget;
  WM_CALLBACK * cb;
  WM_HWIN hFocussedChild;
  WM_DIALOG_STATUS * pDialogStatus;
  GUI_COLOR BkColor;
} WINDOW_OBJ;

#define WINDOW_LOCK(h) (WINDOW_OBJ *)GUI_LOCK_H(h)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

GUI_COLOR WINDOW__DefaultBkColor = WINDOW_BKCOLOR_DEFAULT;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _OnChildHasFocus
*/
static void _OnChildHasFocus(WM_HWIN hObj, const WM_MESSAGE * pMsg) {
  WINDOW_OBJ * pObj;
  pObj = WINDOW_LOCK(hObj);
  if (pMsg->Data.p) {
    const WM_NOTIFY_CHILD_HAS_FOCUS_INFO * pInfo;
    int IsDesc;
    pInfo = (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)pMsg->Data.p;
    IsDesc = WM__IsAncestorOrSelf(pInfo->hNew, hObj);
    if (!IsDesc) {  /* A child has received the focus, Framewindow needs to be activated */
      /* Remember the child which had the focus so we can reactive this child */
      if (WM__IsAncestor(pInfo->hOld, hObj)) {
        pObj->hFocussedChild = pInfo->hOld;
      }
    }
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       WINDOW_Callback
*/
void WINDOW_Callback(WM_MESSAGE* pMsg) {
  WM_HWIN hObj;
  WINDOW_OBJ * pObj;
  WM_CALLBACK * cb;
  int r;
  WM_HWIN hFocussedChild;
  GUI_COLOR BkColor;

  hObj = pMsg->hWin;
  pObj = WINDOW_LOCK(hObj);
  hFocussedChild = pObj->hFocussedChild;
  cb             = pObj->cb;
  BkColor        = pObj->BkColor;
  GUI_UNLOCK_H(pObj);
  r = 0;
  switch (pMsg->MsgId) {
  case WM_HANDLE_DIALOG_STATUS:
    pObj = WINDOW_LOCK(hObj);
    if (pMsg->Data.p) {                           /* set pointer to Dialog status */
      pObj->pDialogStatus = (WM_DIALOG_STATUS *)pMsg->Data.p;
    } else {                                      /* return pointer to Dialog status */
      pMsg->Data.p = pObj->pDialogStatus;      
    }
    GUI_UNLOCK_H(pObj);
    r = 1;
    break;
  case WM_SET_FOCUS:
    if (pMsg->Data.v) {   /* Focus received */
      if (hFocussedChild && (hFocussedChild != hObj)) {
        if (WM_IsWindow(hFocussedChild)) {
          WM_SetFocus(hFocussedChild);
        } else {
          hFocussedChild = WM_SetFocusOnNextChild(hObj);
          pObj = WINDOW_LOCK(hObj);
          pObj->hFocussedChild = hFocussedChild;
          GUI_UNLOCK_H(pObj);
        }
      } else {
        hFocussedChild = WM_SetFocusOnNextChild(hObj);
        pObj = WINDOW_LOCK(hObj);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
      }
      pMsg->Data.v = 0;   /* Focus change accepted */
    }
    r = 1;
    break;
  case WM_GET_ACCEPT_FOCUS:
    WIDGET_HandleActive(hObj, pMsg);
    r = 1;
    break;
  case WM_NOTIFY_CHILD_HAS_FOCUS:
    _OnChildHasFocus(hObj, pMsg);
    r = 1;
    break;
  case WM_KEY:
    if (((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt > 0) {
      int Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
      switch (Key) {
      case GUI_KEY_BACKTAB:
        hFocussedChild = WM_SetFocusOnPrevChild(hObj);
        pObj = WINDOW_LOCK(hObj);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
        return;
      case GUI_KEY_TAB:
        hFocussedChild = WM_SetFocusOnNextChild(hObj);
        pObj = WINDOW_LOCK(hObj);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
        return;
      }
    }
    break;
  case WM_PAINT:
    if (BkColor != GUI_INVALID_COLOR) {
      LCD_SetBkColor(BkColor);
      GUI_Clear();
    }
    break;
  case WM_GET_BKCOLOR:
    pMsg->Data.Color = BkColor;
    r = 1;
    break;
  }
  if (r == 0) {
    if (cb) {
      (*cb)(pMsg);
    } else {
      WM_DefaultProc(pMsg);
    }
  }
}

/*********************************************************************
*
*       WINDOW_SetBkColor
*/
void WINDOW_SetBkColor(WM_HWIN hObj, GUI_COLOR Color) {
  WINDOW_OBJ * pObj;
  
  WM_LOCK();
  pObj = WINDOW_LOCK(hObj);
  pObj->BkColor = Color;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/
/*********************************************************************
*
*       WINDOW_CreateEx
*/
WM_HWIN WINDOW_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, WM_CALLBACK * cb) {
  WM_HWIN hObj;
  WINDOW_OBJ * pObj;

  GUI_USE_PARA(ExFlags);
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WinFlags, WINDOW_Callback, sizeof(WINDOW_OBJ) - sizeof(WM_Obj));
  if (hObj) {
    pObj = WINDOW_LOCK(hObj);
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    pObj->cb             = cb;
    pObj->hFocussedChild = 0;
    pObj->BkColor        = WINDOW__DefaultBkColor;
    GUI_UNLOCK_H(pObj);
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       WINDOW_CreateIndirect
*/
WM_HWIN WINDOW_CreateIndirect(const GUI_WIDGET_CREATE_INFO * pCreateInfo, WM_HWIN hWinParent, int x0, int y0, WM_CALLBACK * cb) {
  WM_HWIN hObj;

  hObj = WINDOW_CreateEx(pCreateInfo->x0 + x0, pCreateInfo->y0 + y0, pCreateInfo->xSize, pCreateInfo->ySize, hWinParent, pCreateInfo->Flags, 0, pCreateInfo->Id, cb);
  return hObj;
}

#else
  void WINDOW_c(void);
  void WINDOW_c(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */
