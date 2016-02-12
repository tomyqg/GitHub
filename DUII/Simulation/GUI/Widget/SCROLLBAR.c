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
File        : SCROLLBAR.c
Purpose     : Implementation of scrollbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "GUI_Private.h"
#include "SCROLLBAR_Private.h"
#include "WIDGET.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defaults
*
**********************************************************************
*/
#ifndef   SCROLLBAR_DRAW_SKIN_DEFAULT
  #define SCROLLBAR_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Module internal data
*
**********************************************************************
*/
SCROLLBAR_PROPS SCROLLBAR__DefaultProps = {
  {
    SCROLLBAR_COLOR_THUMB_DEFAULT,
    SCROLLBAR_COLOR_SHAFT_DEFAULT,
    SCROLLBAR_COLOR_ARROW_DEFAULT,
  },
  {
    SCROLLBAR_DRAW_SKIN_DEFAULT,
  }
};

I16 SCROLLBAR__DefaultWidth = SCROLLBAR_DEFAULT_WIDTH;

WIDGET_SKIN const * SCROLLBAR__pSkinDefault = &SCROLLBAR__SkinClassic;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _ScrollbarPressed
*/
static void _ScrollbarPressed(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;
  U16 Status;

  pObj = SCROLLBAR_LOCK_H(hObj);
  Status = pObj->Widget.Win.Status;
  GUI_UNLOCK_H(pObj);
  WIDGET_OrState(hObj, SCROLLBAR_STATE_PRESSED);
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
  }
}

/*********************************************************************
*
*       _DeleteTimer
*/
static void _DeleteTimer(SCROLLBAR_Handle hObj) {
  #ifdef GUI_X_CREATE_TIMER
    GUI_USE_PARA(hObj);
  #else
    SCROLLBAR_OBJ * pObj;
    pObj = SCROLLBAR_LOCK_H(hObj);
    WM_DeleteTimer(pObj->hTimer);
    pObj->hTimer = 0;
    pObj->TimerStep = 0;
    GUI_UNLOCK_H(pObj);
  #endif
}

/*********************************************************************
*
*       _ScrollbarReleased
*/
static void _ScrollbarReleased(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;
  U16 Status;

  WIDGET_AndState(hObj, SCROLLBAR_STATE_PRESSED);
  pObj = SCROLLBAR_LOCK_H(hObj);
  if (pObj->TimerStep) {
    _DeleteTimer(hObj);
  }
  Status = pObj->Widget.Win.Status;
  pObj->State = PRESSED_STATE_NONE;
  GUI_UNLOCK_H(pObj);
  if (Status & WM_SF_ISVIS) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
  }
}

/*********************************************************************
*
*       _RestartTimer
*/
static void _RestartTimer(SCROLLBAR_OBJ * pObj, int Period) {
  #ifdef GUI_X_CREATE_TIMER
    GUI_USE_PARA(pObj);
    GUI_USE_PARA(Period);
  #else
    WM_RestartTimer(pObj->hTimer, Period);
  #endif
}

