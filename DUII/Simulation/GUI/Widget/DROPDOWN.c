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
File        : DROPDOWN.c
Purpose     : Implementation of dropdown widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#include "GUI_ARRAY.h"
#include "DROPDOWN.h"
#include "DROPDOWN_Private.h"
#include "SCROLLBAR.h"
#include "WIDGET.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"
#include "WM_Intern.h"
#include "LISTBOX.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default fonts */
#ifndef   DROPDOWN_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define DROPDOWN_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define DROPDOWN_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define DROPDOWN_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define colors */
#ifndef   DROPDOWN_BKCOLOR0_DEFAULT
  #define DROPDOWN_BKCOLOR0_DEFAULT GUI_WHITE     /* Not selected */
#endif

#ifndef   DROPDOWN_BKCOLOR1_DEFAULT
  #define DROPDOWN_BKCOLOR1_DEFAULT GUI_GRAY      /* Selected, no focus */
#endif

#ifndef   DROPDOWN_BKCOLOR2_DEFAULT
  #define DROPDOWN_BKCOLOR2_DEFAULT GUI_BLUE      /* Selected, focus */
#endif

#ifndef   DROPDOWN_TEXTCOLOR0_DEFAULT
  #define DROPDOWN_TEXTCOLOR0_DEFAULT GUI_BLACK   /* Not selected */
#endif

#ifndef   DROPDOWN_TEXTCOLOR1_DEFAULT
  #define DROPDOWN_TEXTCOLOR1_DEFAULT GUI_WHITE   /* Selected, no focus */
#endif

#ifndef   DROPDOWN_TEXTCOLOR2_DEFAULT
  #define DROPDOWN_TEXTCOLOR2_DEFAULT GUI_WHITE   /* Selected, focus */
#endif

#ifndef   SCROLLBAR_COLOR_ARROW_DEFAULT
  #define SCROLLBAR_COLOR_ARROW_DEFAULT GUI_BLACK /* Arrow color */
#endif

#ifndef   SCROLLBAR_COLOR_BUTTON_DEFAULT
  #define SCROLLBAR_COLOR_BUTTON_DEFAULT 0xc0c0c0 /* Button color */
#endif

#ifndef   DROPDOWN_BORDER_DEFAULT
  #define DROPDOWN_BORDER_DEFAULT 2
#endif

#ifndef   DROPDOWN_ALIGN_DEFAULT
  #define DROPDOWN_ALIGN_DEFAULT GUI_TA_LEFT      /* Default text alignment */
#endif

#ifndef   DROPDOWN_KEY_EXPAND
  #define DROPDOWN_KEY_EXPAND GUI_KEY_SPACE
#endif

#ifndef   DROPDOWN_KEY_SELECT
  #define DROPDOWN_KEY_SELECT GUI_KEY_ENTER
#endif

#ifndef   DROPDOWN_DRAW_SKIN_DEFAULT
  #define DROPDOWN_DRAW_SKIN_DEFAULT NULL
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

DROPDOWN_PROPS DROPDOWN__DefaultProps = {
  DROPDOWN_FONT_DEFAULT,
  {
    DROPDOWN_BKCOLOR0_DEFAULT,
    DROPDOWN_BKCOLOR1_DEFAULT,
    DROPDOWN_BKCOLOR2_DEFAULT,
  },
  {
    DROPDOWN_TEXTCOLOR0_DEFAULT,
    DROPDOWN_TEXTCOLOR1_DEFAULT,
    DROPDOWN_TEXTCOLOR2_DEFAULT,
  },
  {
    SCROLLBAR_COLOR_ARROW_DEFAULT,
    SCROLLBAR_COLOR_BUTTON_DEFAULT,
  },
  {
    GUI_INVALID_COLOR,
    GUI_INVALID_COLOR,
    GUI_INVALID_COLOR,
  },
  {
    DROPDOWN_DRAW_SKIN_DEFAULT
  },
  DROPDOWN_BORDER_DEFAULT,
  DROPDOWN_ALIGN_DEFAULT
};

WIDGET_SKIN const * DROPDOWN__pSkinDefault = &DROPDOWN__SkinClassic;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN__GetNumItems

  Returns:
    Number of fully or partially visible items
