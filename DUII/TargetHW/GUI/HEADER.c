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
File        : HEADER.c
Purpose     : Implementation of header widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "HEADER_Private.h"
#include "SCROLLBAR.h"
#include "GUI.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

#ifndef HEADER_SUPPORT_DRAG
  #define HEADER_SUPPORT_DRAG 1
#endif

#ifndef HEADER_BORDER_V_DEFAULT
  #define HEADER_BORDER_V_DEFAULT 0
#endif

#ifndef HEADER_BORDER_H_DEFAULT
  #define HEADER_BORDER_H_DEFAULT 2
#endif

/* Define default fonts */
#ifndef HEADER_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define HEADER_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define HEADER_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define HEADER_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef HEADER_BKCOLOR_DEFAULT
  #define HEADER_BKCOLOR_DEFAULT 0xAAAAAA
#endif

#ifndef HEADER_TEXTCOLOR_DEFAULT
  #define HEADER_TEXTCOLOR_DEFAULT GUI_BLACK
#endif

#ifndef HEADER_ARROWCOLOR_DEFAULT
  #define HEADER_ARROWCOLOR_DEFAULT GUI_BLACK
#endif

/* Define cursors */
#ifndef HEADER_CURSOR_DEFAULT
  #define HEADER_CURSOR_DEFAULT &GUI_CursorHeaderM
#endif

#ifndef   HEADER_DRAW_SKIN_DEFAULT
  #define HEADER_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

HEADER_PROPS HEADER__DefaultProps = {
  HEADER_FONT_DEFAULT,
  HEADER_BKCOLOR_DEFAULT,
  HEADER_TEXTCOLOR_DEFAULT,
  HEADER_ARROWCOLOR_DEFAULT,
  {
    HEADER_DRAW_SKIN_DEFAULT
  }
};

/* Remember the old cursor */
static const GUI_CURSOR GUI_UNI_PTR * _pOldCursor;

/* Default values */
const GUI_CURSOR GUI_UNI_PTR * HEADER__pDefaultCursor   = HEADER_CURSOR_DEFAULT;
int                            HEADER__DefaultBorderH   = HEADER_BORDER_H_DEFAULT;
int                            HEADER__DefaultBorderV   = HEADER_BORDER_V_DEFAULT;

WIDGET_SKIN const * HEADER__pSkinDefault = &HEADER__SkinClassic;

/*********************************************************************
*
*       static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _Paint
*/
static void _Paint(HEADER_Handle hObj) {
  HEADER_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = HEADER_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _RestoreOldCursor
*/
static void _RestoreOldCursor(void) {
  if (_pOldCursor) {
    #if GUI_SUPPORT_CURSOR
      if (GUI_CURSOR_GetStateEx(WM__TOUCHED_LAYER)) {
        GUI_CURSOR_SelectEx(_pOldCursor, WM__TOUCHED_LAYER);
      }
    #endif
    _pOldCursor = 0;
  }
}

/*********************************************************************
*
*       _FreeAttached
*
* Delete attached objects (if any)
*/
static void _FreeAttached(HEADER_Handle hObj) {
  int i, NumItems;
  HEADER_Obj * pObj;
  GUI_ARRAY Columns;
  pObj = HEADER_LOCK_H(hObj);
  Columns = pObj->Columns;
  GUI_UNLOCK_H(pObj);
  NumItems = GUI_ARRAY_GetNumItems(Columns);
  for (i = 0; i < NumItems; i++) {
    HEADER_COLUMN * pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, i);
    if (pColumn->hDrawObj) {
      GUI_ALLOC_Free(pColumn->hDrawObj);
    }
    GUI_UNLOCK_H(pColumn);
  }
  /* Delete attached objects (if any) */
  GUI_ARRAY_Delete(Columns);
  _RestoreOldCursor();
}

/*********************************************************************
*
*       _GetDividerIndex
*/
#if (HEADER_SUPPORT_DRAG)
static int _GetDividerIndex(HEADER_Handle hObj, HEADER_Obj * pObj, int x, int y) {
  int Item;
  int LastFixedPos;
  Item = -1;
  LastFixedPos = 0;
  if ((y >= 0) && (y < WM_GetWindowSizeY(hObj))) {
    if (hObj) {
      int xPos;
      unsigned Index, NumColumns;
      NumColumns = GUI_ARRAY_GetNumItems(pObj->Columns);
      if (pObj->Fixed) {
        xPos = pObj->ScrollPos;
      } else {
        xPos = 0;
      }
      for (Index = 0; Index < NumColumns; Index++) {
        HEADER_COLUMN * pColumn;
        pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index);
        xPos += pColumn->Width;
        GUI_UNLOCK_H(pColumn);
        if (Index < pObj->Fixed) {
          LastFixedPos = xPos;
        } else if (pObj->Fixed && (Index == pObj->Fixed)) {
          xPos -= pObj->ScrollPos;
        }
        if (xPos >= LastFixedPos) {
          if ((xPos >= (x - 4)) && (xPos <= (x + 4))) {
            Item = Index;
            if ((Index < (NumColumns - 1)) && (x < xPos)) {
              int Width;
              pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index + 1);
              Width = pColumn->Width;
              GUI_UNLOCK_H(pColumn);
              if (Width == 0) {
                break;
              }
            }
          }
        }
      }
    }
  }
  return Item;
}
#endif