/*********************************************************************
*
*       _CreateTimer
*/
static void _CreateTimer(SCROLLBAR_Handle hObj, int Dir, int Period) {
  SCROLLBAR_OBJ * pObj;
  WM_HMEM hTimer;

  hTimer = WM_CreateTimer(hObj, SCROLLBAR_TIMER_ID, Period, 0);
  pObj = SCROLLBAR_LOCK_H(hObj);
  pObj->hTimer    = hTimer;
  pObj->TimerStep = Dir;
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(SCROLLBAR_Handle hObj, WM_MESSAGE * pMsg) {
  SCROLLBAR_OBJ * pObj;
  SCROLLBAR_POSITIONS Pos;
  GUI_PID_STATE * pState;
  int Sel, Range, x, TimerStep, TimerStepNew, temp;
  U16 State;

  if (pMsg->Data.p) {  // Something happened in our area (pressed or released)
    pState = (GUI_PID_STATE *)pMsg->Data.p;
    if (pState->Pressed) {
      pObj = SCROLLBAR_LOCK_H(hObj);
      Sel = pObj->v;
      pObj->pfCalcPositions(hObj, &Pos);
      Range = pObj->NumItems - pObj->PageSize;
      //
      // Swap mouse coordinates if necessary
      //
      if (pObj->Widget.State & WIDGET_STATE_VERTICAL) {
        temp = pState->x;
        pState->x = pState->y;
        pState->y = temp;
      }
      x = pState->x;
      TimerStepNew = 0;
      State = pObj->Widget.State;
      if (pObj->State == PRESSED_STATE_THUMB) {
        //
        // No timer used for moving with thumb
        //
        _DeleteTimer(hObj);
        //
        // Calculate new position
        //
        x = x - Pos.ThumbSize / 2 - Pos.x1_LeftArrow - 1;
        Sel = pObj->TouchPos = (int)GUI__DivideRound32((I32)Range * x, Pos.xSizeMoveable);
        GUI_UNLOCK_H(pObj);
      } else {
        pObj->State = PRESSED_STATE_NONE;
        if (x <= Pos.x1_LeftArrow) {         // left arrow (line left)
          Sel--;
          TimerStepNew = -1;
          pObj->State = PRESSED_STATE_LEFT;
        } else if (x < Pos.x0_Thumb) {       // left area  (page left)
          Sel -= pObj->PageSize;
          TimerStepNew = -pObj->PageSize;
        } else if (x <= Pos.x1_Thumb) {      // Thumb area
          if (Pos.xSizeMoveable > 0) {
            x = x - Pos.ThumbSize / 2 - Pos.x1_LeftArrow - 1;
            Sel = (int)GUI__DivideRound32((I32)Range * x, Pos.xSizeMoveable);
          }
          pObj->State = PRESSED_STATE_THUMB;
        } else if (x < Pos.x0_RightArrow) {  // right area (page right)
          Sel += pObj->PageSize;
          TimerStepNew = +pObj->PageSize;
        } else  if (x <= Pos.x1_RightArrow) { // right arrow (line right)
          Sel++;
          TimerStepNew = +1;
          pObj->State = PRESSED_STATE_RIGHT;
        }
        TimerStep = pObj->TimerStep;
        //
        // Remember position
        //
        x = x - Pos.ThumbSize / 2 - Pos.x1_LeftArrow - 1;
        pObj->TouchPos = (int)GUI__DivideRound32((I32)Range * x, Pos.xSizeMoveable);
        GUI_UNLOCK_H(pObj);
        if (TimerStepNew) {
          if (!TimerStep) {
            _CreateTimer(hObj, TimerStepNew, 400);
          }
        } else {
          if (TimerStep) {
            _DeleteTimer(hObj);
          }
        }
      }
      WM_SetCapture(hObj, 1);
      SCROLLBAR_SetValue(hObj, Sel);
      if ((State & SCROLLBAR_STATE_PRESSED) == 0){   
        _ScrollbarPressed(hObj);
      }
    } else {
      //
      // React only if button was pressed before ... avoid problems with moving / hiding windows above (such as dropdown)
      //
      pObj = SCROLLBAR_LOCK_H(hObj);
      State = pObj->Widget.State;
      GUI_UNLOCK_H(pObj);
      if (State & SCROLLBAR_STATE_PRESSED) {   
        _ScrollbarReleased(hObj);
      }
    }
  }
}

/*********************************************************************
*
*       _OnKey
*/
static void  _OnKey(SCROLLBAR_Handle hObj, WM_MESSAGE * pMsg) {
  SCROLLBAR_OBJ * pObj;
  const WM_KEY_INFO * pKeyInfo;
  int Key, PageSize;

  pObj = SCROLLBAR_LOCK_H(hObj); {
    PageSize = pObj->PageSize;
  } GUI_UNLOCK_H(pObj);
  pKeyInfo = (const WM_KEY_INFO *)(pMsg->Data.p);
  Key = pKeyInfo->Key;
  if (pKeyInfo->PressedCnt > 0) {
    switch (Key) {
      case GUI_KEY_PGUP:
        SCROLLBAR_AddValue(hObj,  PageSize);
        break;
      case GUI_KEY_PGDOWN:
        SCROLLBAR_AddValue(hObj,  -PageSize);
        break;
      case GUI_KEY_RIGHT:
      case GUI_KEY_DOWN:
        SCROLLBAR_Inc(hObj);
        break;                    /* Send to parent by not doing anything */
      case GUI_KEY_LEFT:
      case GUI_KEY_UP:
        SCROLLBAR_Dec(hObj);
        break;                    /* Send to parent by not doing anything */
      default:
        return;
    }
  }
}

/*********************************************************************
*
*       _OnSetScrollState
*/
static void _OnSetScrollState(SCROLLBAR_Handle hObj, const WM_SCROLL_STATE* pState) {
  SCROLLBAR_OBJ * pObj;
  
  pObj = SCROLLBAR_LOCK_H(hObj);
  if (  (pState->NumItems != pObj->NumItems)
      || (pObj->PageSize   != pState->PageSize)
      || (pObj->v          != pState->v))
  {
    pObj->NumItems = pState->NumItems;
    pObj->PageSize = pState->PageSize;
    pObj->v        = pState->v;
    WM_InvalidateWindow(hObj);
    GUI_UNLOCK_H(pObj);
    WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _OnTimer
*/
static void _OnTimer(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;
  int Delete;

  pObj = SCROLLBAR_LOCK_H(hObj);
  if (pObj->TimerStep) {
    SCROLLBAR_AddValue(hObj, pObj->TimerStep);
  }
  //
  // Check if timer needs to be stopped
  //
  if (pObj->TimerStep > 0) {
    Delete = (pObj->v >= pObj->TouchPos) ? 1 : 0;
    if (pObj->v > pObj->TouchPos) {
      pObj->v = pObj->TouchPos;
    }
  } else {
    Delete = (pObj->v <= pObj->TouchPos) ? 1 : 0;
    if (pObj->v < pObj->TouchPos) {
      pObj->v = pObj->TouchPos;
    }
  }
  //
  // Delete or restart timer
  //
  if (Delete) {
    _DeleteTimer(hObj);
  } else {
    _RestartTimer(pObj, 50);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR__Rect2VRect
*
* Purpose:
*   Convert rectangle in real coordinates into virtual coordinates
*
* Add. info:
*   This function could eventually be made none-static and move into
*   a module of its own.
*/
void SCROLLBAR__Rect2VRect(const WIDGET * pWidget, GUI_RECT * pRect) {
  int xSize, x0, x1;

  if (pWidget->State & WIDGET_STATE_VERTICAL) {
    xSize = pWidget->Win.Rect.x1 - pWidget->Win.Rect.x0 + 1;
    x0 = pRect->x0;
    x1 = pRect->x1;
    pRect->x0 = pRect->y0;
    pRect->x1 = pRect->y1;
    pRect->y1 = xSize - 1 - x0;
    pRect->y0 = xSize - 1 - x1;
  }
}

/*********************************************************************
*
*       SCROLLBAR_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
SCROLLBAR_OBJ * SCROLLBAR_LockH(SCROLLBAR_Handle h) {
  SCROLLBAR_OBJ * p = (SCROLLBAR_OBJ *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != SCROLLBAR_ID) {
      GUI_DEBUG_ERROROUT("SCROLLBAR.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       SCROLLBAR__InvalidatePartner
*/
void SCROLLBAR__InvalidatePartner(SCROLLBAR_Handle hObj) {     /* Invalidate the partner, since it is also affected */
  WM_InvalidateWindow(WM_GetScrollPartner(hObj));
  WM_SendMessageNoPara(WM_GetParent(hObj), WM_NOTIFY_CLIENTCHANGE);   /* Client area may have changed */
}

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_Callback
*/
void SCROLLBAR_Callback (WM_MESSAGE *pMsg) {
  SCROLLBAR_Handle hObj;
  SCROLLBAR_OBJ * pObj;
  WIDGET_PAINT * pfPaint;

  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_DELETE:
    SCROLLBAR__InvalidatePartner(hObj);
    break;
  case WM_PAINT:
    GUI_DEBUG_LOG("SCROLLBAR: _Callback(WM_PAINT)\n");
    pObj = SCROLLBAR_LOCK_H(hObj);
    pfPaint = pObj->pWidgetSkin->pfPaint;
    GUI_UNLOCK_H(pObj);
    pfPaint(hObj);
    //_Paint(hObj);
    return;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_KEY:
    _OnKey(hObj, pMsg);
    break;
  case WM_SET_SCROLL_STATE:
    _OnSetScrollState(hObj, (const WM_SCROLL_STATE*)pMsg->Data.p);
    break;
  case WM_GET_SCROLL_STATE:
    pObj = SCROLLBAR_LOCK_H(hObj);
    ((WM_SCROLL_STATE *)pMsg->Data.p)->NumItems = pObj->NumItems;
    ((WM_SCROLL_STATE *)pMsg->Data.p)->PageSize = pObj->PageSize;
    ((WM_SCROLL_STATE *)pMsg->Data.p)->v        = pObj->v;
    GUI_UNLOCK_H(pObj);
    break;
  case WM_TIMER:
    _OnTimer(hObj);
    break;
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Exported routines:  Create
*
**********************************************************************
*/

/* Note: the parameters to a create function may vary.
         Some widgets may have multiple create functions */

/*********************************************************************
*
*       SCROLLBAR_CreateEx
*/
SCROLLBAR_Handle SCROLLBAR_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                    int WinFlags, int ExFlags, int Id)
{
  SCROLLBAR_Handle hObj;
  SCROLLBAR_OBJ * pObj;
  GUI_RECT Rect;
  U16 InitState;

  WM_LOCK();
  /* Set defaults if necessary */
  if ((xsize == 0) && (ysize == 0)) {
    WM_GetInsideRectEx(hParent, &Rect);
    if (ExFlags & SCROLLBAR_CF_VERTICAL) {
      xsize = SCROLLBAR__DefaultWidth;
      x0    = Rect.x1 + 1 - xsize;
      y0    = Rect.y0;
      ysize = Rect.y1 - Rect.y0 + 1;
    } else {
      ysize = SCROLLBAR__DefaultWidth;
      y0    = Rect.y1 + 1 - ysize;
      x0    = Rect.x0;
      xsize = Rect.x1 - Rect.x0 + 1;
    }
  }
  /* Create the window */
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, SCROLLBAR_Callback,
                                sizeof(SCROLLBAR_OBJ) - sizeof(WM_Obj));
  if (hObj) {
    pObj = (SCROLLBAR_OBJ *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* Handle SpecialFlags */
    InitState = 0;
    if (ExFlags & SCROLLBAR_CF_VERTICAL) {
      InitState |= WIDGET_CF_VERTICAL;
    }
    if (ExFlags & SCROLLBAR_CF_FOCUSSABLE) {
      InitState |= WIDGET_STATE_FOCUSSABLE;
    }
    if ((Id != GUI_ID_HSCROLL) && (Id != GUI_ID_VSCROLL)) {
      InitState |= WIDGET_STATE_FOCUSSABLE;
    }
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, InitState);
    /* init member variables */
    SCROLLBAR_INIT_ID(pObj);
    pObj->Props         = SCROLLBAR__DefaultProps;
    pObj->NumItems      = 100;
    pObj->PageSize      =  10;
    pObj->v             =   0;
    pObj->pWidgetSkin = SCROLLBAR__pSkinDefault;
    GUI_UNLOCK_H(pObj);
    SCROLLBAR__pSkinDefault->pfCreate(hObj);
    SCROLLBAR__InvalidatePartner(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "SCROLLBAR_Create failed")
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines:  Various methods
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_Dec
*/
void SCROLLBAR_Dec(SCROLLBAR_Handle hObj) {
  SCROLLBAR_AddValue(hObj, -1);
}

/*********************************************************************
*
*       SCROLLBAR_Inc
*/
void SCROLLBAR_Inc(SCROLLBAR_Handle hObj) {
  SCROLLBAR_AddValue(hObj,  1);
}

/*********************************************************************
*
*       SCROLLBAR_AddValue
*/
void SCROLLBAR_AddValue(SCROLLBAR_Handle hObj, int Add) {
  SCROLLBAR_OBJ * pObj;
  int v;

  if (hObj) {
    WM_LOCK();
    pObj = SCROLLBAR_LOCK_H(hObj);
    v = pObj->v;
    GUI_UNLOCK_H(pObj);
    SCROLLBAR_SetValue(hObj, v + Add);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SCROLLBAR_SetValue
*/
void SCROLLBAR_SetValue(SCROLLBAR_Handle hObj, int v) {
  SCROLLBAR_OBJ * pObj;
  int Max;
  if (hObj) {
    WM_LOCK();
    pObj = SCROLLBAR_LOCK_H(hObj);
    Max = pObj->NumItems - pObj->PageSize;
    if (Max < 0) {
      Max = 0;
    }
    /* Put in min/max range */
    if (v < 0) {
      v = 0;
    }
    if (v > Max) {
      v = Max;
    }
    if (pObj->v != v) {
      pObj->v = v;
      WM_InvalidateWindow(hObj);
      GUI_UNLOCK_H(pObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_VALUE_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SCROLLBAR_SetNumItems
*/
void SCROLLBAR_SetNumItems(SCROLLBAR_Handle hObj, int NumItems) {
  SCROLLBAR_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = SCROLLBAR_LOCK_H(hObj);
    if (pObj->NumItems != NumItems) {
      pObj->NumItems = NumItems;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SCROLLBAR_SetPageSize
*/
void SCROLLBAR_SetPageSize(SCROLLBAR_Handle hObj, int PageSize) {
  SCROLLBAR_OBJ * pObj;

  if (hObj) {
    WM_LOCK();
    pObj = SCROLLBAR_LOCK_H(hObj);
    if (pObj->PageSize != PageSize) {
      pObj->PageSize = PageSize;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       SCROLLBAR_SetState
*/
void  SCROLLBAR_SetState   (SCROLLBAR_Handle hObj, const WM_SCROLL_STATE* pState) {
  if (hObj) {
    SCROLLBAR_SetPageSize(hObj, pState->PageSize);
    SCROLLBAR_SetNumItems(hObj, pState->NumItems);
    SCROLLBAR_SetValue   (hObj, pState->v);
  }
}

#else /* avoid empty object files */

void SCROLLBAR_C(void);
void SCROLLBAR_C(void){}

#endif  /* GUI_WINSUPPORT */
