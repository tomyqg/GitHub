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
File        : LISTBOX.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI_ARRAY.h"
#include "LISTBOX_Private.h"
#include "SCROLLBAR.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef LISTBOX_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define LISTBOX_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define LISTBOX_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define LISTBOX_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif


/* Define colors */
#ifndef LISTBOX_BKCOLOR0_DEFAULT
  #define LISTBOX_BKCOLOR0_DEFAULT GUI_WHITE     /* Not selected */
#endif

#ifndef LISTBOX_BKCOLOR1_DEFAULT
  #define LISTBOX_BKCOLOR1_DEFAULT GUI_GRAY      /* Selected, no focus */
#endif

#ifndef LISTBOX_BKCOLOR2_DEFAULT
  #define LISTBOX_BKCOLOR2_DEFAULT GUI_BLUE      /* Selected, focus */
#endif

#ifndef LISTBOX_BKCOLOR3_DEFAULT
  #define LISTBOX_BKCOLOR3_DEFAULT 0xC0C0C0      /* Disabled */
#endif


#ifndef LISTBOX_TEXTCOLOR0_DEFAULT
  #define LISTBOX_TEXTCOLOR0_DEFAULT GUI_BLACK   /* Not selected */
#endif

#ifndef LISTBOX_TEXTCOLOR1_DEFAULT
  #define LISTBOX_TEXTCOLOR1_DEFAULT GUI_WHITE   /* Selected, no focus */
#endif

#ifndef LISTBOX_TEXTCOLOR2_DEFAULT
  #define LISTBOX_TEXTCOLOR2_DEFAULT GUI_WHITE   /* Selected, focus */
#endif

#ifndef LISTBOX_TEXTCOLOR3_DEFAULT
  #define LISTBOX_TEXTCOLOR3_DEFAULT GUI_GRAY    /* Disabled */
#endif

#ifndef LISTBOX_TEXTALIGN_DEFAULT
  #define LISTBOX_TEXTALIGN_DEFAULT GUI_TA_LEFT  /* Left aligned */
#endif

#ifndef LISTBOX_SCROLLSTEP_H_DEFAULT
  #define LISTBOX_SCROLLSTEP_H_DEFAULT 10
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/


LISTBOX_PROPS LISTBOX_DefaultProps = {
  LISTBOX_FONT_DEFAULT,
  LISTBOX_SCROLLSTEP_H_DEFAULT,
  {
    LISTBOX_BKCOLOR0_DEFAULT,
    LISTBOX_BKCOLOR1_DEFAULT,
    LISTBOX_BKCOLOR2_DEFAULT,
    LISTBOX_BKCOLOR3_DEFAULT,
  },
  {
    LISTBOX_TEXTCOLOR0_DEFAULT,
    LISTBOX_TEXTCOLOR1_DEFAULT,
    LISTBOX_TEXTCOLOR2_DEFAULT,
    LISTBOX_TEXTCOLOR3_DEFAULT,
  },
  {
    GUI_INVALID_COLOR,
    GUI_INVALID_COLOR,
    GUI_INVALID_COLOR,
  },
  LISTBOX_TEXTALIGN_DEFAULT
};


/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _CallOwnerDraw
*/
static int _CallOwnerDraw(LISTBOX_Handle hObj, int Cmd, int ItemIndex) {
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  LISTBOX_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawItem;
  int r;
  ItemInfo.Cmd       = Cmd;
  ItemInfo.hWin      = hObj;
  ItemInfo.ItemIndex = ItemIndex;
  pObj = LISTBOX_LOCK_H(hObj);
  pfDrawItem = pObj->pfDrawItem;
  GUI_UNLOCK_H(pObj);
  if (pfDrawItem) {
    r = pfDrawItem(&ItemInfo);
  }
  return r;
}

/*********************************************************************
*
*       _GetYSize
*/
static int _GetYSize(LISTBOX_Handle hObj) {
  GUI_RECT Rect;
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  return (Rect.y1 - Rect.y0 + 1);
}

/*********************************************************************
*
*       _GetItemSizeY
*/
static int _GetItemSizeY(LISTBOX_Handle hObj, unsigned Index) {
  int ySize;
  LISTBOX_Obj * pObj;
  LISTBOX_ITEM * pItem;
  GUI_ARRAY ItemArray;
  const GUI_FONT GUI_UNI_PTR * pFont;
  pObj = LISTBOX_LOCK_H(hObj);
  pFont = pObj->Props.pFont;
  ItemArray = pObj->ItemArray;
  GUI_UNLOCK_H(pObj);
  pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
  if (pItem == NULL) {
    return 0;
  }
  ySize = pItem->ySize;
  if (ySize == 0) {
    const GUI_FONT GUI_UNI_PTR * pOldFont;
    pOldFont = GUI_SetFont(pFont);
    GUI_UNLOCK_H(pItem);
    ySize    = _CallOwnerDraw(hObj, WIDGET_ITEM_GET_YSIZE, Index);
    GUI_SetFont(pOldFont);
    if (ySize) {
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
      pItem->ySize = ySize;
      GUI_UNLOCK_H(pItem);
    }
  } else {
    GUI_UNLOCK_H(pItem);
  }
  return ySize;
}

