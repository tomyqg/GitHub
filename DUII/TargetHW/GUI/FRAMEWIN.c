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
File        : FRAMEWIN.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "FRAMEWIN_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

#ifndef   FRAMEWIN_DRAW_SKIN_DEFAULT
  #define FRAMEWIN_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Private data
*
**********************************************************************
*/
FRAMEWIN_PROPS FRAMEWIN__DefaultProps = {
  FRAMEWIN_DEFAULT_FONT,
  { FRAMEWIN_BARCOLOR_INACTIVE_DEFAULT,
    FRAMEWIN_BARCOLOR_ACTIVE_DEFAULT,
  },
  { FRAMEWIN_TEXTCOLOR0_DEFAULT,
    FRAMEWIN_TEXTCOLOR1_DEFAULT,
  },
  FRAMEWIN_CLIENTCOLOR_DEFAULT,
  {
    FRAMEWIN__GetBorderSize,
    FRAMEWIN_DRAW_SKIN_DEFAULT
  },
  FRAMEWIN_TITLEHEIGHT_DEFAULT,
  FRAMEWIN_BORDER_DEFAULT,
  FRAMEWIN_IBORDER_DEFAULT,
  FRAMEWIN_TEXTALIGN_DEFAULT,
};

WIDGET_SKIN const * FRAMEWIN__pSkinDefault = &FRAMEWIN__SkinClassic;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static I16 FRAMEWIN__MinVisibility = 5;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetActive
*/
static void _SetActive(FRAMEWIN_Handle hObj, int State) {
  FRAMEWIN_Obj * pObj;
  pObj = FRAMEWIN_LOCK_H(hObj);
  if        (State && !(pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
    pObj->Flags |= FRAMEWIN_CF_ACTIVE;
    FRAMEWIN_Invalidate(hObj);
  } else if (!State && (pObj->Flags & FRAMEWIN_SF_ACTIVE)) {
    pObj->Flags &= ~FRAMEWIN_CF_ACTIVE;
    FRAMEWIN_Invalidate(hObj);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(FRAMEWIN_Handle hObj, WM_MESSAGE * pMsg) {
  FRAMEWIN_Obj * pObj;
  const GUI_PID_STATE * pState;
  U16 Flags;
  #if (FRAMEWIN_ALLOW_DRAG_ON_FRAME == 0)
  I16 BorderSize;
  I16 IBorderSize;
  #endif

  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  pObj = FRAMEWIN_LOCK_H(hObj);
  Flags = pObj->Flags;
  #if (FRAMEWIN_ALLOW_DRAG_ON_FRAME == 0)
  BorderSize = pObj->Props.BorderSize;
  IBorderSize = pObj->Props.IBorderSize;
  #endif
  GUI_UNLOCK_H(pObj);
  if (pMsg->Data.p) {  // Something happened in our area (pressed or released)
    if (pState->Pressed) {
      if (!(Flags & FRAMEWIN_SF_ACTIVE)) {
        WM_SetFocus(hObj);
      }
      WM_BringToTop(hObj);
      if ((Flags & FRAMEWIN_SF_MOVEABLE) && (Flags & FRAMEWIN_SF_DRAGGING)) {
        int LimitTop;
        #if (FRAMEWIN_ALLOW_DRAG_ON_FRAME)
          LimitTop = 0;
        #else
          FRAMEWIN_POSITIONS Pos;
          FRAMEWIN__CalcPositions(hObj, &Pos);
          LimitTop = BorderSize + Pos.TitleHeight - 1 + IBorderSize;
          if ((pState->y <= LimitTop) || WM_HasCaptured(hObj))
        #endif
        {
          if ((Flags & FRAMEWIN_SF_MAXIMIZED) == 0) {
            WM_SetCaptureMove(hObj, pState, FRAMEWIN__MinVisibility, LimitTop);
          }
        }
      }
    } else {
      pObj = FRAMEWIN_LOCK_H(hObj);
      pObj->Flags &= ~FRAMEWIN_SF_DRAGGING;
      GUI_UNLOCK_H(pObj);
    }
  }
}

/*********************************************************************
*
*       _OnChildHasFocus
*
* Function:
*   A child has received or lost the focus.
*   The basic idea is to make sure the framewindow is active if a
*   descendant has the focus.
*   If the focus travels from one desc. to an other, there is no need
*   to make the framewindow inactive and active again.
*   Avoiding this complicates the code a litlle, but avoids flicker
*   and waste of CPU load.
*   
*/
static void _OnChildHasFocus(FRAMEWIN_Handle hObj, WM_MESSAGE *pMsg) {
  FRAMEWIN_Obj * pObj;
  if (pMsg->Data.p) {
    const WM_NOTIFY_CHILD_HAS_FOCUS_INFO * pInfo = (const WM_NOTIFY_CHILD_HAS_FOCUS_INFO *)pMsg->Data.p;
    int IsDesc = WM__IsAncestorOrSelf(pInfo->hNew, hObj);
    if (IsDesc) {                         // A child has received the focus, Framewindow needs to be activated
      _SetActive(hObj, 1);
    } else {                  // A child has lost the focus, we need to deactivate
      _SetActive(hObj, 0);
      //
      // Remember the child which had the focus so we can reactive this child
      //
      if (WM__IsAncestor(pInfo->hOld, hObj)) {
        pObj = FRAMEWIN_LOCK_H(hObj);
        pObj->hFocussedChild = pInfo->hOld;
        GUI_UNLOCK_H(pObj);
      }
    }
  }
}

/*********************************************************************
*
*       Client Callback
*/
static void FRAMEWIN__cbClient(WM_MESSAGE * pMsg) {
  WM_HWIN hWin, hParent;
  FRAMEWIN_Obj * pObj;
  WM_CALLBACK * cb;
  GUI_COLOR ClientColor;
  WM_HWIN hFocussedChild;

  hWin    = pMsg->hWin;
  hParent = WM_GetParent(pMsg->hWin);
  pObj = (FRAMEWIN_Obj *)GUI_LOCK_H(hParent); // Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point
  cb = pObj->cb;
  ClientColor = pObj->Props.ClientColor;
  hFocussedChild = pObj->hFocussedChild;
  GUI_UNLOCK_H(pObj);
  switch (pMsg->MsgId) {
  case WM_PAINT:
    if (ClientColor != GUI_INVALID_COLOR) {
      LCD_SetBkColor(ClientColor);
      GUI_Clear();
    }
    //
    // Give the user callback  a chance to draw.
    // Note that we can not run into the bottom part, as this passes the parents handle
    //
    if (cb) {
	    WM_MESSAGE Msg;
      Msg      = *pMsg;
      Msg.hWin = hWin;
      (*cb)(&Msg);
    }
    return;
  case WM_SET_FOCUS:
    if (pMsg->Data.v) {     // Focus received
      if (hFocussedChild && (hFocussedChild != hWin)) {
        WM_SetFocus(hFocussedChild);
      } else {
        hFocussedChild = WM_SetFocusOnNextChild(hWin);
        pObj = FRAMEWIN_LOCK_H(hParent);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
      }
      pMsg->Data.v = 0;     // Focus change accepted
    }
    return;
  case WM_GET_ACCEPT_FOCUS:
    WIDGET_HandleActive(hParent, pMsg);
    return;
  case WM_KEY:
    if (((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt > 0) {
      int Key = ((const WM_KEY_INFO*)(pMsg->Data.p))->Key;
      switch (Key) {
      case GUI_KEY_BACKTAB:
        hFocussedChild = WM_SetFocusOnPrevChild(hWin);
        pObj = FRAMEWIN_LOCK_H(hParent);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
        return;
      case GUI_KEY_TAB:
        hFocussedChild = WM_SetFocusOnNextChild(hWin);
        pObj = FRAMEWIN_LOCK_H(hParent);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
        return;
      }
    }
    break;	                       // Send to parent by not doing anything
  case WM_GET_BKCOLOR:
    pMsg->Data.Color = ClientColor;
    return;                       // Message handled
  case WM_GET_INSIDE_RECT:        // This should not be passed to parent ... (We do not want parents coordinates
  case WM_GET_ID:                 // This should not be passed to parent ... (Possible recursion problem
  case WM_GET_CLIENT_WINDOW:      // return handle to client window. For most windows, there is no seperate client window, so it is the same handle
    WM_DefaultProc(pMsg);
    return;                       // We are done !
  }
  //
  // Call user callback. Note that the user callback gets the handle of the Framewindow itself, NOT the Client.
  //
  if (cb) {
    pMsg->hWin = hParent;
    (*cb)(pMsg);
  } else {
    WM_DefaultProc(pMsg);
  }
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
FRAMEWIN_Obj * FRAMEWIN_LockH(FRAMEWIN_Handle h) {
  FRAMEWIN_Obj * p = (FRAMEWIN_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != FRAMEWIN_ID) {
      GUI_DEBUG_ERROROUT("FRAMEWIN.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       FRAMEWIN__GetBorderSize
*/
unsigned FRAMEWIN__GetBorderSize(FRAMEWIN_Handle hObj, unsigned Index) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  
  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  GUI_UNLOCK_H(pObj);
  ItemInfo.hWin = hObj;
  switch (Index) {
  case FRAMEWIN_GET_BORDERSIZE_T:
    ItemInfo.Cmd = WIDGET_ITEM_GET_BORDERSIZE_T;
    break;
  case FRAMEWIN_GET_BORDERSIZE_L:
    ItemInfo.Cmd = WIDGET_ITEM_GET_BORDERSIZE_L;
    break;
  case FRAMEWIN_GET_BORDERSIZE_B:
    ItemInfo.Cmd = WIDGET_ITEM_GET_BORDERSIZE_B;
    break;
  case FRAMEWIN_GET_BORDERSIZE_R:
    ItemInfo.Cmd = WIDGET_ITEM_GET_BORDERSIZE_R;
    break;
  }
  return pSkinPrivate->pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*       FRAMEWIN__CalcTitleHeight
*/
int FRAMEWIN__CalcTitleHeight(FRAMEWIN_Obj * pObj) {
  int r = 0;
  if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
    r = pObj->Props.TitleHeight;
    if (r == 0) {
      r = 2 + GUI_GetYSizeOfFont(pObj->Props.pFont);
    }
  }
  return r;
}

/*********************************************************************
*
*       FRAMEWIN__GetTitleLimits
*/
void FRAMEWIN__GetTitleLimits(FRAMEWIN_Handle hObj, int * pxMin, int * pxMax) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  WM_HWIN hChild;
  WM_Obj * pChild;
  int x0, x1, y0, BorderSizeT;
  GUI_RECT Rect;

  //
  // Get properties
  //
  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  Rect = pObj->Widget.Win.Rect;
  GUI_UNLOCK_H(pObj);
  BorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
  //
  // Iterate over child windows
  //
  hChild = WM_GetFirstChild(hObj);
  while (hChild) {
    pChild = (WM_Obj *)GUI_LOCK_H(hChild);
    x0 = pChild->Rect.x0 - Rect.x0;
    x1 = pChild->Rect.x1 - Rect.x0;
    y0 = pChild->Rect.y0 - Rect.y0;
    if (y0 == BorderSizeT) {
      if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
        if (x0 <= *pxMax) {
          *pxMax = x0 - 1;
        }
      } else {
        if (x1 >= *pxMin) {
          *pxMin = x1 + 1;
        }
      }
    }
    GUI_UNLOCK_H(pChild);
    hChild = WM_GetNextSibling(hChild);
  }
}

/*********************************************************************
*
*       FRAMEWIN__CalcPositions
*/
void FRAMEWIN__CalcPositions(FRAMEWIN_Handle hObj, FRAMEWIN_POSITIONS * pPos) {
  FRAMEWIN_Obj * pObj;
  int TitleHeight;
  int MenuHeight = 0;
  int IBorderSize = 0;
  int BorderSizeL;
  int BorderSizeR;
  int BorderSizeB;
  int BorderSizeT;
  int xsize, ysize;
  int x0, x1/*, y0*/;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;

  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  BorderSizeL = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_L);
  BorderSizeR = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_R);
  BorderSizeB = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_B);
  BorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
  xsize = WM__GetWindowSizeX(&pObj->Widget.Win);
  ysize = WM__GetWindowSizeY(&pObj->Widget.Win);
  if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
    IBorderSize = pObj->Props.IBorderSize;
  }
  TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
  if (pObj->hMenu) {
    MenuHeight = WM_GetWindowSizeY(pObj->hMenu);
  }
  pPos->TitleHeight = TitleHeight;
  pPos->MenuHeight  = MenuHeight;
  //
  // Set object properties accordingly
  //
  pPos->rClient.x0  =         BorderSizeL;
  pPos->rClient.x1  = xsize - BorderSizeR - 1;
  pPos->rClient.y0  =         BorderSizeT + IBorderSize + TitleHeight + MenuHeight;
  pPos->rClient.y1  = ysize - BorderSizeB - 1;
  //
  // Calculate title rect
  //
  pPos->rTitleText.x0 =         BorderSizeL;
  pPos->rTitleText.x1 = xsize - BorderSizeR - 1;
  pPos->rTitleText.y0 =         BorderSizeT;
  pPos->rTitleText.y1 =         BorderSizeT + TitleHeight - 1;
  #if 1
    GUI_UNLOCK_H(pObj);
    x0 = pPos->rTitleText.x0;
    x1 = pPos->rTitleText.x1;
    FRAMEWIN__GetTitleLimits(hObj, &x0, &x1);
    pPos->rTitleText.x0 = x0;
    pPos->rTitleText.x1 = x1;
  #else
  {
    WM_HWIN hChild;
    WM_HWIN hNext;
    WM_Obj * pChild;
    int y0;
    //
    // Iterate over all children
    //
    for (hChild = pObj->Widget.Win.hFirstChild; hChild; hChild = hNext) {
      pChild = (WM_Obj *)GUI_LOCK_H(hChild);
      x0 = pChild->Rect.x0 - pObj->Widget.Win.Rect.x0;
      x1 = pChild->Rect.x1 - pObj->Widget.Win.Rect.x0;
      y0 = pChild->Rect.y0 - pObj->Widget.Win.Rect.y0;
      if (y0 == BorderSizeT) {
        if (pChild->Status & WM_SF_ANCHOR_RIGHT) {
          if (x0 <= pPos->rTitleText.x1) {
            pPos->rTitleText.x1 = x0 - 1;
          }
        } else {
          if (x1 >= pPos->rTitleText.x0) {
            pPos->rTitleText.x0 = x1 + 1;
          }
        }
      }
      hNext = pChild->hNext;
      GUI_UNLOCK_H(pChild);
    }
    GUI_UNLOCK_H(pObj);
  }
  #endif
}

/*********************************************************************
*
*       FRAMEWIN__UpdatePositions
*/
void FRAMEWIN__UpdatePositions(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_POSITIONS Pos;
  WM_HWIN hClient;
  WM_HWIN hMenu;
  pObj = FRAMEWIN_LOCK_H(hObj);
  hClient = pObj->hClient;
  hMenu = pObj->hMenu;
  GUI_UNLOCK_H(pObj);
  //
  // Move client window accordingly
  //
  if (hClient || hMenu) {
    FRAMEWIN__CalcPositions(hObj, &Pos);
    if (hClient) {
      WM_MoveChildTo(hClient, Pos.rClient.x0, Pos.rClient.y0);
      WM_SetSize(hClient, 
                 Pos.rClient.x1 - Pos.rClient.x0 + 1, 
                 Pos.rClient.y1 - Pos.rClient.y0 + 1);
    }
    if (hMenu) {
      WM_MoveChildTo(hMenu, Pos.rClient.x0, Pos.rClient.y0 - Pos.MenuHeight);
    }
  }
}

/*********************************************************************
*
*       _OnPidStateChange
*/
static void _OnPidStateChange(FRAMEWIN_Handle hObj, WM_MESSAGE * pMsg) {
  FRAMEWIN_Obj * pObj;
  const WM_PID_STATE_CHANGED_INFO * pState = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  pObj = FRAMEWIN_LOCK_H(hObj);
  if ((pState->StatePrev == 0) && (pState->State == 1)) {
    pObj->Flags |= FRAMEWIN_SF_DRAGGING;
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       Exported API routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       Framewin Callback
*/
void FRAMEWIN_Callback (WM_MESSAGE * pMsg) {
  FRAMEWIN_Handle hObj;
  FRAMEWIN_Obj * pObj;
  GUI_RECT * pRect;
  FRAMEWIN_POSITIONS Pos;
  GUI_HOOK * pHook;
  WM_HWIN hFocussedChild;
  WM_HWIN hClient;
  U16 Flags;
  //void (* pfPaint)(FRAMEWIN_Handle hObj);
  WIDGET_PAINT * pfPaint;

  hObj = (FRAMEWIN_Handle)(pMsg->hWin);
  pObj = (FRAMEWIN_Obj *)GUI_LOCK_H(hObj); // Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point
  pHook = pObj->pFirstHook;
  hFocussedChild = pObj->hFocussedChild;
  hClient = pObj->hClient;
  Flags = pObj->Flags;
  if (pObj->pWidgetSkin) {
    pfPaint = pObj->pWidgetSkin->pfPaint;
  }
  GUI_UNLOCK_H(pObj);
  
  //
  // Call hook functions
  //
  for (; pHook; pHook = pHook->pNext) {
    int r;
    r = (*pHook->pHookFunc)(pMsg);
    if (r) {
      return;   // Message handled
    }
  }
  switch (pMsg->MsgId) {
  case WM_PID_STATE_CHANGED:
    _OnPidStateChange(hObj, pMsg);
    break;
  case WM_HANDLE_DIALOG_STATUS:
    pObj = (FRAMEWIN_Obj *)GUI_LOCK_H(hObj);
    if (pMsg->Data.p) {                           // set pointer to Dialog status
      pObj->pDialogStatus = (WM_DIALOG_STATUS *)pMsg->Data.p;
    } else {                                      // return pointer to Dialog status
      pMsg->Data.p = pObj->pDialogStatus;      
    }
    GUI_UNLOCK_H(pObj);
    return;
  case WM_PAINT:
    pfPaint(hObj);
    //_Paint(hObj);
    break;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    return;                       // Return here ... Message handled
  case WM_GET_INSIDE_RECT:
    FRAMEWIN__CalcPositions(hObj, &Pos);
    pRect = (GUI_RECT *)(pMsg->Data.p);
		*pRect = Pos.rClient;
    return;                       // Return here ... Message handled
  case WM_GET_CLIENT_WINDOW:      // return handle to client window. For most windows, there is no seperate client window, so it is the same handle
    pMsg->Data.v = (int)hClient;
    return;                       // Return here ... Message handled
  case WM_NOTIFY_PARENT:
    if (pMsg->Data.v == WM_NOTIFICATION_RELEASED) {
      WM_MESSAGE Msg;
      Msg.hWinSrc = hObj;
      Msg.Data    = pMsg->Data;
      Msg.MsgId   = WM_NOTIFY_PARENT_REFLECTION;
      WM__SendMessage(pMsg->hWinSrc, &Msg);
    }
    return;
  case WM_SET_FOCUS:                 // We have received or lost focus
    if (pMsg->Data.v == 1) {
      if (WM_IsWindow(hFocussedChild)) {
        WM_SetFocus(hFocussedChild);
      } else {
        hFocussedChild = WM_SetFocusOnNextChild(hClient);
        pObj = FRAMEWIN_LOCK_H(hObj);
        pObj->hFocussedChild = hFocussedChild;
        GUI_UNLOCK_H(pObj);
      }
      FRAMEWIN_SetActive(hObj, 1);
      pMsg->Data.v = 0;              // Focus could be accepted
    } else {
      FRAMEWIN_SetActive(hObj, 0);
    }
    return;
  case WM_TOUCH_CHILD:
    //
    // If a child of this framewindow has been touched and the frame window was not active,
    // the framewindow will receive the focus.
    //
    if (!(Flags & FRAMEWIN_SF_ACTIVE)) {
      const WM_MESSAGE * pMsgOrg;
      const GUI_PID_STATE * pState;
      pMsgOrg = (const WM_MESSAGE *)pMsg->Data.p;      // The original touch message
      pState = (const GUI_PID_STATE *)pMsgOrg->Data.p;
      if (pState) {          // Message may not have a valid pointer (moved out) !
        if (pState->Pressed) {
          WM_SetFocus(hObj);
        }
      }
    }
    break;
  case WM_NOTIFY_CHILD_HAS_FOCUS:
    _OnChildHasFocus(hObj, pMsg);
    break;
  case WM_DELETE:
    GUI_DEBUG_LOG("FRAMEWIN: FRAMEWIN_Callback(WM_DELETE)\n");
    pObj = FRAMEWIN_LOCK_H(hObj);
    GUI_ALLOC_FreePtr(&pObj->hText);
    GUI_UNLOCK_H(pObj);
    break;
  }
  //
  // Let widget handle the standard messages
  //
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported API routines:  Create
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_CreateEx
*/
FRAMEWIN_Handle FRAMEWIN_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                  int WinFlags, int ExFlags, int Id, const char * pTitle, WM_CALLBACK* cb)
{
  FRAMEWIN_Handle hObj;
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_POSITIONS Pos;
  WM_HWIN hClient;

  //
  // Create the window
  //
  GUI_LOCK();
  WinFlags |= WM_CF_LATE_CLIP;    // Always use late clipping since widget is optimized for it.
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, FRAMEWIN_Callback,
                                sizeof(FRAMEWIN_Obj) - sizeof(WM_Obj));
  if (hObj) {
    pObj = (FRAMEWIN_Obj *)GUI_LOCK_H(hObj); // Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point
    //
    // Init widget specific variables
    //
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE | FRAMEWIN_SF_TITLEVIS);
    //
    // Init member variables
    //
    FRAMEWIN_INIT_ID(pObj);
    pObj->Props = FRAMEWIN__DefaultProps;
    pObj->cb             = cb;
    pObj->Flags          = ExFlags;
    pObj->hFocussedChild = 0;
    pObj->hMenu          = 0;
    pObj->pFirstHook     = NULL;
    pObj->pWidgetSkin = FRAMEWIN__pSkinDefault;
    GUI_UNLOCK_H(pObj);
    FRAMEWIN__CalcPositions(hObj, &Pos);
    hClient = WM_CreateWindowAsChild(Pos.rClient.x0,Pos.rClient.y0,
                                     Pos.rClient.x1 - Pos.rClient.x0 +1,
                                     Pos.rClient.y1 - Pos.rClient.y0 +1,
                                     hObj, 
                                     WM_CF_ANCHOR_RIGHT | WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_TOP | WM_CF_ANCHOR_BOTTOM | WM_CF_SHOW | WM_CF_LATE_CLIP, 
                                     FRAMEWIN__cbClient, 0);
    FRAMEWIN__pSkinDefault->pfCreate(hObj);
    pObj = FRAMEWIN_LOCK_H(hObj);
    pObj->hClient = hClient;
    GUI_UNLOCK_H(pObj);
    //
    // Normally we disable memory devices for the frame window:
    // The frame window does not flicker, and not using memory devices is usually faster.
    // You can still use memory by explicitly specifying the flag
    //
    if ((WinFlags & (WM_CF_MEMDEV | (WM_CF_MEMDEV_ON_REDRAW))) == 0) {
      WM_DisableMemdev(hObj);
    }
    FRAMEWIN_SetText(hObj, pTitle);
  }
  GUI_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines:  Set Properties
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_SetText
*/
void FRAMEWIN_SetText(FRAMEWIN_Handle hObj, const char * s) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    WM_HWIN hText;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    hText = pObj->hText;
    GUI_UNLOCK_H(pObj);
    if (GUI__SetText(&hText, s)) {
      pObj = FRAMEWIN_LOCK_H(hObj);
      pObj->hText = hText;
      GUI_UNLOCK_H(pObj);
      FRAMEWIN_Invalidate(hObj);
    }
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetTextAlign
*/
void FRAMEWIN_SetTextAlign(FRAMEWIN_Handle hObj, int Align) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (pObj->Props.TextAlign != Align) {
      pObj->Props.TextAlign = Align;
      FRAMEWIN_Invalidate(hObj);
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetMoveable
*/
void FRAMEWIN_SetMoveable(FRAMEWIN_Handle hObj, int State) {
  if (hObj) {
    FRAMEWIN_Obj * pObj;
    GUI_LOCK();
    pObj = FRAMEWIN_LOCK_H(hObj);
    if (State) {
      pObj->Flags |= FRAMEWIN_CF_MOVEABLE;
    } else {
      pObj->Flags &= ~FRAMEWIN_CF_MOVEABLE;
    }
    GUI_UNLOCK_H(pObj);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       FRAMEWIN_SetActive
*/
void FRAMEWIN_SetActive(FRAMEWIN_Handle hObj, int State) {
  if (hObj) {
    WM_LOCK();
    _SetActive(hObj, State);
    WM_UNLOCK();
  }
}

#else
  void WIDGET_FrameWin(void) {} /* Avoid empty object files */
#endif /* GUI_WINSUPPORT */