*/
int DROPDOWN__GetNumItems(DROPDOWN_Obj * pObj) {
  int NumItems;
  NumItems = GUI_ARRAY_GetNumItems(pObj->Handles);
  return NumItems;
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
*       _SelectByKey
*/
static void _SelectByKey(DROPDOWN_Handle hObj, int Key) {
  int i;
  DROPDOWN_Obj * pObj;
  int NumItems;

  pObj = DROPDOWN_LOCK_H(hObj);
  NumItems = DROPDOWN__GetNumItems(pObj);
  GUI_UNLOCK_H(pObj);
  Key = _Tolower(Key);
  for (i = 0; i < NumItems; i++) {
    const char * s;
    char c;
    s = DROPDOWN__GetpItemLocked(hObj, i);
    c = _Tolower(*s);
    GUI_UNLOCK_H(s);
    if (c == Key) {
      DROPDOWN_SetSel(hObj, i);
      break;
    }
  }
}

/*********************************************************************
*
*       _FreeAttached
*/
static void _FreeAttached(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;
  pObj = DROPDOWN_LOCK_H(hObj);
  GUI_ARRAY_Delete(pObj->Handles);
  WM_DeleteWindow(pObj->hListWin);
  GUI_ALLOC_Free(pObj->hDisabled);
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;
  WIDGET_PAINT * pfPaint;

  pObj = DROPDOWN_LOCK_H(hObj);
  pfPaint = pObj->pWidgetSkin->pfPaint;
  GUI_UNLOCK_H(pObj);
  if (pfPaint) {
    pfPaint(hObj);
  }
}

/*********************************************************************
*
*       _OnTouch
*/
static int _OnTouch(DROPDOWN_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pMsg->Data.p) {  /* Something happened in our area (pressed or released) */
    if (pState->Pressed) {
      WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
    } else {
      WM_NotifyParent(hObj, WM_NOTIFICATION_RELEASED);
    }
  } else {     /* Mouse moved out */
    WM_NotifyParent(hObj, WM_NOTIFICATION_MOVED_OUT);
  }
  return 0; /* Message handled */
}

/*********************************************************************
*
*       Private routines
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN__GetpItemLocked
*
* Returns:
*   Pointer to the specified item in locked state. Note that caller
*   need to unlock the pointer after use!
*/
const char * DROPDOWN__GetpItemLocked(DROPDOWN_Handle hObj, int Index) {
  DROPDOWN_Obj * pObj;
  const char * s;
  WM_HMEM h;

  s = NULL;
  pObj = DROPDOWN_LOCK_H(hObj);
  h = GUI_ARRAY_GethItem(pObj->Handles, Index);
  GUI_UNLOCK_H(pObj);
  if (h) {
    s = (const char *)GUI_LOCK_H(h);
  }
  return s;
}

/*********************************************************************
*
*       DROPDOWN_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
DROPDOWN_Obj * DROPDOWN_LockH(DROPDOWN_Handle h) {
  DROPDOWN_Obj * p = (DROPDOWN_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != DROPDOWN_ID) {
      GUI_DEBUG_ERROROUT("DROPDOWN.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       DROPDOWN__AdjustHeight
*/
void DROPDOWN__AdjustHeight(DROPDOWN_Handle hObj) {
  int Height, xSize;
  DROPDOWN_Obj * pObj;
  
  pObj = DROPDOWN_LOCK_H(hObj);
  Height = pObj->TextHeight;
  if (!Height) {
    Height = GUI_GetYDistOfFont(pObj->Props.pFont);
  }
  Height += pObj->Widget.pEffect->EffectSize + 2 * pObj->Props.TextBorderSize;
  xSize = WM__GetWindowSizeX(&pObj->Widget.Win);
  GUI_UNLOCK_H(pObj);
  WM_SetSize(hObj, xSize, Height);
}