/*********************************************************************
*
*       _GetContentsSizeX
*/
static int _GetContentsSizeX(LISTBOX_Handle hObj) {
  int ContentSizeX;
  LISTBOX_Obj  * pObj;
  LISTBOX_ITEM * pItem;
  int i, NumItems, xSize;
  GUI_ARRAY ItemArray;
  const GUI_FONT GUI_UNI_PTR * pFont;

  pObj = LISTBOX_LOCK_H(hObj); {
    pFont = pObj->Props.pFont;
    NumItems = LISTBOX__GetNumItems(pObj);
    ItemArray = pObj->ItemArray;
    ContentSizeX = pObj->ContentSizeX;
  } GUI_UNLOCK_H(pObj);
  if (ContentSizeX == 0) {
    for (i = 0; i < NumItems; i++) {
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i);
      if (pItem == NULL) {
        continue;
      }
      xSize = pItem->xSize;
      if (xSize == 0) {
        const GUI_FONT GUI_UNI_PTR * pOldFont;
        pOldFont = GUI_SetFont(pFont);
        GUI_UNLOCK_H(pItem);
        xSize    = _CallOwnerDraw(hObj, WIDGET_ITEM_GET_XSIZE, i);
        GUI_SetFont(pOldFont);
        if (xSize) {
          pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i);
          pItem->xSize = xSize;
          GUI_UNLOCK_H(pItem);
        }
      } else {
        GUI_UNLOCK_H(pItem);
      }
      if (ContentSizeX < xSize) {
        ContentSizeX = xSize;
      }
    }
  }
  return ContentSizeX;
}

/*********************************************************************
*
*       _GetItemPosY
*/
static int _GetItemPosY(LISTBOX_Handle hObj, unsigned Index) {
  int ScrollPosV;
  LISTBOX_Obj * pObj;
  unsigned NumItems;
  int i;
  int PosY;
  LISTBOX_ITEM * pItem;
  GUI_ARRAY ItemArray;
    
  //
  // Get properties of object
  //
  pObj = LISTBOX_LOCK_H(hObj); {
    ScrollPosV = pObj->ScrollStateV.v;
    NumItems   = LISTBOX__GetNumItems(pObj);
    ItemArray  = pObj->ItemArray;
  } GUI_UNLOCK_H(pObj);
  if (Index < NumItems) {
    if ((int)Index >= ScrollPosV) {
      //
      // Get position from item
      //
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index); {
        PosY = pItem->ItemPosY;
      } GUI_UNLOCK_H(pItem);
      //
      // Calculate position only if not initialized
      //
      if (PosY < 0) {
        //
        // Get latest valid y-position
        //
        i = Index;
        while (--i >= 0) {
          pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, i); {
            PosY = pItem->ItemPosY;
          } GUI_UNLOCK_H(pItem);
          if (PosY >= 0) {
            break;
          }
        }
        //
        // Add sizes from elements above
        //
        if (PosY < 0) {
          PosY = i = 0;
        }
        if (Index) {
          do {
            PosY += _GetItemSizeY(hObj, i);
          } while ((unsigned)++i < Index);
        }
        //
        // Save value
        //
        pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index); {
          pItem->ItemPosY = PosY;
        } GUI_UNLOCK_H(pItem);
      }
      return PosY;
    }
  }
  return -1;
}

/*********************************************************************
*
*       _IsPartiallyVis
*/
static int _IsPartiallyVis(LISTBOX_Handle hObj) {
  LISTBOX_Obj * pObj;
  int Index, NumItems, ScrollPosV, y;
  pObj = LISTBOX_LOCK_H(hObj);
  Index = pObj->Sel;
  NumItems = (int)LISTBOX__GetNumItems(pObj);
  ScrollPosV = pObj->ScrollStateV.v;
  GUI_UNLOCK_H(pObj);
  if (Index < NumItems) {
    if (Index >= ScrollPosV) {
      y  = _GetItemPosY (hObj, Index);
      y += _GetItemSizeY(hObj, Index);
      y -= _GetItemPosY (hObj, ScrollPosV);
      if (y > _GetYSize(hObj)) {
        return 1;
      }
    }
  }
  return 0;
}

/*********************************************************************
*
*       _GetNumVisItems
*
*  Returns:
*   Number of fully or partially visible items
*/
static unsigned _GetNumVisItems(LISTBOX_Handle hObj) {
  LISTBOX_Obj * pObj;
  int NumItems, r = 1;
  pObj = LISTBOX_LOCK_H(hObj);
  NumItems = LISTBOX__GetNumItems(pObj);
  GUI_UNLOCK_H(pObj);
  if (NumItems > 1) {
    int i, ySize, DistY = 0;
    ySize = _GetYSize(hObj);
    for (i = NumItems - 1; i >= 0; i--) {
      DistY += _GetItemSizeY(hObj, i);
      if (DistY > ySize) {
        break;
      }
    }
    r = NumItems - i - 1;
    if (r < 1) {
      return 1;
    }
  }
  return r;
}

/*********************************************************************
*
*       _NotifyOwner
*
* Purpose:
*   Notify owner of the window by sending a WM_NOTIFY_PARENT message.
*   If no owner is registered, the parent is considered owner.
*/
static void _NotifyOwner(WM_HWIN hObj, int Notification) {
  WM_MESSAGE Msg = {0};
  WM_HWIN hOwner;
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(hObj);
  Msg.MsgId  = WM_NOTIFY_PARENT;
  Msg.Data.v = Notification;
  Msg.hWinSrc= hObj;
  GUI_UNLOCK_H(pObj);
  WM__SendMessage(hOwner, &Msg);
}

/*********************************************************************
*
*       _SendToOwner
*
* Purpose:
*   Sends the given emssage to the owner of the window.
*   If no owner is registered, the parent is considered owner.
*/
static void _SendToOwner(WM_HWIN hObj, WM_MESSAGE * pMsg) {
  WM_HWIN hOwner;
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(hObj);
  pMsg->hWinSrc= hObj;
  GUI_UNLOCK_H(pObj);
  WM__SendMessage(hOwner, pMsg);
}