/*********************************************************************
*
*       _LimitDragWitdh
*
* Purpose:
*   Limits dragging of the captured header item to the right border of the window.
*
* Return value:
*   1 - If function has limitted the clipped header item
*   0 - If function has not limitted the clipped header item
*/
static int _LimitDragWitdh(HEADER_Handle hObj) {
  HEADER_Obj * pObj;
  int DragLimit, i, SumX;
  GUI_RECT Rect;
  WM_HWIN hVScroll, hParent;
  int CaptureItem;
  pObj = HEADER_LOCK_H(hObj);
  CaptureItem = pObj->CaptureItem;
  DragLimit = pObj->DragLimit;
  GUI_UNLOCK_H(pObj);
  if (DragLimit) {
    /* Take the x-size of the widgets client rectangle as limit */
    WM_GetClientRectEx(hObj, &Rect);
    DragLimit = Rect.x1;
    /* If the parent window has a vertical scrollbar, reduce the drag limit by the xsize of the scrollbar */
    hParent = WM_GetParent(hObj);
    if (hParent) {
      hVScroll = WM_GetScrollbarV(WM_GetParent(hObj));
      if (hVScroll) {
        DragLimit -= WM_GetWindowSizeX(hVScroll);
      }
    }
    /* Calculate the sum of the width of all header items */
    for (SumX = i = 0; i <= CaptureItem; i++) {
      SumX += HEADER_GetItemWidth(hObj, i);
    }
    /* If the sum of the width of all header items exeeds the limit, limit the captured item */
    if (SumX > DragLimit) {
      for (SumX = i = 0; i < CaptureItem; i++) {
        SumX += HEADER_GetItemWidth(hObj, i);
      }
      HEADER_SetItemWidth(hObj, CaptureItem, DragLimit - SumX);
      return 1;
    }
  }
  return 0;
}

/*********************************************************************
*
*       _HandlePID
*/
#if (HEADER_SUPPORT_DRAG)
static void _HandlePID(HEADER_Handle hObj, int x, int y, int Pressed) {
  HEADER_Obj * pObj;
  int Hit;
  int CapturePosX;
  int CaptureItem;

  pObj = HEADER_LOCK_H(hObj);
  Hit = _GetDividerIndex(hObj, pObj, x, y);
  /* set capture position */
  if ((Pressed == 1) && (Hit >= 0) && (pObj->CapturePosX == -1)) {
    pObj->CapturePosX = x;
    pObj->CaptureItem = Hit;
  }
  if (Pressed <= 0) {
    pObj->CapturePosX = -1;
  }
  CapturePosX = pObj->CapturePosX;
  CaptureItem = pObj->CaptureItem;
  GUI_UNLOCK_H(pObj);
  /* set mouse cursor and capture */
  if (Hit >= 0) {
    WM_SetCapture(hObj, 1);
    #if GUI_SUPPORT_CURSOR
      if (!_pOldCursor) {
        if (GUI_CURSOR_GetStateEx(WM__TOUCHED_LAYER)) {
          _pOldCursor = GUI_CURSOR_SelectEx(HEADER__pDefaultCursor, WM__TOUCHED_LAYER);
        }
      }
    #endif
  }
  /* modify header */
  if ((CapturePosX >= 0) && (x != CapturePosX) && (Pressed == 1)) {
    int NewSize = HEADER_GetItemWidth(hObj, CaptureItem) + x - CapturePosX;
    if (NewSize >= 0) {
      HEADER_SetItemWidth(hObj, CaptureItem, NewSize);
      if (!_LimitDragWitdh(hObj)) {
        CapturePosX = x;
      }
    }
  }
  /* release capture & restore cursor */
  if (Pressed <= 0) {
    #if (GUI_SUPPORT_MOUSE)
    if (Hit == -1)
    #endif
    {
      _RestoreOldCursor();
      CapturePosX = -1;
      WM_ReleaseCapture();
    }
  }
  pObj = HEADER_LOCK_H(hObj);
  pObj->CapturePosX = CapturePosX;
  GUI_UNLOCK_H(pObj);
}
#endif