/*********************************************************************
*
*       Exported routines:  Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_Callback
*/
void DROPDOWN_Callback (WM_MESSAGE * pMsg) {
  DROPDOWN_Handle hObj;
  DROPDOWN_Obj * pObj;
  char IsExpandedBeforeMsg;
  WM_HWIN hListWin;

  hObj = pMsg->hWin;
  pObj = (DROPDOWN_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
  hListWin = pObj->hListWin;
  GUI_UNLOCK_H(pObj);
  IsExpandedBeforeMsg = hListWin ? 1 : 0;
  /* Let widget handle the standard messages */
  if (WIDGET_HandleActive(hObj, pMsg) == 0) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_NOTIFY_OWNER_KEY:
    /* Close the listbox if DROPDOWN_KEY_SELECT has been pressed */
    if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) {
      int Key;
      Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
      if (Key == DROPDOWN_KEY_SELECT) {
        int Sel;
        Sel = LISTBOX_GetSel(hListWin);
        if (Sel > 0) {
          DROPDOWN_SetSel(hObj, LISTBOX_GetSel(hListWin));
        }
        DROPDOWN_Collapse(hObj);
        WM_SetFocus(hObj);
      }
    }
    break;
  case WM_NOTIFY_PARENT:
    switch (pMsg->Data.v) {
    case WM_NOTIFICATION_SCROLL_CHANGED:
      WM_NotifyParent(hObj, WM_NOTIFICATION_SCROLL_CHANGED);
      break;
    case WM_NOTIFICATION_CLICKED:
      {
        int Sel;
        Sel = LISTBOX_GetSel(hListWin);
        if (Sel >= 0)  {
          DROPDOWN_SetSel(hObj, LISTBOX_GetSel(hListWin));
          WM_SetFocus(hObj);
        }
      }
      break;
    case LISTBOX_NOTIFICATION_LOST_FOCUS:
      DROPDOWN_Collapse(hObj);
      break;
    }
    break;
  case WM_PID_STATE_CHANGED:
    if (IsExpandedBeforeMsg == 0) {    /* Make sure we do not react a second time */
      const WM_PID_STATE_CHANGED_INFO * pInfo = (const WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
      if (pInfo->State) {
        DROPDOWN_Expand(hObj);
      }
    }
    break;
  case WM_TOUCH:
    if (_OnTouch(hObj, pMsg) == 0) {
      return;
    }
    break;
  case WM_PAINT:
    _Paint(hObj);
    break;
  case WM_DELETE:
    _FreeAttached(hObj);
    break;       /* No return here ... WM_DefaultProc needs to be called */
  case WM_KEY:
    if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt >0) {
      int Key = ((const WM_KEY_INFO *)(pMsg->Data.p))->Key;
      switch (Key) {
      case DROPDOWN_KEY_EXPAND:
        DROPDOWN_Expand(hObj);
        break;
      case GUI_KEY_BACKTAB:
        break;
      case GUI_KEY_TAB:
        if (WM_GetFocussedWindow() == hListWin) {
          WM_SetFocus(hObj);      /* Make sure, WM_SetFocusOnNextChild() works right when calling WM_DefaultProc() */
        }
        break;                    /* Send to parent by not doing anything */
      default:
        DROPDOWN_AddKey(hObj, Key);
        return;
      }
    }
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
/*********************************************************************
*
*       DROPDOWN_CreateEx
*/
DROPDOWN_Handle DROPDOWN_CreateEx(int x0, int y0, int xsize, int ysize, WM_HWIN hParent,
                                  int WinFlags, int ExFlags, int Id)
{
  DROPDOWN_Handle hObj;
  GUI_ARRAY Handles;
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xsize, -1, hParent, WinFlags, DROPDOWN_Callback,
                                sizeof(DROPDOWN_Obj) - sizeof(WM_Obj));
  if (hObj) {
    DROPDOWN_Obj * pObj;
    pObj = (DROPDOWN_Obj *)GUI_LOCK_H(hObj); /* Don't use use WIDGET_H2P because WIDGET_INIT_ID() has not be called at this point */
    /* init widget specific variables */
    WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
    pObj->Flags          = ExFlags;
    pObj->Props          = DROPDOWN__DefaultProps;
    pObj->pWidgetSkin = DROPDOWN__pSkinDefault;
    pObj->ScrollbarWidth = 0;
    DROPDOWN_INIT_ID(pObj);
    pObj->ySizeEx = ysize;
    DROPDOWN__AdjustHeight(hObj);
    /* Init sub-classes */
    GUI_UNLOCK_H(pObj);
    Handles = GUI_ARRAY_Create();
    pObj = DROPDOWN_LOCK_H(hObj);
    pObj->Handles = Handles;
    GUI_UNLOCK_H(pObj);
    DROPDOWN__pSkinDefault->pfCreate(hObj);
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
*       DROPDOWN_Collapse
*/
void DROPDOWN_Collapse(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    if (pObj->hListWin) {
      WM_DeleteWindow(pObj->hListWin);
      pObj->hListWin = 0;
      WM_InvalidateWindow(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       DROPDOWN_Expand
*/
void DROPDOWN_Expand(DROPDOWN_Handle hObj) {
  #if WIDGET_USE_PARENT_EFFECT
    const WIDGET_EFFECT * pEffect;
  #endif
  int xSize, ySize, i, NumItems;
  WM_HWIN hListWin;
  GUI_RECT r;
  GUI_RECT RectParent;
  WM_Obj * pParent;
  DROPDOWN_Obj * pObj;
  U8 ScrollbarWidth;
  U8 Flags;
  I16 Sel;
  GUI_ARRAY Handles;
  U8 * pDisabled;
  U16  nDisabled;
  U16  ItemSpacing;
  I16  Align;
  const GUI_FONT GUI_UNI_PTR * pFont;
  WM_HMEM hString;
  GUI_COLOR aBackColor[3];
  GUI_COLOR aTextColor[3];
  GUI_COLOR aScrollbarColor[3];
  WM_HMEM hDisabled;

  if (hObj == 0) {
    return;
  }
  WM_LOCK();
  //
  // Get object properties
  //
  pObj = DROPDOWN_LOCK_H(hObj);
  xSize          = WM_GetWindowSizeX(hObj);
  ySize          = pObj->ySizeEx;
  ScrollbarWidth = pObj->ScrollbarWidth;
  Handles        = pObj->Handles;
  Flags          = pObj->Flags;
  Sel            = pObj->Sel;
  NumItems       = DROPDOWN__GetNumItems(pObj);
  ItemSpacing    = pObj->ItemSpacing;
  Align          = pObj->Props.Align;
  pFont          = pObj->Props.pFont;
  r              = pObj->Widget.Win.Rect;
  hDisabled      = pObj->hDisabled;
  #if WIDGET_USE_PARENT_EFFECT
    pEffect      = pObj->Widget.pEffect;
  #endif
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aBackColor); i++) {
    aBackColor[i] = pObj->Props.aBackColor[i];
  }
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
    aTextColor[i] = pObj->Props.aTextColor[i];
  }
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aScrollbarColor); i++) {
    aScrollbarColor[i] = pObj->Props.aScrollbarColor[i];
  }
  GUI_UNLOCK_H(pObj);
  //
  // Get parent properties
  //
  pParent = (WM_Obj *)GUI_LOCK_H(WM_GetParent(hObj));
  RectParent = pParent->Rect;
  GUI_UNLOCK_H(pParent);
  //
  // Calculate position and size of listview
  //
  GUI_MoveRect(&r, -RectParent.x0, -RectParent.y0);
  if (Flags & DROPDOWN_CF_UP) {
    r.y0 -= ySize;
  } else {
    r.y0 = r.y1;
  }
  //
  // Create listview
  //
  hListWin = LISTBOX_CreateAsChild(NULL, WM_GetParent(hObj), r.x0, r.y0, xSize, ySize, WM_CF_SHOW);
  #if WIDGET_USE_PARENT_EFFECT
    WIDGET_SetEffect(hListWin, pEffect);
  #endif
  //
  // Set scrollbar behavior of listbox
  //
  if (Flags & DROPDOWN_SF_AUTOSCROLLBAR) {
    LISTBOX_SetScrollbarWidth(hListWin, ScrollbarWidth);
    LISTBOX_SetAutoScrollV(hListWin, 1);
  }
  //
  // Copy data to listbox
  //
  for (i = 0; i< NumItems; i++) {
    hString = GUI_ARRAY_GethItem(Handles, i);
    LISTBOX_AddStringH(hListWin, hString);
  }
  //
  // Set colors of listbox
  //
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aBackColor); i++) {
    LISTBOX_SetBkColor(hListWin, i, aBackColor[i]);
  }
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aTextColor); i++) {
    LISTBOX_SetTextColor(hListWin, i, aTextColor[i]);
  }
  for (i = 0; i < GUI_COUNTOF(pObj->Props.aScrollbarColor); i++) {
    LISTBOX_SetScrollbarColor(hListWin, i, aScrollbarColor[i]);
  }
  //
  // Set disabled state(s)
  //
  if (hDisabled) {
    pDisabled = (U8 *)GUI_LOCK_H(hDisabled);
    nDisabled = *(U16 *)pDisabled;
    pDisabled += sizeof(U16);
    for (i = 0; i < nDisabled; i++) {
      LISTBOX_SetItemDisabled(hListWin, i, *(pDisabled + (i >> 3)) & (1 << (i & 7)) ? 1 : 0);
    }
    GUI_UNLOCK_H(pDisabled);
  }
  //
  // Set remaining properties of listview
  //
  LISTBOX_SetItemSpacing(hListWin, ItemSpacing);
  LISTBOX_SetFont(hListWin, pFont);
  LISTBOX_SetTextAlign(hListWin, Align);
  WM_SetFocus(hListWin);
  LISTBOX_SetOwner(hListWin, hObj);
  LISTBOX_SetSel(hListWin, Sel);
  WM_NotifyParent(hObj, WM_NOTIFICATION_CLICKED);
  //
  // Remember listbox handle
  //
  pObj = DROPDOWN_LOCK_H(hObj);
  pObj->hListWin = hListWin;
  GUI_UNLOCK_H(pObj);
  //
  // Ready...
  //
  WM_UNLOCK();
}