/*********************************************************************
*
*       _UpdateScrollPos
*
* Purpose:
*   Checks whether if we must scroll up or scroll down to ensure
*   that selection is in the visible area. This function also
*   makes sure that scroll positions are in valid ranges.
*
* Return value:
*   Difference between old and new vertical scroll pos.
*/
static int _UpdateScrollPos(LISTBOX_Handle hObj) {
  WM_SCROLL_STATE ScrollStateV, ScrollStateH;
  LISTBOX_Obj * pObj;
  int PrevScrollStateV;
  int IsPartiallyVis;
  int r;
  IsPartiallyVis = _IsPartiallyVis(hObj);
  pObj = LISTBOX_LOCK_H(hObj);
  PrevScrollStateV = pObj->ScrollStateV.v;
  if (pObj->Sel >= 0) {
    /* Check upper limit */
    if (IsPartiallyVis) {
      pObj->ScrollStateV.v = pObj->Sel - (pObj->ScrollStateV.PageSize - 1);
    }
    /* Check lower limit */
    if (pObj->Sel < pObj->ScrollStateV.v) {
      pObj->ScrollStateV.v = pObj->Sel;
    }
  }
  WM_CheckScrollBounds(&pObj->ScrollStateV);
  WM_CheckScrollBounds(&pObj->ScrollStateH);
  ScrollStateV = pObj->ScrollStateV;
  ScrollStateH = pObj->ScrollStateH;
  GUI_UNLOCK_H(pObj);
  WIDGET__SetScrollState(hObj, &ScrollStateV, &ScrollStateH);
  r = ScrollStateV.v - PrevScrollStateV;
  return r;
}

/*********************************************************************
*
*       _CalcScrollParas
*/
static int _CalcScrollParas(LISTBOX_Handle hObj) {
  int r;
  GUI_RECT Rect;
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  /* Calc vertical scroll parameters */
  pObj->ScrollStateV.NumItems = LISTBOX__GetNumItems(pObj);
  pObj->ScrollStateV.PageSize = _GetNumVisItems(hObj);
  /* Calc horizontal scroll parameters */
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  pObj->ScrollStateH.NumItems = _GetContentsSizeX(hObj);
  pObj->ScrollStateH.PageSize = Rect.x1 - Rect.x0 + 1;
  r = _UpdateScrollPos(hObj);
  GUI_UNLOCK_H(pObj);
  return r;
}

