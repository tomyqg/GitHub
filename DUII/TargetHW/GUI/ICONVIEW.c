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
File        : ICONVIEW.c
Purpose     : Implementation of listbox widget
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "ICONVIEW_Private.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/
/* Define default fonts */
#ifndef   ICONVIEW_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define ICONVIEW_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define ICONVIEW_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define ICONVIEW_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef   ICONVIEW_BKCOLOR0_DEFAULT
  #define ICONVIEW_BKCOLOR0_DEFAULT GUI_WHITE               /* Not selected */
#endif

#ifndef   ICONVIEW_BKCOLOR1_DEFAULT
  #define ICONVIEW_BKCOLOR1_DEFAULT GUI_BLUE                /* Selected */
#endif

#ifndef   ICONVIEW_BKCOLOR2_DEFAULT
  #define ICONVIEW_BKCOLOR2_DEFAULT 0xC0C0C0                /* Disabled */
#endif

#ifndef   ICONVIEW_TEXTCOLOR0_DEFAULT
  #define ICONVIEW_TEXTCOLOR0_DEFAULT GUI_WHITE             /* Not selected */
#endif

#ifndef   ICONVIEW_TEXTCOLOR1_DEFAULT
  #define ICONVIEW_TEXTCOLOR1_DEFAULT GUI_WHITE             /* Selected */
#endif

#ifndef   ICONVIEW_TEXTCOLOR2_DEFAULT
  #define ICONVIEW_TEXTCOLOR2_DEFAULT GUI_GRAY              /* Disabled */
#endif

#ifndef   ICONVIEW_FRAMEX_DEFAULT
  #define ICONVIEW_FRAMEX_DEFAULT 5
#endif

#ifndef   ICONVIEW_FRAMEY_DEFAULT
  #define ICONVIEW_FRAMEY_DEFAULT 5
#endif

#ifndef   ICONVIEW_SPACEX_DEFAULT
  #define ICONVIEW_SPACEX_DEFAULT 5
#endif

#ifndef   ICONVIEW_SPACEY_DEFAULT
  #define ICONVIEW_SPACEY_DEFAULT 5
#endif