/*********************************************************************
*
*       _OnMouseOver
*/
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
static void _OnMouseOver(HEADER_Handle hObj, WM_MESSAGE * pMsg) {
  HEADER_Obj * pObj;
  const GUI_PID_STATE * pState;
  int ScrollPos;
  pObj = HEADER_LOCK_H(hObj);
  ScrollPos = pObj->ScrollPos;
  GUI_UNLOCK_H(pObj);
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pState) {
    _HandlePID(hObj, pState->x + ScrollPos, pState->y, -1);
  }
}
#endif

/*********************************************************************
*
*       _GetItemIndex
*/
static int _GetItemIndex(HEADER_Handle hObj, HEADER_Obj * pObj, int x, int y) {
  int Item;
  int LastFixedPos;
  Item = -1;
  LastFixedPos = 0;
  if ((y >= 0) && (y < WM_GetWindowSizeY(hObj))) {
    if (hObj) {
      int xPos;
      unsigned Index, NumColumns;
      NumColumns = GUI_ARRAY_GetNumItems(pObj->Columns);
      if (pObj->Fixed) {
        xPos = pObj->ScrollPos;
      } else {
        xPos = 0;
      }
      for (Index = 0; Index < NumColumns; Index++) {
        int Width;
        HEADER_COLUMN * pColumn;
        pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index);
        Width = pColumn->Width;
        GUI_UNLOCK_H(pColumn);
        if (xPos >= LastFixedPos) {
          if ((x > (xPos + 4)) && (x < (xPos + Width - 4))) {
            Item = Index;
            break;
          }
        }
        if (Index < pObj->Fixed) {
          LastFixedPos = xPos;
        } else if (pObj->Fixed && (Index == pObj->Fixed)) {
          xPos -= pObj->ScrollPos;
        }
        xPos += Width;
      }
    }
  }
  return Item;
}

/*********************************************************************
*
*       _OnTouch
*/
#if (HEADER_SUPPORT_DRAG)
static void _OnTouch(HEADER_Handle hObj, WM_MESSAGE * pMsg) {
  HEADER_Obj * pObj;
  int Notification;
  int ScrollPos;
  const GUI_PID_STATE * pState;

  pObj = HEADER_LOCK_H(hObj);
  ScrollPos = pObj->ScrollPos;
  GUI_UNLOCK_H(pObj);
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pState) {
    _HandlePID(hObj, pState->x + ScrollPos, pState->y, pState->Pressed);
    if (pState->Pressed) {
      Notification = WM_NOTIFICATION_CLICKED;
    } else {
      Notification = WM_NOTIFICATION_RELEASED;
    }
  } else {
    Notification = WM_NOTIFICATION_MOVED_OUT;
  }
  WM_NotifyParent(hObj, Notification);
}
#endif