/*********************************************************************
*
*       _ManageAutoScroll
*/
static void _ManageAutoScroll(LISTBOX_Handle hObj) {
  char IsRequired;
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  if (pObj->Flags & LISTBOX_SF_AUTOSCROLLBAR_V) {
    IsRequired = (_GetNumVisItems(hObj) < LISTBOX__GetNumItems(pObj));
    GUI_UNLOCK_H(pObj);
    WM_SetScrollbarV(hObj, IsRequired);
    pObj = LISTBOX_LOCK_H(hObj);
  }
  if (pObj->Flags & LISTBOX_SF_AUTOSCROLLBAR_H) {
    GUI_RECT Rect;
    int xSize, xSizeContents;
    xSizeContents = _GetContentsSizeX(hObj);
    WM_GetInsideRectExScrollbar(hObj, &Rect);
    xSize = Rect.x1 - Rect.x0 + 1;
    IsRequired = (xSizeContents > xSize);
    GUI_UNLOCK_H(pObj);
    WM_SetScrollbarH(hObj, IsRequired);
    pObj = LISTBOX_LOCK_H(hObj);
  }
  if (pObj->ScrollbarWidth) {
    LISTBOX__SetScrollbarWidth(hObj, pObj);
  }
  LISTBOX__SetScrollbarColor(hObj, pObj);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       LISTBOX_UpdateScrollers
*/
int LISTBOX_UpdateScrollers(LISTBOX_Handle hObj) {
  _ManageAutoScroll(hObj);
  return _CalcScrollParas(hObj);
}

/*********************************************************************
*
*       _Tolower
*/
static int _Tolower(int Key) {
  if ((Key >= 0x41) && (Key <= 0x5a)) {
    Key += 0x20;
  }
  return Key;
}

/*********************************************************************
*
*       _IsAlphaNum
*/
static int _IsAlphaNum(int Key) {
  Key = _Tolower(Key);
  if (Key >= 'a' && Key <= 'z') {
    return 1;
  }
  if (Key >= '0' && Key <= '9') {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _SelectByKey
*/
static void _SelectByKey(LISTBOX_Handle hObj, int Key) {
  int i, NumItems;
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  Key = _Tolower(Key);
  NumItems = LISTBOX__GetNumItems(pObj);
  GUI_UNLOCK_H(pObj);
  for (i = 0; i < NumItems; i++) {
    const char * s;
    int FirstChar;
    s = LISTBOX__GetpStringLocked(hObj, i);
    FirstChar = _Tolower(*s);
    GUI_UNLOCK_H(s);
    if (FirstChar == Key) {
      LISTBOX_SetSel(hObj, i);
      break;
    }
  }
}

/*********************************************************************
*
*       _FreeAttached
*/
static void _FreeAttached(LISTBOX_Handle hObj) {
  LISTBOX_Obj * pObj;
  pObj = LISTBOX_LOCK_H(hObj);
  GUI_ARRAY_Delete(pObj->ItemArray);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(LISTBOX_Handle hObj, WM_MESSAGE * pMsg) {
  LISTBOX_Obj * pObj;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  GUI_RECT RectInside, RectItem, ClipRect;
  int ItemDistY, NumItems, i, ScrollStateV;
  WIDGET_DRAW_ITEM_FUNC* pfDrawItem;
  GUI_COLOR BackColor;

  pObj = LISTBOX_LOCK_H(hObj);
  BackColor = pObj->Props.aBackColor[0];
  pfDrawItem = pObj->pfDrawItem;
  ScrollStateV = pObj->ScrollStateV.v;
  NumItems = LISTBOX__GetNumItems(pObj);
  GUI_SetFont(pObj->Props.pFont);
  /* Calculate clipping rectangle */
  ClipRect = *(const GUI_RECT*)pMsg->Data.p;
  GUI_MoveRect(&ClipRect, -pObj->Widget.Win.Rect.x0, -pObj->Widget.Win.Rect.y0);
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  GUI__IntersectRect(&ClipRect, &RectInside);
  RectItem.x0 = ClipRect.x0;
  RectItem.x1 = ClipRect.x1;
  /* Fill item info structure */
  ItemInfo.Cmd  = WIDGET_ITEM_DRAW;
  ItemInfo.hWin = hObj;
  ItemInfo.x0   = RectInside.x0 - pObj->ScrollStateH.v;
  ItemInfo.y0   = RectInside.y0;
  GUI_UNLOCK_H(pObj);
  /* Do the drawing */
  for (i = ScrollStateV; i < NumItems; i++) {
    RectItem.y0 = ItemInfo.y0;
    /* Break when all other rows are outside the drawing area */
    if (RectItem.y0 > ClipRect.y1) {
      break;
    }
    ItemDistY = _GetItemSizeY(hObj, i);
    RectItem.y1 = RectItem.y0 + ItemDistY - 1;
    /* Make sure that we draw only when row is in drawing area */
    if (RectItem.y1 >= ClipRect.y0) {
      /* Set user clip rect */
      WM_SetUserClipArea(&RectItem);
      /* Fill item info structure */
      ItemInfo.ItemIndex = i;
      /* Draw item */
      if (pfDrawItem) {
        pfDrawItem(&ItemInfo);
      }
    }
    ItemInfo.y0 += ItemDistY;
  }
  WM_SetUserClipArea(NULL);
  /* Calculate & clear 'data free' area */
  #if WM_SUPPORT_TRANSPARENCY
  if (!WM_GetHasTrans(hObj)) {
  #else
  {
  #endif
    RectItem.y0 = ItemInfo.y0;
    RectItem.y1 = RectInside.y1;
    LCD_SetBkColor(BackColor);
    GUI_ClearRectEx(&RectItem);
  }
  /* Draw the 3D effect (if configured) */
  pObj = LISTBOX_LOCK_H(hObj);
  WIDGET__EFFECT_DrawDown(&pObj->Widget);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _ToggleMultiSel
*/
static void _ToggleMultiSel(LISTBOX_Handle hObj, int Sel) {
  LISTBOX_Obj * pObj;
  U8 Flags;
  GUI_ARRAY ItemArray;
  pObj = LISTBOX_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  Flags = pObj->Flags;
  GUI_UNLOCK_H(pObj);
  if (Flags & LISTBOX_SF_MULTISEL) {
    WM_HMEM hItem = GUI_ARRAY_GethItem(ItemArray, Sel);
    if (hItem) {
      LISTBOX_ITEM * pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
      if (!(pItem->Status & LISTBOX_ITEM_DISABLED)) {
        pItem->Status ^= LISTBOX_ITEM_SELECTED;
        GUI_UNLOCK_H(pItem);
        _NotifyOwner(hObj, WM_NOTIFICATION_SEL_CHANGED);
        LISTBOX__InvalidateItem(hObj, Sel);
      } else {
        GUI_UNLOCK_H(pItem);
      }
    }
  }
}

/*********************************************************************
*
*       _GetItemFromPos
*/
static int _GetItemFromPos(LISTBOX_Handle hObj, int x, int y) {
  LISTBOX_Obj * pObj;
  int Sel = -1;
  int NumItems, ScrollStateV, ItemSize;
  GUI_RECT Rect;

  pObj = LISTBOX_LOCK_H(hObj);
  ScrollStateV = pObj->ScrollStateV.v; 
  NumItems = LISTBOX__GetNumItems(pObj);
  GUI_UNLOCK_H(pObj);
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  if ((x >= Rect.x0) && (y >= Rect.y0)) {
    if ((x <= Rect.x1) && (y <= Rect.y1)) {
      int i, y0 = Rect.y0;
      for (i = ScrollStateV; i < NumItems; i++) {
        if (y >= y0) {
          Sel = i;
        }
        ItemSize = _GetItemSizeY(hObj, i);
        y0 += ItemSize;
      }
    }
  }
  return Sel;
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(LISTBOX_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed == 0) {
      _NotifyOwner(hObj, WM_NOTIFICATION_RELEASED);
    }
  } else {     /* Mouse moved out */
    _NotifyOwner(hObj, WM_NOTIFICATION_MOVED_OUT);
  }
}

/*********************************************************************
*
*       _OnMouseOver
*/
#if GUI_SUPPORT_MOUSE
static int _OnMouseOver(LISTBOX_Handle hObj, WM_MESSAGE * pMsg) {
  int ScrollStateV, Sel, NumVisItems;
  LISTBOX_Obj * pObj;
  WM_HWIN hOwner;
  const GUI_PID_STATE * pState;
  
  pState = (const GUI_PID_STATE *)pMsg->Data.p;
  pObj = LISTBOX_LOCK_H(hObj);
  ScrollStateV = pObj->ScrollStateV.v;
  hOwner = pObj->hOwner;
  GUI_UNLOCK_H(pObj);
  if (hOwner) {
    if (pState) {  /* Something happened in our area (pressed or released) */
      Sel = _GetItemFromPos(hObj, pState->x, pState->y);
      if (Sel >= 0) {
        NumVisItems = _GetNumVisItems(hObj);
        if (Sel < (int)(ScrollStateV + NumVisItems)) {
          LISTBOX_SetSel(hObj, Sel);
        }
      }
    }
  }
  return 0;                        /* Message handled */
}
#endif

/*********************************************************************
*
*       _MoveSel
*
*  Moves the selection/focus to the next valid item
*/
static void _MoveSel(LISTBOX_Handle hObj, int Dir) {
  int Index, NewSel = -1, NumItems;
  LISTBOX_Obj * pObj;
  GUI_ARRAY ItemArray;
  Index = LISTBOX_GetSel(hObj);
  pObj = LISTBOX_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  NumItems = LISTBOX__GetNumItems(pObj);
  GUI_UNLOCK_H(pObj);
  do {
    WM_HMEM hItem;
    Index += Dir;
    if ((Index < 0) || (Index >= NumItems)) {
      break;
    }
    hItem = GUI_ARRAY_GethItem(ItemArray, Index);
    if (hItem) {
      LISTBOX_ITEM * pItem;
      pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
      if (!(pItem->Status & LISTBOX_ITEM_DISABLED)) {
        NewSel = Index;
      }
      GUI_UNLOCK_H(pItem);
    }
  } while(NewSel < 0);
  if (NewSel >= 0) {
    LISTBOX_SetSel(hObj, NewSel);
  } else {
    int PrevScrollStateV;
    pObj = LISTBOX_LOCK_H(hObj);
    PrevScrollStateV = pObj->ScrollStateV.v;
    pObj->ScrollStateV.v += Dir;
    WM_CheckScrollBounds(&pObj->ScrollStateV);
    if (PrevScrollStateV != pObj->ScrollStateV.v) {
      LISTBOX__InvalidateInsideArea(hObj);
    }
    GUI_UNLOCK_H(pObj);
    LISTBOX_UpdateScrollers(hObj);
  }
}

/*********************************************************************
*
*       _AddKey
*
* Returns: 1 if Key has been consumed
*          0 else 
*/
static int _AddKey(LISTBOX_Handle hObj, int Key) {
  LISTBOX_Obj * pObj;
  int r;
  switch (Key) {
  case GUI_KEY_SPACE:
    _ToggleMultiSel(hObj, LISTBOX_GetSel(hObj));
    return 1;               /* Key has been consumed */
  case GUI_KEY_RIGHT:
    pObj = LISTBOX_LOCK_H(hObj);
    r = WM_SetScrollValue(&pObj->ScrollStateH, pObj->ScrollStateH.v + pObj->Props.ScrollStepH);
    GUI_UNLOCK_H(pObj);
    if (r) {
      LISTBOX_UpdateScrollers(hObj);
      LISTBOX__InvalidateInsideArea(hObj);
    }
    return 1;               /* Key has been consumed */
  case GUI_KEY_LEFT:
    pObj = LISTBOX_LOCK_H(hObj);
    r = WM_SetScrollValue(&pObj->ScrollStateH, pObj->ScrollStateH.v - pObj->Props.ScrollStepH);
    GUI_UNLOCK_H(pObj);
    if (r) {
      LISTBOX_UpdateScrollers(hObj);
      LISTBOX__InvalidateInsideArea(hObj);
    }
    return 1;               /* Key has been consumed */
  case GUI_KEY_DOWN:
    LISTBOX_IncSel(hObj);
    return 1;               /* Key has been consumed */
  case GUI_KEY_UP:
    LISTBOX_DecSel(hObj);
    return 1;               /* Key has been consumed */
  default:
    if(_IsAlphaNum(Key)) {
      _SelectByKey(hObj, Key);
      return 1;               /* Key has been consumed */
    }
  }
  return 0;
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
LISTBOX_Obj * LISTBOX_LockH(LISTBOX_Handle h) {
  LISTBOX_Obj * p = (LISTBOX_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != LISTBOX_ID) {
      GUI_DEBUG_ERROROUT("LISTBOX.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       LISTBOX__GetNumItems
*
*  Returns:
*    Number of items
*/
unsigned LISTBOX__GetNumItems(const LISTBOX_Obj * pObj) {
  return GUI_ARRAY_GetNumItems(pObj->ItemArray);
}

/*********************************************************************
*
*       LISTBOX__GetpStringLocked
*
*  Returns:
*    Pointer to the specified item
*/
const char * LISTBOX__GetpStringLocked(LISTBOX_Handle hObj, int Index) {
  LISTBOX_Obj * pObj;
  const char * s = NULL;
  LISTBOX_ITEM * pItem;
  pObj = LISTBOX_LOCK_H(hObj);
  pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
  GUI_UNLOCK_H(pObj);
  if (pItem) {
    s = pItem->acText;
  }
  return s;
}

/*********************************************************************
*
*       LISTBOX__InvalidateItemSize
*/
void LISTBOX__InvalidateItemSize(const LISTBOX_Obj * pObj, unsigned Index) {
  LISTBOX_ITEM * pItem;
  pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
  if (pItem) {
    pItem->xSize = 0;
    pItem->ySize = 0;
  }
  GUI_UNLOCK_H(pItem);
}

/*********************************************************************
*
*       LISTBOX__InvalidateInsideArea
*/
void LISTBOX__InvalidateInsideArea(LISTBOX_Handle hObj) {
  GUI_RECT Rect;
  WM_GetInsideRectExScrollbar(hObj, &Rect);
  WM_InvalidateRect(hObj, &Rect);
}

/*********************************************************************
*
*       LISTBOX__InvalidateItem
*/
void LISTBOX__InvalidateItem(LISTBOX_Handle hObj, int Sel) {
  int ItemDistY, ItemPosY, ScrollPosV;
  GUI_RECT Rect;
  LISTBOX_Obj * pObj;
  
  if (Sel >= 0) {
    ItemPosY = _GetItemPosY(hObj, Sel);
    if (ItemPosY >= 0) {
      pObj = LISTBOX_LOCK_H(hObj); {
        ScrollPosV = pObj->ScrollStateV.v;
      } GUI_UNLOCK_H(pObj);
      ScrollPosV = _GetItemPosY(hObj, ScrollPosV);
      ItemPosY -= ScrollPosV;
      ItemDistY = _GetItemSizeY(hObj, Sel);
      WM_GetInsideRectExScrollbar(hObj, &Rect);
      Rect.y0 += ItemPosY;
      Rect.y1  = Rect.y0 + ItemDistY - 1;
      WM_InvalidateRect(hObj, &Rect);
    }
  }
}

/*********************************************************************
*
*       LISTBOX__InvalidateItemAndBelow
*/
void LISTBOX__InvalidateItemAndBelow(LISTBOX_Handle hObj, int Sel) {
  int ItemPosY, ScrollPosV;
  GUI_RECT Rect;
  LISTBOX_Obj * pObj;

  if (Sel >= 0) {
    ItemPosY = _GetItemPosY(hObj, Sel);
    if (ItemPosY >= 0) {
      pObj = LISTBOX_LOCK_H(hObj); {
        ScrollPosV = pObj->ScrollStateV.v;
      } GUI_UNLOCK_H(pObj);
      ScrollPosV = _GetItemPosY(hObj, ScrollPosV);
      WM_GetInsideRectExScrollbar(hObj, &Rect);
      Rect.y0 += ItemPosY;
      WM_InvalidateRect(hObj, &Rect);
    }
  }
}

/*********************************************************************
*
*       LISTBOX__SetScrollbarWidth
*/
void LISTBOX__SetScrollbarWidth(LISTBOX_Handle hObj, const LISTBOX_Obj * pObj) {
  WM_HWIN hBarH, hBarV;
  int Width;
  Width = pObj->ScrollbarWidth;
  if (Width == 0) {
    Width = SCROLLBAR_GetDefaultWidth();
  }
  hBarH = WM_GetDialogItem(hObj, GUI_ID_HSCROLL);
  hBarV = WM_GetDialogItem(hObj, GUI_ID_VSCROLL);
  SCROLLBAR_SetWidth(hBarH, Width);
  SCROLLBAR_SetWidth(hBarV, Width);
}

/*********************************************************************
*
*       LISTBOX__SetScrollbarColor
*/
void LISTBOX__SetScrollbarColor(LISTBOX_Handle hObj, const LISTBOX_Obj * pObj) {
  WM_HWIN hBarH, hBarV;
  int i;
  hBarH = WM_GetDialogItem(hObj, GUI_ID_HSCROLL);
  hBarV = WM_GetDialogItem(hObj, GUI_ID_VSCROLL);
  if (hBarH || hBarV) {
    for (i = 0; i < GUI_COUNTOF(pObj->Props.aScrollbarColor); i++) {
      if (pObj->Props.aScrollbarColor[i] != GUI_INVALID_COLOR) {
        SCROLLBAR_SetColor(hBarH, i, pObj->Props.aScrollbarColor[i]);
        SCROLLBAR_SetColor(hBarV, i, pObj->Props.aScrollbarColor[i]);
      }
    }
  }
}

/*********************************************************************
*
*       LISTBOX__AddSize
*/
void LISTBOX__AddSize(LISTBOX_Obj * pObj, int Index) {
  const GUI_FONT GUI_UNI_PTR * pOldFont;
  LISTBOX_ITEM * pItem;
  int SizeX;

  pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index); {
    pOldFont = GUI_SetFont(pObj->Props.pFont);
    SizeX = GUI_GetStringDistX(pItem->acText);
    GUI_SetFont(pOldFont);
    pItem->xSize = SizeX;
  } GUI_UNLOCK_H(pItem);
  if (pObj->ContentSizeX < SizeX) {
    pObj->ContentSizeX = SizeX;
  }
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       LISTBOX_OwnerDraw
*/
int LISTBOX_OwnerDraw(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  switch (pDrawItemInfo->Cmd) {
    case WIDGET_ITEM_GET_XSIZE: {
      LISTBOX_Obj * pObj;
      const GUI_FONT GUI_UNI_PTR* pOldFont;
      const char * s;
      int DistX;
      pObj = LISTBOX_LOCK_H(pDrawItemInfo->hWin);
      pOldFont = GUI_SetFont(pObj->Props.pFont);
      GUI_UNLOCK_H(pObj);
      s = LISTBOX__GetpStringLocked(pDrawItemInfo->hWin, pDrawItemInfo->ItemIndex);
      DistX = GUI_GetStringDistX(s);
      GUI_UNLOCK_H(s);
      GUI_SetFont(pOldFont);
      return DistX;
    }
    case WIDGET_ITEM_GET_YSIZE: {
      int r;
      LISTBOX_Obj * pObj;
      pObj = LISTBOX_LOCK_H(pDrawItemInfo->hWin);
      r = GUI_GetYDistOfFont(pObj->Props.pFont) + pObj->ItemSpacing;
      GUI_UNLOCK_H(pObj);
      return r;
    }
    case WIDGET_ITEM_DRAW: {
      LISTBOX_Obj * pObj;
      LISTBOX_ITEM * pItem;
      WM_HMEM hItem;
      GUI_RECT r, rText;
      int FontDistY;
      int ItemIndex;
      const char * s;
      int ColorIndex;
      char IsDisabled;
      char IsSelected;
      ItemIndex = pDrawItemInfo->ItemIndex;
      pObj = LISTBOX_LOCK_H(pDrawItemInfo->hWin);
      hItem = GUI_ARRAY_GethItem(pObj->ItemArray, ItemIndex);
      pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
      WM_GetInsideRect(&r);
      FontDistY = GUI_GetFontDistY();
      /* Calculate color index */
      IsDisabled = (pItem->Status & LISTBOX_ITEM_DISABLED) ? 1 : 0;
      IsSelected = (pItem->Status & LISTBOX_ITEM_SELECTED) ? 1 : 0;
      if (pObj->Flags & LISTBOX_SF_MULTISEL) {
        if (IsDisabled) {
          ColorIndex = LISTBOX_CI_DISABLED;
        } else {
          ColorIndex = (IsSelected) ? LISTBOX_CI_SELFOCUS : LISTBOX_CI_UNSEL;
        }
      } else {
        if (IsDisabled) {
          ColorIndex = LISTBOX_CI_DISABLED;
        } else {
          if (ItemIndex == pObj->Sel) {
            ColorIndex = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? LISTBOX_CI_SELFOCUS : LISTBOX_CI_SEL;
          } else {
            ColorIndex = LISTBOX_CI_UNSEL;
          }
        }
      }
      /* Display item */
      LCD_SetBkColor(pObj->Props.aBackColor[ColorIndex]);
      LCD_SetColor  (pObj->Props.aTextColor[ColorIndex]);
      GUI_SetTextMode(GUI_TM_TRANS);
      GUI_Clear();
      rText.y0 = pDrawItemInfo->y0;
      rText.x0 = pDrawItemInfo->x0 + 1;
      rText.y1 = rText.y0 + pItem->ySize - 1;
      if (pObj->Props.Align & (GUI_TA_RIGHT | GUI_TA_CENTER)) {
        GUI_RECT Rect;
        WM_GetInsideRectExScrollbar(pDrawItemInfo->hWin, &Rect);
        rText.x1 = rText.x0 + Rect.x1 - Rect.x0 - pObj->Widget.pEffect->EffectSize;
      } else {
        rText.x1 = rText.x0 + pItem->xSize - 1;
      }
      s = LISTBOX__GetpStringLocked(pDrawItemInfo->hWin, ItemIndex);
      GUI_UNLOCK_H(pItem);
      GUI_DispStringInRect(s, &rText, pObj->Props.Align);
      GUI_UNLOCK_H(s);
      /* Display focus rectangle */
      if ((pObj->Flags & LISTBOX_SF_MULTISEL) && (ItemIndex == pObj->Sel)) {
        GUI_RECT rFocus;
        rFocus.x0 = pDrawItemInfo->x0;
        rFocus.x1 = r.x1;
        switch (pObj->Props.Align & GUI_TA_VERTICAL) {
        case GUI_TA_TOP:
          rFocus.y0 = pDrawItemInfo->y0;
          rFocus.y1 = pDrawItemInfo->y0 + FontDistY - 1;
          break;
        case GUI_TA_VCENTER:
          rFocus.y0 = pDrawItemInfo->y0 + (GUI_Context.WM__pUserClipRect->y1 - GUI_Context.WM__pUserClipRect->y0 - FontDistY) / 2;
          rFocus.y1 = rFocus.y0 + FontDistY - 1;
          break;
        case GUI_TA_BOTTOM:
          rFocus.y1 = pDrawItemInfo->y0 + (GUI_Context.WM__pUserClipRect->y1 - GUI_Context.WM__pUserClipRect->y0);
          rFocus.y0 = rFocus.y1 - FontDistY + 1;
          break;
        }
        LCD_SetColor(GUI_WHITE - pObj->Props.aBackColor[ColorIndex]);
        GUI_DrawFocusRect(&rFocus, 0);
      }
      GUI_UNLOCK_H(pObj);
      return 0;
    }
  }
  return 0;
}

/*********************************************************************
*
*       LISTBOX_Callback
*/
void LISTBOX_Callback(WM_MESSAGE * pMsg) {
  LISTBOX_Handle hObj;
  LISTBOX_Obj * pObj;
  WM_SCROLL_STATE ScrollState;
  WM_MESSAGE Msg;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    /* Owner needs to be informed about focus change */
    if (pMsg->MsgId == WM_SET_FOCUS) {
      if (pMsg->Data.v == 0) {            /* Lost focus ? */
        _NotifyOwner(hObj, LISTBOX_NOTIFICATION_LOST_FOCUS);
      }
    }
    return;
  }
  switch (pMsg->MsgId) {
  case WM_NOTIFY_PARENT:
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED:
      if (pMsg->hWinSrc  == WM_GetScrollbarV(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj = LISTBOX_LOCK_H(hObj);
        pObj->ScrollStateV.v = ScrollState.v;
        GUI_UNLOCK_H(pObj);
        LISTBOX__InvalidateInsideArea(hObj);
        _NotifyOwner(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      } else if (pMsg->hWinSrc == WM_GetScrollbarH(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj = LISTBOX_LOCK_H(hObj);
        pObj->ScrollStateH.v = ScrollState.v;
        GUI_UNLOCK_H(pObj);
        LISTBOX__InvalidateInsideArea(hObj);
        _NotifyOwner(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      }
      break;
    case WM_NOTIFICATION_SCROLLBAR_ADDED:
      #if WIDGET_USE_PARENT_EFFECT
        pObj = LISTBOX_LOCK_H(hObj);
        WIDGET_SetEffect(pMsg->hWinSrc, pObj->Widget.pEffect);
        GUI_UNLOCK_H(pObj);
      #endif
      LISTBOX_UpdateScrollers(hObj);
      break;
    }
    break;
  case WM_PAINT:
    _OnPaint(hObj, pMsg);
    break;
  case WM_PID_STATE_CHANGED:
    {
      const WM_PID_STATE_CHANGED_INFO * pInfo = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
      if (pInfo->State) {
        int Sel;
        Sel = _GetItemFromPos(hObj, pInfo->x, pInfo->y);
        if (Sel >= 0) {
          _ToggleMultiSel(hObj, Sel);
          LISTBOX_SetSel(hObj, Sel);
          _NotifyOwner(hObj, WM_NOTIFICATION_CLICKED);
        }
        return;
      }
    }
    break;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    return;
#if GUI_SUPPORT_MOUSE
  case WM_MOUSEOVER:
    if (_OnMouseOver(hObj, pMsg) == 0)
      return;
    break;
#endif
  case WM_DELETE:
    _FreeAttached(hObj);
    break;       /* No return here ... WM_DefaultProc needs to be called */
  case WM_KEY:
    if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) {
      int Key;
      Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
      if (LISTBOX_AddKey(hObj, Key)) {
        return;
      }
    }
    /* Send a WM_NOTIFY_OWNER_KEY message with the key information to the owner window */
    Msg = *pMsg;
    Msg.MsgId = WM_NOTIFY_OWNER_KEY;
    _SendToOwner(hObj, &Msg);
    if (!WM_IsWindow(hObj)) {
      return;
    }
    break;
  case WM_SIZE:
    LISTBOX_UpdateScrollers(hObj);
    WM_InvalidateWindow(hObj);
    break;
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       LISTBOX_CreateEx
*/
LISTBOX_Handle LISTBOX_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                int WinFlags, int ExFlags, int Id, const GUI_ConstString * ppText)
{
  LISTBOX_Handle hObj;
  GUI_USE_PARA(ExFlags);
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, LISTBOX_Callback,
                                sizeof(LISTBOX_Obj) - sizeof(WM_Obj));
  if (hObj) {
    GUI_ARRAY ItemArray;
    LISTBOX_Obj * pObj;
    ItemArray = GUI_ARRAY_Create();
    if (ItemArray) {
      pObj = (LISTBOX_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
       /* Init sub-classes */
      pObj->ItemArray = ItemArray;
     /* init widget specific variables */
      WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
      LISTBOX_INIT_ID(pObj);
      pObj->Props = LISTBOX_DefaultProps;
      pObj->pfDrawItem = LISTBOX_OwnerDraw;
      GUI_UNLOCK_H(pObj);
      if (ppText) {
        /* init member variables */
        /* Set non-zero attributes */
        LISTBOX_SetText(hObj, ppText);
      }
      LISTBOX_UpdateScrollers(hObj);
    } else {
      WM_DeleteWindow(hObj);
    }
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       LISTBOX_InvalidateItem
*/
void LISTBOX_InvalidateItem(LISTBOX_Handle hObj, int Index) {
  LISTBOX_ITEM * pItem;
  LISTBOX_Obj * pObj;
  int i, NumItems;

  if (hObj) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    NumItems = LISTBOX__GetNumItems(pObj);
    if (Index < NumItems) {
      if (Index < 0) {
        for (i = 0; i < NumItems; i++) {
          LISTBOX__InvalidateItemSize(pObj, i);
          pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, i); {
            pItem->ItemPosY = -1;
          } GUI_UNLOCK_H(pItem);
        }
        pObj->ContentSizeX = 0;
        GUI_UNLOCK_H(pObj);
        LISTBOX_UpdateScrollers(hObj);
        LISTBOX__InvalidateInsideArea(hObj);
      } else {
        LISTBOX__InvalidateItemSize(pObj, Index);
        GUI_UNLOCK_H(pObj);
        LISTBOX_UpdateScrollers(hObj);
        LISTBOX__InvalidateItemAndBelow(hObj, Index);
      }
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_AddKey
*
* Returns: 1 if Key has been consumed
*          0 else 
*/
int LISTBOX_AddKey(LISTBOX_Handle hObj, int Key) {
  int r = 0;
  if (hObj) {
    WM_LOCK();
    r = _AddKey(hObj, Key);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       LISTBOX_AddString
*/
void LISTBOX_AddString(LISTBOX_Handle hObj, const char * s) {
  GUI_ARRAY ItemArray;
  LISTBOX_Obj  * pObj;
  LISTBOX_ITEM * pItem;
  LISTBOX_ITEM Item = {0, 0};
  unsigned ItemIndex;

  if (hObj && s) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    if (GUI_ARRAY_AddItem(ItemArray, &Item, sizeof(LISTBOX_ITEM) + strlen(s)) == 0) {
      ItemIndex = GUI_ARRAY_GetNumItems(ItemArray) - 1;
      pItem = (LISTBOX_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
      pItem->ItemPosY = -1;
      strcpy(pItem->acText, s);
      GUI_UNLOCK_H(pItem);
      pObj = LISTBOX_LOCK_H(hObj);
      LISTBOX__InvalidateItemSize(pObj, ItemIndex);
      LISTBOX__AddSize(pObj, ItemIndex);
      GUI_UNLOCK_H(pObj);
      LISTBOX_UpdateScrollers(hObj);
      LISTBOX__InvalidateItem(hObj, ItemIndex);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_SetText
*/
void LISTBOX_SetText(LISTBOX_Handle hObj, const GUI_ConstString * ppText) {
  if (hObj) {
    int i;
    const char * s;
    WM_LOCK();
    LISTBOX_InvalidateItem(hObj, LISTBOX_ALL_ITEMS);
    if (ppText) {
      for (i = 0; (s = *(ppText + i)) != 0; i++) {
        LISTBOX_AddString(hObj, s);
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_SetSel
*/
void LISTBOX_SetSel (LISTBOX_Handle hObj, int NewSel) {
  if (hObj) {
    LISTBOX_Obj * pObj;
    int MaxSel;
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    MaxSel = LISTBOX__GetNumItems(pObj);
    MaxSel = MaxSel ? MaxSel - 1 : 0;
    if (NewSel > MaxSel) {
      NewSel = MaxSel;
    }
    if (NewSel < 0) {
      NewSel = -1;
    } else {
      WM_HMEM hItem = GUI_ARRAY_GethItem(pObj->ItemArray, NewSel);
      if (hItem) {
        LISTBOX_ITEM * pItem = (LISTBOX_ITEM *)GUI_LOCK_H(hItem);
        if (pItem->Status & LISTBOX_ITEM_DISABLED) {
          NewSel = -1;
        }
        GUI_UNLOCK_H(pItem);
      }
    }
    if (NewSel != pObj->Sel) {
      int OldSel;
      OldSel    = pObj->Sel;
      pObj->Sel = NewSel;
      GUI_UNLOCK_H(pObj);
      if (_UpdateScrollPos(hObj)) {
        LISTBOX__InvalidateInsideArea(hObj);
      } else {
        LISTBOX__InvalidateItem(hObj, OldSel);
        LISTBOX__InvalidateItem(hObj, NewSel);
      }
      _NotifyOwner(hObj, WM_NOTIFICATION_SEL_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_GetSel
*/
int  LISTBOX_GetSel (LISTBOX_Handle hObj) {
  int r = -1;
  LISTBOX_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = LISTBOX_LOCK_H(hObj);
    r = pObj->Sel;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       LISTBOX_IncSel
*/
void LISTBOX_IncSel      (LISTBOX_Handle hObj) {
  if (hObj) {
    WM_LOCK();
    _MoveSel(hObj, 1);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       LISTBOX_DecSel
*/
void LISTBOX_DecSel      (LISTBOX_Handle hObj) {
  if (hObj) {
    WM_LOCK();
    _MoveSel(hObj, -1);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void LISTBOX_C(void) {}
#endif

/*************************** End of file ****************************/