#ifndef   ICONVIEW_ALIGN_DEFAULT
  #define ICONVIEW_ALIGN_DEFAULT GUI_TA_HCENTER | GUI_TA_BOTTOM
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
ICONVIEW_PROPS ICONVIEW__DefaultProps = {
  ICONVIEW_FONT_DEFAULT,
  {
    ICONVIEW_BKCOLOR0_DEFAULT,
    ICONVIEW_BKCOLOR1_DEFAULT,
    ICONVIEW_BKCOLOR2_DEFAULT,
  },
  {
    ICONVIEW_TEXTCOLOR0_DEFAULT,
    ICONVIEW_TEXTCOLOR1_DEFAULT,
    ICONVIEW_TEXTCOLOR2_DEFAULT,
  },
  ICONVIEW_FRAMEX_DEFAULT,
  ICONVIEW_FRAMEY_DEFAULT,
  ICONVIEW_SPACEX_DEFAULT,
  ICONVIEW_SPACEY_DEFAULT,
  ICONVIEW_ALIGN_DEFAULT,
};

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       ICONVIEW_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
ICONVIEW_OBJ * ICONVIEW_LockH(ICONVIEW_Handle h) {
  ICONVIEW_OBJ * p = (ICONVIEW_OBJ *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != ICONVIEW_ID) {
      GUI_DEBUG_ERROROUT("ICONVIEW.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       _LightRect
*/
static void _LightRect(GUI_RECT * pRectItem, GUI_COLOR AlphaColor) {
  GUI_COLOR Color;
  U8 Alpha;
  Color = AlphaColor & 0x00FFFFFF;
  Alpha = AlphaColor >> 24;
  GUI_SetColor(Color);
  GUI_SetAlpha(Alpha);
  GUI_FillRectEx(pRectItem);
  GUI_SetAlpha(0);
}

/*********************************************************************
*
*       _GetNumItemsPerLine
*/
static int _GetNumItemsPerLine(ICONVIEW_Handle hObj) {
  ICONVIEW_OBJ * pObj;
  GUI_RECT RectInside;
  int ItemsPerLine;
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  pObj = ICONVIEW_LOCK_H(hObj);
  ItemsPerLine = (RectInside.x1 - RectInside.x0 + 1 + pObj->Props.SpaceX) / (pObj->xSizeItems + pObj->Props.SpaceX);
  GUI_UNLOCK_H(pObj);
  return ItemsPerLine;
}

/*********************************************************************
*
*       _GetNumLines
*/
static int _GetNumLines(ICONVIEW_Handle hObj, ICONVIEW_OBJ * pObj) {
  GUI_RECT RectInside;
  int ItemsPerLine;
  int NumLines;
  int NumItems;
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  ItemsPerLine = _GetNumItemsPerLine(hObj);
  NumItems = GUI_ARRAY_GetNumItems(pObj->ItemArray);
  NumLines = (NumItems + ItemsPerLine - 1) / ItemsPerLine;
  return NumLines;
}

/*********************************************************************
*
*       _GetNumVisLines
*/
static int _GetNumVisLines(ICONVIEW_Handle hObj, ICONVIEW_OBJ * pObj) {
  GUI_RECT RectInside;
  int NumVisLines;
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  NumVisLines = (RectInside.y1 - RectInside.y0 + 1 + pObj->Props.SpaceY) / (pObj->ySizeItems + pObj->Props.SpaceY);
  return NumVisLines;
}

/*********************************************************************
*
*       _GetRelRectFromIndex
*/
static void _GetRelRectFromIndex(ICONVIEW_Handle hObj, GUI_RECT * pRect, int Index) {
  ICONVIEW_OBJ * pObj;
  GUI_RECT RectInside;
  int ItemsPerLine;
  int Row, Col;
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  ItemsPerLine = _GetNumItemsPerLine(hObj);
  pObj = ICONVIEW_LOCK_H(hObj);
  Row = Index / ItemsPerLine - pObj->ScrollStateV.v;
  Col = Index - (Row + pObj->ScrollStateV.v) * ItemsPerLine;
  pRect->x0 = pObj->Props.FrameX + Col * (pObj->xSizeItems + pObj->Props.SpaceX);
  pRect->y0 = pObj->Props.FrameY + Row * (pObj->ySizeItems + pObj->Props.SpaceY);
  pRect->x1 = pRect->x0 + pObj->xSizeItems - 1;
  pRect->y1 = pRect->y0 + pObj->ySizeItems - 1;
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(ICONVIEW_Handle hObj, WM_MESSAGE * pMsg) {
  ICONVIEW_OBJ * pObj;
  int NumItems, ColorIndex, ItemIndex, ItemsPerLine;
  GUI_RECT RectInside, RectItem, ClipRect, RectWin;
  ICONVIEW_ITEM * pItem;
  const void * pData;
  tDrawImage * pfDrawImage;
  int Sel, i;
  GUI_COLOR BkColor_BK, BkColor_SEL;
  GUI_COLOR aTextColor[3];
  GUI_ARRAY ItemArray;
  const GUI_FONT GUI_UNI_PTR * pFont;
  WM_SCROLL_STATE ScrollStateV;

  /* Get object properties */
  pObj = ICONVIEW_LOCK_H(hObj);
  Sel          = pObj->Sel;
  BkColor_BK   = pObj->Props.aBkColor[ICONVIEW_CI_BK];
  BkColor_SEL  = pObj->Props.aBkColor[ICONVIEW_CI_SEL];
  ItemArray    = pObj->ItemArray;
  RectWin      = pObj->Widget.Win.Rect;
  pFont        = pObj->Props.pFont;
  ScrollStateV = pObj->ScrollStateV;
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
    aTextColor[i] = pObj->Props.aTextColor[i];
  }
  GUI_UNLOCK_H(pObj);


  ClipRect = *(const GUI_RECT*)pMsg->Data.p;
  GUI_MoveRect(&ClipRect, -RectWin.x0, -RectWin.y0);
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  GUI__IntersectRect(&ClipRect, &RectInside);
  #if WM_SUPPORT_TRANSPARENCY
    if (WM_GetHasTrans(hObj) == 0) {
      GUI_SetBkColor(BkColor_BK);
      GUI_ClearRect(ClipRect.x0, ClipRect.y0, ClipRect.x1, ClipRect.y1);
    }
  #endif

  NumItems = GUI_ARRAY_GetNumItems(ItemArray);
  ItemsPerLine = _GetNumItemsPerLine(hObj);
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetFont(pFont);
  ItemIndex = ScrollStateV.v * ItemsPerLine;
  while (ItemIndex < NumItems) {
    _GetRelRectFromIndex(hObj, &RectItem, ItemIndex);
    if (RectItem.y0 > ClipRect.y1) {
      break;
    }
    if ((RectItem.y1 >= ClipRect.y0) && 
        (RectItem.y0 <= ClipRect.y1) && 
        (RectItem.x1 >= ClipRect.x0) && 
        (RectItem.x0 <= ClipRect.x1)) {
      /* Manage background of selected image */
      if (WM_GetFocussedWindow() == hObj) {
        if (Sel == ItemIndex) {
          if (BkColor_SEL & 0xFF000000) {
            _LightRect(&RectItem, BkColor_SEL);
          } else {
            GUI_SetColor(BkColor_SEL);
            GUI_FillRect(RectItem.x0, RectItem.y0, RectItem.x1, RectItem.y1);
          }
        }
      }
      /* Get pointer to item */
      pItem = (ICONVIEW_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
      /* Draw image */
      if (pItem->pfDrawImage) {
        pData = pItem->pData;
        pfDrawImage = pItem->pfDrawImage;
        GUI_UNLOCK_H(pItem);
        pfDrawImage(&RectItem, pData);
        pItem = (ICONVIEW_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
      }
      /* Set right color for drawing the text */
      if (WM__IsEnabled(hObj)) {
        if (Sel == ItemIndex) {
          ColorIndex = ICONVIEW_CI_SEL;
        } else {
          ColorIndex = ICONVIEW_CI_UNSEL;
        }
      } else {
        ColorIndex = ICONVIEW_CI_DISABLED;
      }
      GUI_SetColor(aTextColor[ColorIndex]);
      /* Draw image text */
      if (pItem->pfDrawText) {
        pObj = ICONVIEW_LOCK_H(hObj);
        pItem->pfDrawText(pObj, &RectItem, pItem->acText);
        GUI_UNLOCK_H(pObj);
      }
      GUI_UNLOCK_H(pItem);
    }
    ItemIndex++;
  }
}

/*********************************************************************
*
*       _ManageAutoScroll
*/
static void _ManageAutoScroll(ICONVIEW_Handle hObj) {
  ICONVIEW_OBJ * pObj;
  WM_SCROLL_STATE ScrollStateV;
  GUI_RECT Rect;
  int IsRequiredV;
  int NumLines;
  int NumVisLines;
  U16 Flags;
  static int IsActive;
  if (IsActive == 0) {
    IsActive = 1;
    pObj = ICONVIEW_LOCK_H(hObj);
    NumLines    = _GetNumLines(hObj, pObj);
    NumVisLines = _GetNumVisLines(hObj, pObj);
    ScrollStateV = pObj->ScrollStateV;
    Flags = pObj->Flags;
    GUI_UNLOCK_H(pObj);
    WM_GetInsideRectExScrollbar(hObj, &Rect);
    /*
    * Manage vertical scroll bar
    */
    IsRequiredV = (NumLines > NumVisLines) ? 1 : 0;
    if (IsRequiredV) {
      ScrollStateV.NumItems = NumLines;
      ScrollStateV.PageSize = NumVisLines;
    } else {
      ScrollStateV.v = 0;
    }
    if (Flags & ICONVIEW_SF_AUTOSCROLLBAR_V) {
      WM_SetScrollbarV(hObj, IsRequiredV);
    } else {
      WM_HWIN hWin;
      hWin = WM_GetScrollbarV(hObj);
      if (hWin) {
        if (IsRequiredV == 0) {
          ScrollStateV.NumItems = 1;
          ScrollStateV.PageSize = 1;
          IsRequiredV                 = 1;
        }
      }
    }
    /*
    * Set scroll state
    */
    if (IsRequiredV) {
      WM_HWIN hScroll;
      hScroll = WM_GetDialogItem(hObj, GUI_ID_VSCROLL);
      WM_SetScrollState(hScroll, &ScrollStateV);
    }
    pObj = ICONVIEW_LOCK_H(hObj);
    pObj->ScrollStateV = ScrollStateV;
    GUI_UNLOCK_H(pObj);
  }
  IsActive = 0;
}

/*********************************************************************
*
*       _UpdateScrollPos
*/
static int _UpdateScrollPos(ICONVIEW_Handle hObj, ICONVIEW_OBJ * pObj) {
  WM_HWIN hScroll;
  int ItemsPerLine;
  int CurrentLine;
  int NumVisLines;
  ItemsPerLine = _GetNumItemsPerLine(hObj);
  CurrentLine = pObj->Sel / ItemsPerLine;
  NumVisLines = _GetNumVisLines(hObj, pObj);
  if (CurrentLine >= (pObj->ScrollStateV.v + NumVisLines)) {
    pObj->ScrollStateV.v = CurrentLine - NumVisLines + 1;
    hScroll = WM_GetDialogItem(hObj, GUI_ID_VSCROLL);
    WM_SetScrollState(hScroll, &pObj->ScrollStateV);
    WM_InvalidateWindow(hObj);
  } else if (pObj->ScrollStateV.v > CurrentLine) {
    pObj->ScrollStateV.v = CurrentLine;
    hScroll = WM_GetDialogItem(hObj, GUI_ID_VSCROLL);
    WM_SetScrollState(hScroll, &pObj->ScrollStateV);
    WM_InvalidateWindow(hObj);
  }
  return 0;
}

/*********************************************************************
*
*       _SetSel
*/
static void _SetSel(ICONVIEW_Handle hObj, int Sel) {
  ICONVIEW_OBJ * pObj;
  GUI_RECT Rect;
  int SelOld;
  pObj = ICONVIEW_LOCK_H(hObj);
  SelOld = pObj->Sel;
  if (pObj->Sel != Sel) {
    pObj->Sel = Sel;
    _GetRelRectFromIndex(hObj, &Rect, SelOld);
    WM_InvalidateRect(hObj, &Rect);
    _GetRelRectFromIndex(hObj, &Rect, pObj->Sel);
    WM_InvalidateRect(hObj, &Rect);
    _UpdateScrollPos(hObj, pObj);
    GUI_UNLOCK_H(pObj);
    WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _OnKey
*/
static void _OnKey(ICONVIEW_Handle hObj, WM_MESSAGE * pMsg) {
  ICONVIEW_OBJ * pObj;
  const WM_KEY_INFO  * pInfo;
  int SelOld, SelNew, Key, NumItems, ItemsPerLine;
  GUI_RECT RectInside;
  pInfo = (const WM_KEY_INFO *)pMsg->Data.p;
  if (pInfo->PressedCnt == 0) {
    return;
  }
  pObj = ICONVIEW_LOCK_H(hObj);
  WM_GetInsideRectExScrollbar(hObj, &RectInside);
  ItemsPerLine = (RectInside.x1 - RectInside.x0 + 1 + pObj->Props.SpaceX) / (pObj->xSizeItems + pObj->Props.SpaceX);
  NumItems = GUI_ARRAY_GetNumItems(pObj->ItemArray);
  Key   = pInfo->Key;
  SelOld = SelNew = pObj->Sel;
  GUI_UNLOCK_H(pObj);
  if (SelOld < 0) {
    SelOld = 0;
  }
  switch (Key) {
  case GUI_KEY_RIGHT:
    if (SelOld < (NumItems - 1)) {
      SelNew = SelOld + 1;
    }
    break;
  case GUI_KEY_LEFT:
    if (SelOld > 0) {
      SelNew = SelOld - 1;
    }
    break;
  case GUI_KEY_DOWN:
    if ((SelOld + ItemsPerLine) < NumItems) {
      SelNew = SelOld + ItemsPerLine;
    }
    break;
  case GUI_KEY_UP:
    if (SelOld >= ItemsPerLine) {
      SelNew = SelOld - ItemsPerLine;
    }
    break;
  case GUI_KEY_HOME:
    SelNew = 0;
    break;
  case GUI_KEY_END:
    SelNew = NumItems - 1;
    break;
  default:
    WM_DefaultProc(pMsg);
  }
  if (SelOld != SelNew) {
    _SetSel(hObj, SelNew);
  }
}

/*********************************************************************
*
*       _OnDelete
*/
static void _OnDelete(ICONVIEW_Handle hObj) {
  ICONVIEW_OBJ * pObj;
  pObj = ICONVIEW_LOCK_H(hObj);
  GUI_ARRAY_Delete(pObj->ItemArray);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _GetIndexFromPos
*/
static int _GetIndexFromPos(ICONVIEW_Handle hObj, int xPos, int yPos) {
  ICONVIEW_OBJ * pObj;
  int Row, Col, Skip = 0;
  int NumItemsPerLine, Index, NumItems;
  pObj = ICONVIEW_LOCK_H(hObj);
  NumItems = GUI_ARRAY_GetNumItems(pObj->ItemArray);
  NumItemsPerLine = _GetNumItemsPerLine(hObj);
  Row = (yPos - pObj->Props.FrameY) / (pObj->ySizeItems + pObj->Props.SpaceY);
  Col = (xPos - pObj->Props.FrameX) / (pObj->xSizeItems + pObj->Props.SpaceX);
  if (Col > NumItemsPerLine) {
    Skip = 1;
  }
  if (Skip == 0) {
    Index = (Row + pObj->ScrollStateV.v) * NumItemsPerLine + Col;
    if (Index > NumItems) {
      Index = -1;
    }
  } else {
    Index = -1;
  }
  GUI_UNLOCK_H(pObj);
  return Index;
}

/*********************************************************************
*
*       _OnPidStateChange
*/
static void _OnPidStateChange(ICONVIEW_Handle hObj, WM_MESSAGE * pMsg) {
  int Index;
  const WM_PID_STATE_CHANGED_INFO * pState;
  pState = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  if ((pState->StatePrev == 0) && (pState->State == 1)) {
    Index = _GetIndexFromPos(hObj, pState->x, pState->y);
    if (Index >= 0) {
      _SetSel(hObj, Index);
    }
    WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
  } else if ((pState->StatePrev == 1) && (pState->State == 0)) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
  }
}

/*********************************************************************
*
*       _OnTouch
*/
static void _OnTouch(ICONVIEW_Handle hObj, WM_MESSAGE * pMsg) {
  if (pMsg->Data.p == NULL) {
    WM_NotifyParent(hObj, WM_NOTIFICATION_MOVED_OUT);
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
*       ICONVIEW__DrawBitmap
*/
static void ICONVIEW__DrawBitmap(GUI_RECT * pRect, const void * pData) {
  const GUI_BITMAP * pBitmap;
  pBitmap = (const GUI_BITMAP *)pData;
  GUI_DrawBitmap(pBitmap, pRect->x0 + (pRect->x1 - pRect->x0 + 1 - pBitmap->XSize) / 2, pRect->y0);
}

/*********************************************************************
*
*       ICONVIEW__DrawText
*/
static void ICONVIEW__DrawText(ICONVIEW_OBJ * pObj, GUI_RECT * pRect, const char * s) {
  GUI_DispStringInRect(s, pRect, pObj->Props.TextAlign);
}

/*********************************************************************
*
*       Public routines
*
**********************************************************************
*/
/*********************************************************************
*
*       ICONVIEW_Callback
*/
void ICONVIEW_Callback(WM_MESSAGE * pMsg) {
  ICONVIEW_Handle hObj;
  ICONVIEW_OBJ  * pObj;
  WM_SCROLL_STATE ScrollState;
  hObj = pMsg->hWin;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_DELETE:
    _OnDelete(hObj);
    break;
  case WM_PAINT:
    _OnPaint(hObj, pMsg);
    break;
  case WM_TOUCH:
    _OnTouch(hObj, pMsg);
    break;
  case WM_PID_STATE_CHANGED:
    _OnPidStateChange(hObj, pMsg);
    break;
  case WM_KEY:
    _OnKey(hObj, pMsg);
    break;
  case WM_NOTIFY_PARENT:
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_VALUE_CHANGED:
      pObj = ICONVIEW_LOCK_H(hObj);
      if (pMsg->hWinSrc  == WM_GetScrollbarV(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj->ScrollStateV.v = ScrollState.v;
      } else if (pMsg->hWinSrc == WM_GetScrollbarH(hObj)) {
        WM_GetScrollState(pMsg->hWinSrc, &ScrollState);
        pObj->ScrollStateH.v = ScrollState.v;
      }
      GUI_UNLOCK_H(pObj);
      WM_InvalidateWindow(hObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      break;
    case WM_NOTIFICATION_SCROLLBAR_ADDED:
      #if WIDGET_USE_PARENT_EFFECT
        pObj = ICONVIEW_LOCK_H(hObj);
        WIDGET_SetEffect(pMsg->hWinSrc, pObj->Widget.pEffect);
        GUI_UNLOCK_H(pObj);
      #endif
      _ManageAutoScroll(hObj);
      break;
    }
    break;
  }
}

/*********************************************************************
*
*       ICONVIEW_CreateEx
*/
ICONVIEW_Handle ICONVIEW_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id, int xSizeItems, int ySizeItems) {
  ICONVIEW_Handle hObj;
  ICONVIEW_OBJ * pObj;
  GUI_ARRAY ItemArray;
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, ysize, hParent, WinFlags, ICONVIEW_Callback,
                                sizeof(ICONVIEW_OBJ) - sizeof(WM_Obj));
  if (hObj) {
    ItemArray = GUI_ARRAY_Create();
    pObj = (ICONVIEW_OBJ *)GUI_LOCK_H(hObj);
    /* Create array of items */
    pObj->ItemArray = ItemArray;
    /* Init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    pObj->Widget.pEffect = &WIDGET_Effect_None;
    /* Init member variables */
    ICONVIEW_INIT_ID(pObj);
    pObj->Sel        = -1;
    pObj->Props      = ICONVIEW__DefaultProps;
    pObj->xSizeItems = xSizeItems;
    pObj->ySizeItems = ySizeItems;
    pObj->Flags      = ExFlags;
    GUI_UNLOCK_H(pObj);
    _SetSel(hObj, 0);
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       ICONVIEW_AddBitmapItem
*/
int ICONVIEW_AddBitmapItem(ICONVIEW_Handle hObj, const GUI_BITMAP * pBitmap, const char * pText) {
  ICONVIEW_OBJ  * pObj;
  ICONVIEW_ITEM   Item;
  ICONVIEW_ITEM * pItem;
  GUI_ARRAY ItemArray;
  int Index, Len;
  int r = 1;
  if (hObj) {
    WM_LOCK();
    Item.pfDrawImage = ICONVIEW__DrawBitmap;
    Item.pfDrawText  = ICONVIEW__DrawText;
    Item.pData       = pBitmap;
    Item.SizeofData  = 0; /* Not used for internal bitmap format */
    pObj = ICONVIEW_LOCK_H(hObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    Index = GUI_ARRAY_GetNumItems(ItemArray);
    Len = strlen(pText);
    if (GUI_ARRAY_AddItem(ItemArray, &Item, sizeof(ICONVIEW_ITEM) + Len + 1) == 0) {
      pItem = (ICONVIEW_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
      strcpy(pItem->acText, pText);
      GUI_UNLOCK_H(pItem);
      r = 0;
    }
    _ManageAutoScroll(hObj);
    WM_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       ICONVIEW_SetBkColor
*/
void ICONVIEW_SetBkColor(ICONVIEW_Handle hObj, int Index, GUI_COLOR Color) {
  ICONVIEW_OBJ  * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = ICONVIEW_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aBkColor)) {
      if (pObj->Props.aBkColor[Index] != Color) {
        pObj->Props.aBkColor[Index] = Color;
        WM_InvalidateWindow(hObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       ICONVIEW_SetTextColor
*/
void ICONVIEW_SetTextColor(ICONVIEW_Handle hObj, int Index, GUI_COLOR Color) {
  ICONVIEW_OBJ  * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = ICONVIEW_LOCK_H(hObj);
    if (Index < GUI_COUNTOF(pObj->Props.aTextColor)) {
      if (pObj->Props.aTextColor[Index] != Color) {
        pObj->Props.aTextColor[Index] = Color;
        WM_InvalidateWindow(hObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       ICONVIEW_SetFont
*/
void ICONVIEW_SetFont(ICONVIEW_Handle hObj, const GUI_FONT GUI_UNI_PTR * pFont) {
  ICONVIEW_OBJ  * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = ICONVIEW_LOCK_H(hObj);
    pObj->Props.pFont = pFont;
    GUI_UNLOCK_H(pObj);
    WM_InvalidateWindow(hObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       ICONVIEW_GetSel
*/
int ICONVIEW_GetSel(ICONVIEW_Handle hObj) {
  int Sel = 0;
  ICONVIEW_OBJ  * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = ICONVIEW_LOCK_H(hObj);
    Sel = pObj->Sel;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return Sel;
}

/*********************************************************************
*
*       ICONVIEW_SetSel
*/
void ICONVIEW_SetSel(ICONVIEW_Handle hObj, int Sel) {
  if (hObj) {
    WM_LOCK();
    _SetSel(hObj, Sel);
    WM_UNLOCK();
  }
}

#else                            /* Avoid problems with empty object modules */
  void ICONVIEW_C(void);
  void ICONVIEW_C(void) {}
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