/*********************************************************************
*
*       DROPDOWN_AddKey
*/
void DROPDOWN_AddKey(DROPDOWN_Handle hObj, int Key) {
  if (hObj) {
    WM_LOCK();
    switch (Key) {
      case GUI_KEY_DOWN:
        DROPDOWN_IncSel(hObj);
        break;
      case GUI_KEY_UP:
        DROPDOWN_DecSel(hObj);
        break;
      default:
        _SelectByKey(hObj, Key);
        break;
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       DROPDOWN_SetSel
*/
void DROPDOWN_SetSel(DROPDOWN_Handle hObj, int Sel) {
  int NumItems, MaxSel;
  DROPDOWN_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    NumItems = DROPDOWN__GetNumItems(pObj);
    MaxSel = NumItems ? NumItems-1 : 0;
    if (Sel > MaxSel) {
      Sel = MaxSel;
    }
    if (Sel != pObj->Sel) {
      pObj->Sel = Sel;
      GUI_UNLOCK_H(pObj);
      DROPDOWN_Invalidate(hObj);
      WM_NotifyParent(hObj, WM_NOTIFICATION_SEL_CHANGED);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       DROPDOWN_IncSel
*/
void DROPDOWN_IncSel(DROPDOWN_Handle hObj) {
  int Sel;
  Sel = DROPDOWN_GetSel(hObj);
  DROPDOWN_SetSel(hObj, Sel + 1);
}

/*********************************************************************
*
*       DROPDOWN_DecSel
*/
void DROPDOWN_DecSel(DROPDOWN_Handle hObj) {
  int Sel;
  Sel = DROPDOWN_GetSel(hObj);
  if (Sel) {
	  Sel--;
  }
  DROPDOWN_SetSel(hObj, Sel);
}

/*********************************************************************
*
*       DROPDOWN_GetSel
*/
int  DROPDOWN_GetSel (DROPDOWN_Handle hObj) {
  int r = 0;
  DROPDOWN_Obj * pObj;
  if (hObj) {
    WM_LOCK();
    pObj = DROPDOWN_LOCK_H(hObj);
    r = pObj->Sel;
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return r;
}

#else                            /* Avoid problems with empty object modules */
  void DROPDOWN_C(void) {}
#endif