/*********************************************************************
*
*       _OnPidStateChange
*/
static void _OnPidStateChange(HEADER_Handle hObj, WM_MESSAGE * pMsg) {
  HEADER_Obj * pObj;
  const WM_PID_STATE_CHANGED_INFO * pState;
  pObj = HEADER_LOCK_H(hObj);
  pState = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  if ((pState->StatePrev == 1) && (pState->State == 0)) {
    pObj->Sel = _GetItemIndex(hObj, pObj, pState->x + pObj->ScrollPos, pState->y);
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
*       HEADER_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
HEADER_Obj * HEADER_LockH(HEADER_Handle h) {
  HEADER_Obj * p = (HEADER_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != HEADER_ID) {
      GUI_DEBUG_ERROROUT("HEADER.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_Callback
*/
void HEADER_Callback (WM_MESSAGE *pMsg) {
  HEADER_Handle hObj;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  WM_LOCK();
  switch (pMsg->MsgId) {
  case WM_PAINT:
    _Paint(hObj);
    break;
  case WM_PID_STATE_CHANGED:
    _OnPidStateChange(hObj, pMsg);
    break;
#if (HEADER_SUPPORT_DRAG)
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
#endif
#if (HEADER_SUPPORT_DRAG & GUI_SUPPORT_MOUSE)
  case WM_MOUSEOVER:
    _OnMouseOver(hObj, pMsg);
    break;
#endif
  case WM_DELETE:
    _FreeAttached(hObj); /* No return here ... WM_DefaultProc needs to be called */
  default:
    WM_DefaultProc(pMsg);
  }
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
*       HEADER_Create
*/
HEADER_Handle HEADER_Create(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int Id, int Flags, int ExFlags) {
  return HEADER_CreateEx(x0, y0, xsize, ysize, hParent, Flags, ExFlags, Id);
}

/*********************************************************************
*
*       HEADER_CreateEx
*/
HEADER_Handle HEADER_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                              int WinFlags, int ExFlags, int Id)
{
  HEADER_Handle hObj;
  GUI_USE_PARA(ExFlags);
  /* Create the window */
  WM_LOCK();
  if ((xsize == 0) && (x0 == 0) && (y0 == 0)) {
    GUI_RECT Rect;
    WM_GetInsideRectEx(hParent, &Rect);
    xsize = Rect.x1 - Rect.x0 + 1;
    x0    = Rect.x0;
    y0    = Rect.y0;
  }
  if (ysize == 0) {
    const WIDGET_EFFECT* pEffect = WIDGET_GetDefaultEffect();
    ysize  = GUI_GetYDistOfFont(HEADER__DefaultProps.pFont);
    ysize += 2 * HEADER__DefaultBorderV;
    ysize += 2 * (unsigned)pEffect->EffectSize;
  }
  WinFlags |= WM_CF_ANCHOR_LEFT | WM_CF_ANCHOR_RIGHT;
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, &HEADER_Callback,
                                sizeof(HEADER_Obj) - sizeof(WM_Obj));
  if (hObj) {
    HEADER_Obj * pObj;
    GUI_ARRAY Columns;
    Columns = GUI_ARRAY_Create();
    pObj = (HEADER_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* Init sub-classes */
    pObj->Columns = Columns;
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, 0);
    /* init member variables */
    HEADER_INIT_ID(pObj);
    pObj->Props       = HEADER__DefaultProps;
    pObj->CapturePosX = -1;
    pObj->CaptureItem = -1;
    pObj->ScrollPos   = 0;
    pObj->DirIndicatorColumn = -1;
    pObj->pWidgetSkin = HEADER__pSkinDefault;
    GUI_UNLOCK_H(pObj);
    HEADER__pSkinDefault->pfCreate(hObj);
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "HEADER_Create failed")
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Exported routines: Global functions
*
**********************************************************************
*/
/*********************************************************************
*
*       Exported routines: Member functions
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetFont
*/
void HEADER_SetFont(HEADER_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    pObj->Props.pFont = pFont;
    HEADER_SetHeight(hObj, GUI_GetYDistOfFont(pFont) + 2 * HEADER_BORDER_V_DEFAULT + 2 * pObj->Widget.pEffect->EffectSize);
    WM_InvalidateWindow(hObj);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetHeight
*/
void HEADER_SetHeight(HEADER_Handle hObj, int Height) {
  if (hObj) {
    GUI_RECT Rect;
    WM_GetClientRectEx(hObj, &Rect);
    WM_SetSize(hObj, Rect.x1 - Rect.x0 + 1, Height);
    WM_InvalidateWindow(WM_GetParent(hObj));
  }
}

/*********************************************************************
*
*       HEADER_SetTextAlign
*/
void HEADER_SetTextAlign(HEADER_Handle hObj, unsigned int Index, int Align) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (Index <= GUI_ARRAY_GetNumItems(pObj->Columns)) {
      HEADER_COLUMN * pColumn;
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index);
      pColumn->Align = Align;
      GUI_UNLOCK_H(pColumn);
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetScrollPos
*/
void HEADER_SetScrollPos(HEADER_Handle hObj, int ScrollPos) {
  if (hObj && (ScrollPos >= 0)) {
    HEADER_Obj* pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (ScrollPos != pObj->ScrollPos) {
      pObj->ScrollPos = ScrollPos;
      WM_Invalidate(hObj);
      WM_InvalidateWindow(WM_GetParent(hObj));
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_AddItem
*/
void HEADER_AddItem(HEADER_Handle hObj, int Width, const char * s, int Align) {
  if (hObj) {
    HEADER_Obj * pObj;
    HEADER_COLUMN Column;
    GUI_ARRAY Columns;
    int Index;
    int Len;

    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Columns = pObj->Columns;
    Len = s ? strlen(s) : 0;
    if (Width || Len) {
      if (!Width) {
        const GUI_FONT GUI_UNI_PTR * pFont;
        pFont = GUI_SetFont(pObj->Props.pFont);
        Width = GUI_GetStringDistX(s) + 2 * (pObj->Widget.pEffect->EffectSize + HEADER__DefaultBorderH);
        GUI_SetFont(pFont);
      }
      Column.Width    = Width;
      Column.Align    = Align;
      Column.hDrawObj = 0;
      GUI_UNLOCK_H(pObj);
      Index = GUI_ARRAY_GetNumItems(Columns);
      if (GUI_ARRAY_AddItem(Columns, &Column, sizeof(HEADER_COLUMN) + Len + 1) == 0) {
        HEADER_COLUMN * pColumn;
        pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, Index);
        if (Len) {
          strcpy(pColumn->acText, s);
        } else {
          pColumn->acText[0] = 0;
        }
        GUI_UNLOCK_H(pColumn);
        WM_InvalidateWindow(hObj);
        WM_InvalidateWindow(WM_GetParent(hObj));
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_DeleteItem
*/
void HEADER_DeleteItem(HEADER_Handle hObj, unsigned Index) {
  if (hObj) {
    HEADER_Obj* pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (Index < GUI_ARRAY_GetNumItems(pObj->Columns)) {
      GUI_ARRAY_DeleteItem(pObj->Columns, Index);
      WM_InvalidateWindow(hObj);
      WM_InvalidateWindow(WM_GetParent(hObj));
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetItemText
*/
void HEADER_SetItemText(HEADER_Handle hObj, unsigned int Index, const char* s) {
  if (hObj) {
    GUI_ARRAY Columns;
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Columns = pObj->Columns;
    GUI_UNLOCK_H(pObj);
    if (Index < GUI_ARRAY_GetNumItems(Columns)) {
      HEADER_COLUMN * pColumn;
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_ResizeItemLocked(Columns, Index, sizeof(HEADER_COLUMN) + strlen(s));
      if (pColumn) {
        strcpy(pColumn->acText, s);
        GUI_UNLOCK_H(pColumn);
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_SetItemWidth
*/
void HEADER_SetItemWidth(HEADER_Handle hObj, unsigned int Index, int Width) {
  if (hObj && (Width >= 0)) {
    GUI_ARRAY Columns;
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    Columns = pObj->Columns;
    GUI_UNLOCK_H(pObj);
    if (Index <= GUI_ARRAY_GetNumItems(Columns)) {
      HEADER_COLUMN * pColumn;
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, Index);
      if (pColumn) {
        WM_MESSAGE Msg;
        pColumn->Width = Width;
        GUI_UNLOCK_H(pColumn);
        Msg.hWin  = WM_GetParent(hObj);
        Msg.MsgId = WM_NOTIFY_CLIENTCHANGE;
        Msg.hWinSrc = hObj;
        WM_InvalidateWindow(hObj);
        WM__SendMessage(Msg.hWin, &Msg);
        WM_InvalidateWindow(WM_GetParent(hObj));
      } else {
        GUI_UNLOCK_H(pColumn);
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       HEADER_GetHeight
*/
int HEADER_GetHeight(HEADER_Handle hObj) {
  int Height = 0;
  if (hObj) {
    GUI_RECT Rect;
    WM_GetClientRectEx(hObj, &Rect);
    GUI_MoveRect(&Rect, -Rect.x0, -Rect.y0);
    Height = Rect.y1 - Rect.y0 + 1;
  }
  return Height;
}

/*********************************************************************
*
*       HEADER_GetItemWidth
*/
int HEADER_GetItemWidth(HEADER_Handle hObj, unsigned int Index) {
  int Width = 0;
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if (Index <= GUI_ARRAY_GetNumItems(pObj->Columns)) {
      HEADER_COLUMN * pColumn;
      pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(pObj->Columns, Index);
      Width = pColumn->Width;
      GUI_UNLOCK_H(pColumn);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Width;
}

/*********************************************************************
*
*       HEADER_GetNumItems
*/
int  HEADER_GetNumItems(HEADER_Handle hObj) {
  int NumCols = 0;
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    NumCols = GUI_ARRAY_GetNumItems(pObj->Columns);
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return NumCols;
}

/*********************************************************************
*
*       HEADER_SetDirIndicator
*/
void HEADER_SetDirIndicator(HEADER_Handle hObj, int Column, int Reverse) {
  if (hObj) {
    HEADER_Obj * pObj;
    WM_LOCK();
    pObj = HEADER_LOCK_H(hObj);
    if ((pObj->DirIndicatorColumn != Column) || (pObj->DirIndicatorReverse != Reverse)) {
      pObj->DirIndicatorColumn = Column;
      pObj->DirIndicatorReverse = Reverse;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

#else /* avoid empty object files */

void HEADER_C(void);
void HEADER_C(void){}

#endif  /* #if GUI_WINSUPPORT */
