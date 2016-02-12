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
File        : MENU.c
Purpose     : Implementation of menu widget
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>
#include <string.h>

#define MENU_C       /* Required to generate intermodule data */

#include "MENU.h"
#include "MENU_Private.h"
#include "GUI_Debug.h"
#include "GUI_Private.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Private config defaults
*
**********************************************************************
*/

/* Define default font */
#ifndef MENU_FONT_DEFAULT
  #if   WIDGET_USE_SCHEME_SMALL
    #define MENU_FONT_DEFAULT &GUI_Font13_1
  #elif WIDGET_USE_SCHEME_MEDIUM
    #define MENU_FONT_DEFAULT &GUI_Font16_1
  #elif WIDGET_USE_SCHEME_LARGE
    #define MENU_FONT_DEFAULT &GUI_Font24_1
  #endif
#endif

/* Define default effect */
#ifndef MENU_EFFECT_DEFAULT
  #define MENU_EFFECT_DEFAULT       &WIDGET_Effect_3D1L
#endif

/* Define colors, index 0, enabled, not selected */
#ifndef MENU_TEXTCOLOR0_DEFAULT
  #define MENU_TEXTCOLOR0_DEFAULT   GUI_BLACK
#endif

#ifndef MENU_BKCOLOR0_DEFAULT
  #define MENU_BKCOLOR0_DEFAULT     GUI_LIGHTGRAY
#endif

/* Define colors, index 1, enabled, selected */
#ifndef MENU_TEXTCOLOR1_DEFAULT
  #define MENU_TEXTCOLOR1_DEFAULT   GUI_WHITE
#endif

#ifndef MENU_BKCOLOR1_DEFAULT
  #define MENU_BKCOLOR1_DEFAULT     0x980000
#endif

/* Define colors, index 2, disabled, not selected */
#ifndef MENU_TEXTCOLOR2_DEFAULT
  #define MENU_TEXTCOLOR2_DEFAULT   0x7C7C7C
#endif

#ifndef MENU_BKCOLOR2_DEFAULT
  #define MENU_BKCOLOR2_DEFAULT     GUI_LIGHTGRAY
#endif

/* Define colors, index 3, disabled, selected */
#ifndef MENU_TEXTCOLOR3_DEFAULT
  #define MENU_TEXTCOLOR3_DEFAULT   GUI_LIGHTGRAY
#endif

#ifndef MENU_BKCOLOR3_DEFAULT
  #define MENU_BKCOLOR3_DEFAULT     0x980000
#endif

/* Define colors, index 4, active submenu */
#ifndef MENU_TEXTCOLOR4_DEFAULT
  #define MENU_TEXTCOLOR4_DEFAULT   GUI_WHITE
#endif

#ifndef MENU_BKCOLOR4_DEFAULT
  #define MENU_BKCOLOR4_DEFAULT     0x7C7C7C
#endif

/* Define borders */
#ifndef MENU_BORDER_LEFT_DEFAULT
  #define MENU_BORDER_LEFT_DEFAULT    4
#endif

#ifndef MENU_BORDER_RIGHT_DEFAULT
  #define MENU_BORDER_RIGHT_DEFAULT   4
#endif

#ifndef MENU_BORDER_TOP_DEFAULT
  #define MENU_BORDER_TOP_DEFAULT     2
#endif

#ifndef MENU_BORDER_BOTTOM_DEFAULT
  #define MENU_BORDER_BOTTOM_DEFAULT  2
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/

MENU_PROPS MENU__DefaultProps = {
  {
    MENU_TEXTCOLOR0_DEFAULT,
    MENU_TEXTCOLOR1_DEFAULT,
    MENU_TEXTCOLOR2_DEFAULT,
    MENU_TEXTCOLOR3_DEFAULT,
    MENU_TEXTCOLOR4_DEFAULT,
  },
  {
    MENU_BKCOLOR0_DEFAULT,
    MENU_BKCOLOR1_DEFAULT,
    MENU_BKCOLOR2_DEFAULT,
    MENU_BKCOLOR3_DEFAULT,
    MENU_BKCOLOR4_DEFAULT,
  },
  {
    MENU_BORDER_LEFT_DEFAULT,
    MENU_BORDER_RIGHT_DEFAULT,
    MENU_BORDER_TOP_DEFAULT,
    MENU_BORDER_BOTTOM_DEFAULT,
  },
  MENU_FONT_DEFAULT
};

const WIDGET_EFFECT* MENU__pDefaultEffect = MENU_EFFECT_DEFAULT;

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _IsTopLevelMenu
*/
static char _IsTopLevelMenu(MENU_Handle hObj) {
  MENU_Obj * pObj;
  WM_HWIN     hOwner;
  pObj = MENU_LOCK_H(hObj);
  hOwner = pObj->hOwner;
  GUI_UNLOCK_H(pObj);
  if (MENU__SendMenuMessage(hObj, hOwner, MENU_IS_MENU, 0) == 0) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _GetTopLevelMenu
*
* Purpose:
*   The function returns the top level menu of the given menu.
*
* Parameter:
*   hObj, pObj                  : Obvious
*   phObjTopLevel, ppObjTopLevel: Pointer to handle and object pointer for the result
*   pSubSel                     : Pointer to an integer variable for returning the current 
*                                 selection of the last open sub menu.
*/
static void _GetTopLevelMenu(MENU_Handle hObj, MENU_Handle * phObjTopLevel, int * pSubSel) {
  MENU_Handle hObjTopLevel;
  MENU_Obj * pObjTopLevel;
  if (_IsTopLevelMenu(hObj)) {
    hObjTopLevel = hObj;
  } else {
    pObjTopLevel = MENU_LOCK_H(hObj);
    do {
      hObjTopLevel = pObjTopLevel->hOwner;
      GUI_UNLOCK_H(pObjTopLevel);
      pObjTopLevel = (MENU_Obj *)GUI_LOCK_H(hObjTopLevel);
      if (_IsTopLevelMenu(hObjTopLevel)) {
        break;
      } else {
        if (pSubSel) {
          *pSubSel = pObjTopLevel->Sel;
        }
      }
    } while (1);
    GUI_UNLOCK_H(pObjTopLevel);
  }
  *phObjTopLevel = hObjTopLevel;
}

/*********************************************************************
*
*       _HasEffect
*/
static int _HasEffect(MENU_Handle hObj, MENU_Obj * pObj) {
  if (!(pObj->Flags & MENU_SF_POPUP)) {
    if (_IsTopLevelMenu(hObj)) {
      return 0;
    }
  }
  return 1;
}

/*********************************************************************
*
*       _GetEffectSize
*/
static int _GetEffectSize(MENU_Handle hObj) {
  MENU_Obj * pObj;
  int r;
  pObj = MENU_LOCK_H(hObj);
  r = 0;
  if (_HasEffect(hObj, pObj)) {
    r = pObj->Widget.pEffect->EffectSize;
  }
  GUI_UNLOCK_H(pObj);
  return r;
}

/*********************************************************************
*
*       _CalcTextWidth
*/
static int _CalcTextWidth(MENU_Obj * pObj, const char GUI_UNI_PTR * sText) {
  int TextWidth = 0;
  if (sText) {
    const GUI_FONT GUI_UNI_PTR * pOldFont;
    pOldFont  = GUI_SetFont(pObj->Props.pFont);
    TextWidth = GUI_GetStringDistX(sText);
    GUI_SetFont(pOldFont);
  }
  return TextWidth;
}

/*********************************************************************
*
*       _GetItemWidth
*/
static int _GetItemWidth(MENU_Handle hObj, MENU_Obj * pObj, unsigned Index) {
  int ItemWidth;
  if (pObj->Width && (pObj->Flags & MENU_SF_VERTICAL)) {
    ItemWidth = pObj->Width - (_GetEffectSize(hObj) << 1);
  } else {
    MENU_ITEM * pItem;
    pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
    if ((pObj->Flags & MENU_SF_VERTICAL) || !(pItem->Flags & MENU_IF_SEPARATOR)) {
      ItemWidth = pItem->TextWidth;
    } else {
      ItemWidth = 3;
    }
    GUI_UNLOCK_H(pItem);
    ItemWidth += pObj->Props.aBorder[MENU_BI_LEFT] + pObj->Props.aBorder[MENU_BI_RIGHT];
  }
  return ItemWidth;
}

/*********************************************************************
*
*       _GetItemHeight
*/
static int _GetItemHeight(MENU_Handle hObj, MENU_Obj * pObj, unsigned Index) {
  int ItemHeight;
  if (pObj->Height && !(pObj->Flags & MENU_SF_VERTICAL)) {
    ItemHeight = pObj->Height - (_GetEffectSize(hObj) << 1);
  } else {
    ItemHeight = GUI_GetYDistOfFont(pObj->Props.pFont);
    if (pObj->Flags & MENU_SF_VERTICAL) {
      MENU_ITEM * pItem;
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
      if (pItem->Flags & MENU_IF_SEPARATOR) {
        ItemHeight = 3;
      }
      GUI_UNLOCK_H(pItem);
    }
    ItemHeight += pObj->Props.aBorder[MENU_BI_TOP] + pObj->Props.aBorder[MENU_BI_BOTTOM];
  }
  return ItemHeight;
}

/*********************************************************************
*
*       _CalcMenuSizeX
*/
static int _CalcMenuSizeX(MENU_Handle hObj) {
  MENU_Obj * pObj;
  unsigned i, NumItems;
  int xSize;
  
  pObj = MENU_LOCK_H(hObj);
  NumItems = MENU__GetNumItems(pObj);
  xSize = 0;
  if (pObj->Flags & MENU_SF_VERTICAL) {
    int ItemWidth;
    for (i = 0; i < NumItems; i++) {
      ItemWidth = _GetItemWidth(hObj, pObj, i);
      if (ItemWidth > xSize) {
        xSize = ItemWidth;
      }
    }
  } else {
    for (i = 0; i < NumItems; i++) {
      xSize += _GetItemWidth(hObj, pObj, i);
    }
  }
  xSize += (_GetEffectSize(hObj) << 1);
  GUI_UNLOCK_H(pObj);
  return xSize;
}

/*********************************************************************
*
*       _CalcMenuSizeY
*/
static int _CalcMenuSizeY(MENU_Handle hObj) {
  MENU_Obj * pObj;
  unsigned i, NumItems;
  int ySize;
  
  pObj = MENU_LOCK_H(hObj);
  NumItems = MENU__GetNumItems(pObj);
  ySize = 0;
  if (pObj->Flags & MENU_SF_VERTICAL) {
    for (i = 0; i < NumItems; i++) {
      ySize += _GetItemHeight(hObj, pObj, i);
    }
  } else {
    int ItemHeight;
    for (i = 0; i < NumItems; i++) {
      ItemHeight = _GetItemHeight(hObj, pObj, i);
      if (ItemHeight > ySize) {
        ySize = ItemHeight;
      }
    }
  }
  ySize += (_GetEffectSize(hObj) << 1);
  GUI_UNLOCK_H(pObj);
  return ySize;
}

/*********************************************************************
*
*       _CalcWindowSizeX
*/
static int _CalcWindowSizeX(MENU_Handle hObj, MENU_Obj * pObj) {
  int xSize = pObj->Width;
  if (xSize == 0) {
    xSize = _CalcMenuSizeX(hObj);
  }
  return xSize;
}

/*********************************************************************
*
*       _CalcWindowSizeY
*/
static int _CalcWindowSizeY(MENU_Handle hObj, MENU_Obj * pObj) {
  int ySize = pObj->Height;
  if (ySize == 0) {
    ySize = _CalcMenuSizeY(hObj);
  }
  return ySize;
}

/*********************************************************************
*
*       _GetItemFromPos
*
* Return value:
*   Zero based index of item at given position or -1.
*
* NOTE:
*   This function has to ensure that index is always less than the
*   maximum number of items.
*/
static int _GetItemFromPos(MENU_Handle hObj, int x, int y) {
  MENU_Obj * pObj;
  int xSize, ySize, EffectSize, r = -1;
  ySize = _CalcMenuSizeY(hObj);
  pObj = MENU_LOCK_H(hObj);
  if ((pObj->Height) && (pObj->Height < ySize)) {
    ySize = pObj->Height;
  }
  xSize = _CalcMenuSizeX(hObj);
  if ((pObj->Width) && (pObj->Width < xSize)) {
    xSize = pObj->Width;
  }
  EffectSize = _GetEffectSize(hObj);
  x     -= EffectSize;
  y     -= EffectSize;
  xSize -= (EffectSize << 1);
  ySize -= (EffectSize << 1);
  if ((x >= 0) && (y >= 0) && (x < xSize) && (y < ySize)) {
    unsigned i, NumItems = MENU__GetNumItems(pObj);
    if (pObj->Flags & MENU_SF_VERTICAL) {
      int yPos = 0;
      for (i = 0; i < NumItems; i++) {
        yPos += _GetItemHeight(hObj, pObj, i);
        if (y < yPos) {
          r = i;
          break;
        }
      }
    } else {
      int xPos = 0;
      for (i = 0; i < NumItems; i++) {
        xPos += _GetItemWidth(hObj, pObj, i);
        if (x < xPos) {
          r = i;
          break;
        }
      }
    }
  }
  GUI_UNLOCK_H(pObj);
  return r;
}

/*********************************************************************
*
*       _GetItemPos
*/
static void _GetItemPos(MENU_Handle hObj, unsigned Index, int* px, int* py) {
  MENU_Obj * pObj;
  int i, EffectSize;
  EffectSize = _GetEffectSize(hObj);
  pObj = MENU_LOCK_H(hObj);
  if (pObj->Flags & MENU_SF_VERTICAL) {
    int yPos = 0;
    for (i = 0; i < (int)Index; i++) {
      yPos += _GetItemHeight(hObj, pObj, i);
    }
    *px = EffectSize;
    *py = EffectSize + yPos;
  } else {
    int xPos = 0;
    for (i = 0; i < (int)Index; i++) {
      xPos += _GetItemWidth(hObj, pObj, i);
    }
    *px = EffectSize + xPos;
    *py = EffectSize;
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _SetCapture
*/
static void _SetCapture(MENU_Handle hObj) {
  MENU_Obj * pObj;
  char        IsSubmenuActive;
  pObj = MENU_LOCK_H(hObj);
  IsSubmenuActive = pObj->IsSubmenuActive;
  GUI_UNLOCK_H(pObj);
  if (IsSubmenuActive == 0) {
    if (WM_HasCaptured(hObj) == 0) {
      WM_SetCapture(hObj, 0);
    }
  }
}

/*********************************************************************
*
*       _ReleaseCapture
*/
static void _ReleaseCapture(MENU_Handle hObj) {
  MENU_Obj * pObj;
  U16         Flags;
  if (WM_HasCaptured(hObj)) {
    pObj = MENU_LOCK_H(hObj);
    Flags = pObj->Flags;
    GUI_UNLOCK_H(pObj);
    if (_IsTopLevelMenu(hObj) && !(Flags & MENU_SF_POPUP)) {
      WM_ReleaseCapture();
    }
  }
}

/*********************************************************************
*
*       _CloseSubmenu
*/
static void _CloseSubmenu(MENU_Handle hObj) {
  MENU_Obj * pObj;
  U16         Flags;
  GUI_ARRAY   ItemArray;
  int         Sel;
  char        IsSubmenuActive;
  pObj = MENU_LOCK_H(hObj);
  Flags           = pObj->Flags;
  ItemArray       = pObj->ItemArray;
  Sel             = pObj->Sel;
  IsSubmenuActive = pObj->IsSubmenuActive;
  GUI_UNLOCK_H(pObj);
  if (Flags & MENU_SF_ACTIVE) {
    if (IsSubmenuActive) {
      MENU_ITEM * pItem;
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Sel);
      /* Inform submenu about its deactivation and detach it */
      if (pItem) {
        MENU_Handle hSubmenu;
        hSubmenu = pItem->hSubmenu;
        GUI_UNLOCK_H(pItem);
        MENU__SendMenuMessage(hObj, hSubmenu, MENU_ON_CLOSE, 0);
        WM_DetachWindow(hSubmenu);
      } else {
        GUI_UNLOCK_H(pItem);
      }
      pObj = MENU_LOCK_H(hObj);
      pObj->IsSubmenuActive = 0;
      GUI_UNLOCK_H(pObj);
      /*
       * Keep capture in menu widget. The capture may only released
       * by clicking outside the menu or when mouse moved out.
       * And it may only released from a top level menu.
       */
      _SetCapture(hObj);
      /* Invalidate menu item. This is needed because the appearance may have changed */
      MENU__InvalidateItem(hObj, Sel);
    }
  }
}

/*********************************************************************
*
*       _OpenSubmenu
*/
static void _OpenSubmenu(MENU_Handle hObj, unsigned Index) {
  MENU_Obj * pObj;
  GUI_ARRAY   ItemArray;
  WM_HWIN     hOwner;
  U16         Flags;
  char PrevActiveSubmenu;
  pObj = MENU_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  hOwner    = pObj->hOwner;
  Flags     = pObj->Flags;
  PrevActiveSubmenu = pObj->IsSubmenuActive;
  GUI_UNLOCK_H(pObj);
  if (Flags & MENU_SF_ACTIVE) {
    MENU_ITEM * pItem;
    MENU_Handle Item_hSubmenu;
    U16         Item_Id;
    U16         Item_Flags;
    /* Close previous submenu (if needed) */
    _CloseSubmenu(hObj);
    pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
    Item_hSubmenu = pItem->hSubmenu;
    Item_Id       = pItem->Id;
    Item_Flags    = pItem->Flags;
    GUI_UNLOCK_H(pItem);
    if (Item_hSubmenu) {
      if ((Item_Flags & MENU_IF_DISABLED) == 0) {
        MENU_Handle hObjTopLevel; 
        int x, y, EffectSize;
        WM_HWIN hParent;

        _GetTopLevelMenu(hObj, &hObjTopLevel, 0);
        hParent = WM_GetParent(hObjTopLevel);

        /* Calculate position of submenu */
        EffectSize = _GetEffectSize(hObj);
        _GetItemPos(hObj, Index, &x, &y);
        if (Flags & MENU_SF_VERTICAL) {
          x += _CalcMenuSizeX(hObj) - (_GetEffectSize(hObj) << 1);
          y -= EffectSize;
        } else {
          y += _CalcMenuSizeY(hObj) - (_GetEffectSize(hObj) << 1);
          x -= EffectSize;
        }
        x += WM_GetWindowOrgX(hObjTopLevel) - WM_GetWindowOrgX(hParent);
        y += WM_GetWindowOrgY(hObjTopLevel) - WM_GetWindowOrgY(hParent);
        /*
         * Notify owner window when for the first time open a menu (when no
         * other submenu was open), so it can initialize the menu items.
         */
        if (PrevActiveSubmenu == 0) {
          if (_IsTopLevelMenu(hObj)) {
            MENU__SendMenuMessage(hObj, hOwner, MENU_ON_INITMENU, 0);
          }
        }
        /* Notify owner window when a submenu opens, so it can initialize the menu items. */
        MENU__SendMenuMessage(hObj, hOwner, MENU_ON_INITSUBMENU, Item_Id);
        /* Set active menu as owner of submenu. */
        MENU_SetOwner(Item_hSubmenu, hObj);
        /* Attach submenu and inform it about its activation. */
        WM_AttachWindowAt(Item_hSubmenu, /*WM_HBKWIN*/hParent, x, y);
        MENU__SendMenuMessage(hObj, Item_hSubmenu, MENU_ON_OPEN, 0);
        pObj = MENU_LOCK_H(hObj);
        pObj->IsSubmenuActive = 1;
        GUI_UNLOCK_H(pObj);
        /* Invalidate menu item. This is needed because the appearance may have changed. */
        MENU__InvalidateItem(hObj, Index);
      }
    }
  }
}

/*********************************************************************
*
*       _ClosePopup
*/
static void _ClosePopup(MENU_Handle hObj) {
  MENU_Obj * pObj;
  pObj = MENU_LOCK_H(hObj);
  if (pObj->Flags & MENU_SF_POPUP) {
    pObj->Flags &= ~(MENU_SF_POPUP);
    GUI_UNLOCK_H(pObj);
    WM_DetachWindow(hObj);
    WM_ReleaseCapture();
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _SetSelectionEx
*/
static void _SetSelectionEx(MENU_Handle hObj, int Index, int SuppressNotification) {
  MENU_Obj * pObj;
  char        IsSubmenuActive;
  GUI_ARRAY   ItemArray;
  WM_HWIN     hOwner;
  pObj = MENU_LOCK_H(hObj);
  if (Index != pObj->Sel) {
    IsSubmenuActive = pObj->IsSubmenuActive;
    ItemArray       = pObj->ItemArray;
    hOwner          = pObj->hOwner;
    MENU__InvalidateItem(hObj, pObj->Sel); /* Invalidate previous selection */
    pObj->Sel = Index;
    MENU__InvalidateItem(hObj, Index);     /* Invalidate new selection */
    GUI_UNLOCK_H(pObj);
    if ((IsSubmenuActive == 0) && (Index >= 0) && (SuppressNotification == 0)) {
      MENU_ITEM * pItem;
      U16         Id;
      U16         Flags;
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
      Id    = pItem->Id;
      Flags = pItem->Flags;
      GUI_UNLOCK_H(pItem);
      if ((Flags & MENU_IF_SEPARATOR) == 0) {
        MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMACTIVATE, Id);
      }
    }
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _SetSelection
*/
static void _SetSelection(MENU_Handle hObj, int Index) {
  _SetSelectionEx(hObj, Index, 0);
}

/*********************************************************************
*
*       _SelectItem
*/
static void _SelectItem(MENU_Handle hObj, unsigned Index) {
  MENU_Obj * pObj;
  unsigned Sel;
  pObj = MENU_LOCK_H(hObj);
  Sel = pObj->Sel;
  GUI_UNLOCK_H(pObj);
  if (Sel != Index) {
    _SetCapture(hObj);
    _OpenSubmenu(hObj, Index);
    _SetSelection(hObj, Index);
  }
}

/*********************************************************************
*
*       _DeselectItem
*/
static void _DeselectItem(MENU_Handle hObj) {
  MENU_Obj * pObj;
  char        IsSubmenuActive;
  pObj = MENU_LOCK_H(hObj);
  IsSubmenuActive = pObj->IsSubmenuActive;
  GUI_UNLOCK_H(pObj);
  if (IsSubmenuActive == 0) {
    _SetSelection(hObj, -1);
    _ReleaseCapture(hObj);
  }
}

/*********************************************************************
*
*       _ActivateItem
*/
static void _ActivateItem(MENU_Handle hObj, unsigned Index) {
  MENU_Obj * pObj;
  MENU_ITEM * pItem;
  MENU_Handle Item_hSubmenu;
  U16         Item_Flags;
  U16         Item_Id;
  GUI_ARRAY ItemArray;
  WM_HWIN     hOwner;
  pObj = MENU_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  hOwner = pObj->hOwner;
  GUI_UNLOCK_H(pObj);
  pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
  Item_hSubmenu = pItem->hSubmenu;
  Item_Flags    = pItem->Flags;
  Item_Id       = pItem->Id;
  GUI_UNLOCK_H(pItem);
  if (Item_hSubmenu == 0) {
    if ((Item_Flags & (MENU_IF_DISABLED | MENU_IF_SEPARATOR)) == 0) {
      _ClosePopup(hObj);
      /* Send item select message to owner. */
      MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMSELECT, Item_Id);
    }
  }
}

/*********************************************************************
*
*       _ActivateMenu
*/
static void _ActivateMenu(MENU_Handle hObj, unsigned Index) {
  MENU_Obj * pObj;
  int         Sel;
  U16         Flags;
  MENU_ITEM * pItem;
  MENU_Handle Item_hSubmenu;
  U16         Item_Flags;
  pObj = MENU_LOCK_H(hObj);
  pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
  Sel   = pObj->Sel;
  Flags = pObj->Flags;
  GUI_UNLOCK_H(pObj);
  Item_hSubmenu = pItem->hSubmenu;
  Item_Flags    = pItem->Flags;
  GUI_UNLOCK_H(pItem);
  if (Item_hSubmenu) {
    if ((Item_Flags & MENU_IF_DISABLED) == 0) {
      if ((Flags & MENU_SF_ACTIVE) == 0) {
        pObj = MENU_LOCK_H(hObj);
        pObj->Flags |= MENU_SF_ACTIVE;
        GUI_UNLOCK_H(pObj);
        _OpenSubmenu(hObj, Index);
        _SetSelection(hObj, Index);
      } else if (Flags & MENU_SF_CLOSE_ON_SECOND_CLICK) {
        if ((int)Index == Sel) {
          _CloseSubmenu(hObj);
          pObj = MENU_LOCK_H(hObj);
          pObj->Flags &= ~MENU_SF_ACTIVE;
          GUI_UNLOCK_H(pObj);
        }
      }
    }
  }
}

/*********************************************************************
*
*       _DeactivateMenu
*/
static void _DeactivateMenu(MENU_Handle hObj) {
  MENU_Obj * pObj;
  _CloseSubmenu(hObj);
  pObj = MENU_LOCK_H(hObj);
  if ((pObj->Flags & MENU_SF_OPEN_ON_POINTEROVER) == 0) {
    pObj->Flags &= ~MENU_SF_ACTIVE;
  }
  GUI_UNLOCK_H(pObj);
}

/*******************************************************************
*
*       _ForwardMouseOverMsg
*/
#if (GUI_SUPPORT_MOUSE)
static int _ForwardMouseOverMsg(MENU_Handle hObj, int x, int y) {
  MENU_Obj * pObj;
  char        IsSubmenuActive;
  U16         Flags;
  pObj = MENU_LOCK_H(hObj);
  IsSubmenuActive = pObj->IsSubmenuActive;
  Flags           = pObj->Flags;
  GUI_UNLOCK_H(pObj);
  if ((IsSubmenuActive == 0) && !(Flags & MENU_SF_POPUP)) {
    if (_IsTopLevelMenu(hObj)) {
      WM_HWIN hBelow;
      x += WM_GetWindowOrgX(hObj);
      y += WM_GetWindowOrgY(hObj);
      hBelow = WM_Screen2hWin(x, y);
      if (hBelow && (hBelow != hObj) && WM__IsInModalArea(hBelow)) {
        WM_MESSAGE Msg;
        GUI_PID_STATE State;
        x -= WM_GetWindowOrgX(hBelow);
        y -= WM_GetWindowOrgY(hBelow);
        State.Pressed = 0;
        State.x = x;
        State.y = y;
        Msg.Data.p = &State;
        Msg.MsgId = WM_MOUSEOVER;
        WM__SendMessage(hBelow, &Msg);
        return 1;
      }
    }
  }
  return 0;
}
#endif

/*********************************************************************
*
*       _XYInWidget
*
* Return values:
*   1 = Given position is inside of widget
*   0 = Given position is outside
*/
static int _XYInWidget(MENU_Obj * pObj, int x, int y) {
  int XYInWidget;
  XYInWidget = 0;
  if ((x >= 0) && (y >= 0)) {
    GUI_RECT r;
    WM__GetClientRectWin(&pObj->Widget.Win, &r);
    if ((x <= r.x1) && (y <= r.y1)) {
      XYInWidget = 1;
    }
  }
  return XYInWidget;
}

/*********************************************************************
*
*       _HandlePID
*
* Return values:
*   1 = We need to forward PID message to owner.
*   0 = We do not need to inform owner.
*/
static char _HandlePID(MENU_Handle hObj, int x, int y, int Pressed) {
  MENU_Obj * pObj;
  GUI_ARRAY   ItemArray;
  WM_HWIN     hOwner;
  U16 State;
  GUI_PID_STATE PrevState;
  int XYInWidget;
  WM_PID__GetPrevState(&PrevState, WM__TOUCHED_LAYER);
  /*
   * Check if coordinates are inside the widget.
   */
  pObj = MENU_LOCK_H(hObj);
  ItemArray  = pObj->ItemArray;
  hOwner     = pObj->hOwner;
  State      = pObj->Widget.State;
  XYInWidget = _XYInWidget(pObj, x, y);
  GUI_UNLOCK_H(pObj);
  if (XYInWidget) {
    int ItemIndex;
    ItemIndex = _GetItemFromPos(hObj, x, y);
    /* 
     * Handle PID when coordinates are inside the widget.
     */
    if (ItemIndex >= 0) {
      /* 
       * Coordinates are inside the menu.
       */
      if (Pressed > 0) {
        if ((PrevState.Pressed == 0) && (Pressed == 1)) {  /* Clicked */
          MENU_ITEM * pItem;
          U16         Flags;
          U16         Id;
          _ActivateMenu(hObj, ItemIndex);
          _SelectItem(hObj, ItemIndex);
          pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
          Flags = pItem->Flags;
          Id    = pItem->Id;
          GUI_UNLOCK_H(pItem);
          if ((Flags & MENU_IF_SEPARATOR) == 0) {
            MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMPRESSED, Id);
          }
        } else {
          _SelectItem(hObj, ItemIndex);
        }
      } else if ((Pressed == 0) && (PrevState.Pressed == 1)) {  /* Released */
        _ActivateItem(hObj, ItemIndex);
      } else if (Pressed < 0) {  /* Mouse moved */
        #if (GUI_SUPPORT_MOUSE)
          if (_ForwardMouseOverMsg(hObj, x, y) != 0) {
            _DeselectItem(hObj);
          } else
        #endif
        {
          MENU_ITEM * pItem;
          U16         Flags;
          U16         Id;
          _SelectItem(hObj, ItemIndex);
          pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, ItemIndex);
          Flags = pItem->Flags;
          Id    = pItem->Id;
          GUI_UNLOCK_H(pItem);
          if ((Flags & MENU_IF_SEPARATOR) == 0) {
            MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMACTIVATE, Id);
          }
        }
      }
    } else {
      /* 
       * Coordinates are outside the menu but inside the widget.
       */
      if (Pressed > 0) {
        if (PrevState.Pressed == 0) {  /* Clicked */
          /* 
           * User has clicked outside the menu. Close the active submenu.
           * The widget itself must be closed (if needed) by the owner.
           */
          _DeactivateMenu(hObj);
        }
        _DeselectItem(hObj);
      } else if (Pressed < 0) {  /* Moved out or mouse moved */
        if (!(State & WIDGET_STATE_FOCUS)) {
          _DeselectItem(hObj);
        }
      }
    }
    return 0;
  } else {
    /* 
     * Handle PID when coordinates are outside the widget.
     */
    if ((Pressed > 0) && (PrevState.Pressed == 0)) {
      /* 
       * User has clicked outside the menu. Close the active submenu.
       * The widget itself must be closed (if needed) by the owner.
       */
      _DeselectItem(hObj);
      _DeactivateMenu(hObj);
      _ClosePopup(hObj);
      _SetSelection(hObj, -1);
      if (State & WIDGET_STATE_FOCUS) {
        MENU_Handle hObjTopLevel; 
        _GetTopLevelMenu(hObj, &hObjTopLevel, 0);
        WM_SetFocus(hObjTopLevel);
        _SetSelection(hObjTopLevel, -1);
        _ReleaseCapture(hObjTopLevel);
      }
    }
    if (!(State & WIDGET_STATE_FOCUS)) {
      _DeselectItem(hObj);
    }
    #if (GUI_SUPPORT_MOUSE)
      _ForwardMouseOverMsg(hObj, x, y);
    #endif
  }
  return 1;   /* Coordinates are not in widget, we need to forward PID message to owner */
}

/*********************************************************************
*
*       _ForwardPIDMsgToOwner
*/
static int _ForwardPIDMsgToOwner(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  MENU_Obj * pObj;
  if (_IsTopLevelMenu(hObj) == 0) {
    WM_HWIN hOwner;
    pObj = MENU_LOCK_H(hObj);
    hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(hObj);
    GUI_UNLOCK_H(pObj);
    if (hOwner) {
      if (pMsg->Data.p) {
        GUI_PID_STATE * pState;
        pState = (GUI_PID_STATE *)pMsg->Data.p;
        pState->x += WM_GetWindowOrgX(hObj) - WM_GetWindowOrgX(hOwner);
        pState->y += WM_GetWindowOrgY(hObj) - WM_GetWindowOrgY(hOwner);
      }
      WM__SendMessage(hOwner, pMsg);
      return 0; /* Message has been forwarded */
    }
  }
  return 1; /* Message has not been forwarded */
}

/*********************************************************************
*
*       _ForwardPIDStateChange
*/
static void _ForwardPIDStateChange(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  MENU_Obj * pObj;
  WM_HWIN     hOwner;
  WM_HWIN hWin;
  WM_PID_STATE_CHANGED_INFO * pState;
  int XYInWidget, x, y;
  pObj = MENU_LOCK_H(hObj);
  hOwner     = pObj->hOwner;
  pState     = (WM_PID_STATE_CHANGED_INFO *)pMsg->Data.p;
  x          = pState->x;
  y          = pState->y;
  XYInWidget = _XYInWidget(pObj, x, y);
  GUI_UNLOCK_H(pObj);
  if ((XYInWidget) || (pState->State == 0)) {
    return;
  }
  x += WM_GetWindowOrgX(hObj);
  y += WM_GetWindowOrgY(hObj);
  if (_IsTopLevelMenu(hObj)) {
    hWin = WM_Screen2hWin(x, y);
    WM_ReleaseCapture();
  } else {
    hWin = hOwner ? hOwner : WM_GetParent(hObj);
    x -= WM_GetWindowOrgX(hWin);
    y -= WM_GetWindowOrgY(hWin);
    if (WM_HasFocus(hObj)) {
      WM_SetFocus(hWin);
    }
  }
  if (hWin) {
    pState->x = x;
    pState->y = y;
    pMsg->hWin = hWin;
    WM__SendMessage(hWin, pMsg);
    WM__SetLastTouched(hWin);
  }
}

/*********************************************************************
*
*       _ForwardPIDMsg
*/
static void _ForwardPIDMsg(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  MENU_Obj * pObj;
  WM_HWIN hWin;
  GUI_PID_STATE * pState;
  int XYInWidget, x, y;
  pState = (GUI_PID_STATE *)pMsg->Data.p;
  if (pState == NULL) {
    return;
  }
  x = pState->x;
  y = pState->y;
  pObj = MENU_LOCK_H(hObj);
  XYInWidget = _XYInWidget(pObj, x, y);
  GUI_UNLOCK_H(pObj);
  if (XYInWidget) {
    return;
  }
  x += WM_GetWindowOrgX(hObj);
  y += WM_GetWindowOrgY(hObj);
  hWin = WM_Screen2hWin(x, y);
  if (hWin) {
    x -= WM_GetWindowOrgX(hWin);
    y -= WM_GetWindowOrgY(hWin);
    pState->x = x;
    pState->y = y;
    pMsg->hWin = hWin;
    WM_ReleaseCapture();
    WM__SendMessage(hWin, pMsg);
    WM__SetLastTouched(hWin);
  }
}

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _OnMenu
*/
static void _OnMenu(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  MENU_Obj * pObj;
  WM_HWIN     hOwner;
  const MENU_MSG_DATA * pData;
  pData = (const MENU_MSG_DATA *)pMsg->Data.p;
  pObj = MENU_LOCK_H(hObj);
  hOwner = pObj->hOwner;
  GUI_UNLOCK_H(pObj);
  if (pData) {
    switch (pData->MsgType) {
    case MENU_ON_ITEMSELECT:
      _DeactivateMenu(hObj);
      _DeselectItem(hObj);
      _ClosePopup(hObj);
      /* No break here. We need to forward message to owner. */
    case MENU_ON_INITMENU:
    case MENU_ON_INITSUBMENU:
    case MENU_ON_ITEMACTIVATE:
    case MENU_ON_ITEMPRESSED:
      /* Forward message to owner. */
      if (hOwner == 0) {
        hOwner = WM_GetParent(hObj);
      }
      if (hOwner) {
        pMsg->hWinSrc = hObj;
        WM__SendMessage(hOwner, pMsg);
      }
      break;
    case MENU_ON_OPEN:
      pObj = MENU_LOCK_H(hObj);
      pObj->Sel = -1;
      pObj->IsSubmenuActive = 0;
      pObj->Flags |= MENU_SF_ACTIVE | MENU_SF_OPEN_ON_POINTEROVER;
      GUI_UNLOCK_H(pObj);
      _SetCapture(hObj);
      MENU__ResizeMenu(hObj);
      break;
    case MENU_ON_CLOSE:
      _CloseSubmenu(hObj);
      break;
    case MENU_IS_MENU:
      pMsg->Data.v = 1;
      break;
    }
  }
}

/*********************************************************************
*
*       _OnTouch
*/
static char _OnTouch(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pState) {  /* Something happened in our area (pressed or released) */
    return _HandlePID(hObj, pState->x, pState->y, pState->Pressed);
  }
  return _HandlePID(hObj, -1, -1, -1); /* Moved out */
}

/*********************************************************************
*
*       _OnMouseOver
*/
#if (GUI_SUPPORT_MOUSE)
static char _OnMouseOver(MENU_Handle hObj, WM_MESSAGE * pMsg) {
  const GUI_PID_STATE * pState = (const GUI_PID_STATE *)pMsg->Data.p;
  if (pState) {
    return _HandlePID(hObj, pState->x, pState->y, -1);
  }
  return 0;
}
#endif

/*********************************************************************
*
*       _SetPaintColors
*/
static void _SetPaintColors(const MENU_Obj * pObj, const MENU_ITEM * pItem, int ItemIndex) {
  char Selected;
  unsigned ColorIndex;
  Selected = (ItemIndex == pObj->Sel) ? 1 : 0;
  if (pObj->IsSubmenuActive && Selected) {
    ColorIndex = MENU_CI_ACTIVE_SUBMENU;
  } else if (pItem->Flags & MENU_IF_SEPARATOR) {
    ColorIndex = MENU_CI_ENABLED;
  } else {
    ColorIndex = (Selected) ? MENU_CI_SELECTED : MENU_CI_ENABLED;
    if (pItem->Flags & MENU_IF_DISABLED) {
      if (pObj->Flags & MENU_CF_HIDE_DISABLED_SEL) {
        ColorIndex = MENU_CI_DISABLED;
      } else {
        ColorIndex += MENU_CI_DISABLED;
      }
    }
  }
  GUI_SetBkColor(pObj->Props.aBkColor[ColorIndex]);
  GUI_SetColor(pObj->Props.aTextColor[ColorIndex]);
}

/*********************************************************************
*
*       _OnPaint
*/
static void _OnPaint(MENU_Handle hObj) {
  MENU_Obj * pObj;
  GUI_RECT FillRect, TextRect;
  MENU_ITEM * pItem;
  unsigned TextWidth, NumItems, i;
  U8 BorderLeft;
  U8 BorderTop;
  int FontHeight;
  int EffectSize;

  pObj = MENU_LOCK_H(hObj);
  BorderLeft   = pObj->Props.aBorder[MENU_BI_LEFT];
  BorderTop    = pObj->Props.aBorder[MENU_BI_TOP];
  FontHeight  = GUI_GetYDistOfFont(pObj->Props.pFont);
  EffectSize  = _GetEffectSize(hObj);
  NumItems = MENU__GetNumItems(pObj);
  WM__GetClientRectWin(&pObj->Widget.Win, &FillRect);
  GUI__ReduceRect(&FillRect, &FillRect, EffectSize);
  GUI_SetFont(pObj->Props.pFont);
  if (pObj->Flags & MENU_SF_VERTICAL) {
    int ItemHeight, xSize;
    xSize = _CalcMenuSizeX(hObj);
    FillRect.x1 = xSize - EffectSize - 1;
    TextRect.x0 = FillRect.x0 + BorderLeft;
    for (i = 0; i < NumItems; i++) {
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, i);
      ItemHeight = _GetItemHeight(hObj, pObj, i);
      _SetPaintColors(pObj, pItem, i);
      FillRect.y1 = FillRect.y0 + ItemHeight - 1;
      if (pItem->Flags & MENU_IF_SEPARATOR) {
        GUI_ClearRectEx(&FillRect);
        GUI_SetColor(0x7C7C7C);
        GUI_DrawHLine(FillRect.y0 + BorderTop + 1, FillRect.x0 + 2, FillRect.x1 - 2);
      } else {
        TextWidth = pItem->TextWidth;
        TextRect.x1 = TextRect.x0 + TextWidth - 1;
        TextRect.y0 = FillRect.y0 + BorderTop;
        TextRect.y1 = TextRect.y0 + FontHeight - 1;
        WIDGET__FillStringInRect(pItem->acText, &FillRect, &TextRect, &TextRect);
      }
      FillRect.y0 += ItemHeight;
      GUI_UNLOCK_H(pItem);
    }
  } else {
    int ItemWidth, ySize;
    ySize = _CalcMenuSizeY(hObj);
    FillRect.y1 = ySize - EffectSize - 1;
    TextRect.y0 = FillRect.y0 + BorderTop;
    TextRect.y1 = TextRect.y0 + FontHeight - 1;
    for (i = 0; i < NumItems; i++) {
      pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, i);
      ItemWidth = _GetItemWidth(hObj, pObj, i);
      _SetPaintColors(pObj, pItem, i);
      FillRect.x1 = FillRect.x0 + ItemWidth - 1;
      if (pItem->Flags & MENU_IF_SEPARATOR) {
        GUI_ClearRectEx(&FillRect);
        GUI_SetColor(0x7C7C7C);
        GUI_DrawVLine(FillRect.x0 + BorderLeft + 1, FillRect.y0 + 2, FillRect.y1 - 2);
      } else {
        TextWidth = pItem->TextWidth;
        TextRect.x0 = FillRect.x0 + BorderLeft;
        TextRect.x1 = TextRect.x0 + TextWidth - 1;
        WIDGET__FillStringInRect(pItem->acText, &FillRect, &TextRect, &TextRect);
      }
      FillRect.x0 += ItemWidth;
      GUI_UNLOCK_H(pItem);
    }
  }
  if (pObj->Width || pObj->Height) {
    GUI_RECT r;
    WM__GetClientRectWin(&pObj->Widget.Win, &r);
    GUI__ReduceRect(&r, &r, EffectSize);
    GUI_SetBkColor(pObj->Props.aBkColor[MENU_CI_ENABLED]);
    GUI_ClearRect(FillRect.x1 + 1, EffectSize, r.x1, FillRect.y1);
    GUI_ClearRect(EffectSize, FillRect.y1 + 1, r.x1, r.y1);
  }
  /* Draw 3D effect (if configured) */
  if (_HasEffect(hObj, pObj)) {
    pObj->Widget.pEffect->pfDrawUp();
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _MoveSel
*
* Purpose:
*   Moves the selection of the given menu to the desired direction.
*   If the last or the first item is selected, the selection moves to the begin or the end.
*   Separators will be skipped.
*/
static void _MoveSel(MENU_Handle hObj, int Dir, int SuppressNotification) {
  MENU_Obj * pObj;
  GUI_ARRAY   ItemArray;
  int NewIndex, Index, NumItems, Cnt = 0;
  pObj = MENU_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  Index     = pObj->Sel;
  GUI_UNLOCK_H(pObj);
  NewIndex = -1;
  NumItems = GUI_ARRAY_GetNumItems(ItemArray);
  do {
    MENU_ITEM * pItem;
    Index += Dir;
    if (Index >= NumItems) {
      Index = 0;
      Cnt++;
    } else if (Index < 0) {
      Index = NumItems - 1;
      Cnt++;
    }
    pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
    if (!(pItem->Flags & MENU_IF_SEPARATOR)) {
      NewIndex = Index;
    }
    GUI_UNLOCK_H(pItem);
  } while ((NewIndex < 0) && (Cnt < 2));
  if (NewIndex >= 0) {
    _SetSelectionEx(hObj, Index, SuppressNotification);
  }
}

/*********************************************************************
*
*       _OpenMenu
*/
static void _OpenMenu(MENU_Handle hObj, int Index, int SubSel) {
  MENU_Obj * pObj;
  MENU_ITEM * pItem;
  int NumItems;
  GUI_ARRAY   ItemArray;
  MENU_Handle Item_hSubmenu;
  
  pObj = MENU_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  pObj->Flags |= MENU_SF_ACTIVE;
  GUI_UNLOCK_H(pObj);
  pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
  Item_hSubmenu = pItem->hSubmenu;
  GUI_UNLOCK_H(pItem);
  _OpenSubmenu(hObj, Index);
  NumItems = GUI_ARRAY_GetNumItems(ItemArray);
  if (SubSel >= NumItems) {
    SubSel = NumItems - 1;
  }
  _SetSelection(Item_hSubmenu, SubSel);
  WM_SetFocus(Item_hSubmenu);
  _SetSelection(hObj, Index);
}

/*********************************************************************
*
*       _OnKey
*/
static int _OnKey(MENU_Handle hObj, int Key) {
  MENU_Obj * pObj;
  int KeyOpen, KeyBack, KeyNext, KeyPrev, Index, SubSel = 0;
  MENU_ITEM * pItem;
  MENU_Handle hObjTopLevel;
  MENU_Obj * pObjTopLevel;
  GUI_ARRAY   ItemArray;
  WM_HWIN     hOwner;
  U16         Flags;
  MENU_Handle Item_hSubmenu;
  U16         Item_Id;
  U16         Item_Flags;
  U16         TopLevel_Flags;
  int         TopLevel_Sel;
  
  pObj = MENU_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  hOwner    = pObj->hOwner;
  Flags     = pObj->Flags;
  Index     = pObj->Sel;
  GUI_UNLOCK_H(pObj);

  _GetTopLevelMenu(hObj, &hObjTopLevel, &SubSel);
  pObjTopLevel = MENU_LOCK_H(hObjTopLevel);
  TopLevel_Flags = pObjTopLevel->Flags;
  GUI_UNLOCK_H(pObjTopLevel);
  
  if (Flags & MENU_CF_VERTICAL) {
    KeyOpen = GUI_KEY_RIGHT;
    KeyBack = GUI_KEY_LEFT;
    KeyNext = GUI_KEY_DOWN;
    KeyPrev = GUI_KEY_UP;
  } else {
    KeyOpen = GUI_KEY_DOWN;
    KeyBack = 0;
    KeyNext = GUI_KEY_RIGHT;
    KeyPrev = GUI_KEY_LEFT;
  }
  if (Index < 0) {
    if        (Key == KeyPrev) {
      int NumItems;
      NumItems = GUI_ARRAY_GetNumItems(ItemArray);
      _SetSelection(hObj, NumItems - 1);
    } else if (Key != GUI_KEY_ESCAPE) {
      _SetSelection(hObj, 0);
    }
    if (Key != GUI_KEY_ESCAPE) {
      _SetCapture(hObj);
    }
    return 0;
  }
  
  pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
  Item_hSubmenu = pItem->hSubmenu;
  Item_Flags    = pItem->Flags;
  Item_Id       = pItem->Id;
  GUI_UNLOCK_H(pItem);

  if (Item_hSubmenu && ((Key == KeyOpen) || (Key == GUI_KEY_ENTER))) {
    /*
     *  If the current menu item is a submenu and <ENTER> or KeyOpen has been pressed, open the submenu
     */
    _OpenMenu(hObj, Index, 0);
  } else if (!Item_hSubmenu && (Key == GUI_KEY_ENTER)) {
    /*
     *  If the current menu item is not a submenu and <ENTER> has been pressed,
     *  set the focus to the top level menu, close the submenus and send an MENU_ON_ITEMSELECT message to the owner
     */
    if ((Item_Flags & MENU_IF_DISABLED) == 0) {
      WM_SetFocus(hObjTopLevel);
      _ActivateItem(hObj, Index);
    }
    if ((Item_Flags & MENU_IF_SEPARATOR) == 0) {
      MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMPRESSED, Item_Id);
    }
  } else if (((hObjTopLevel != hObj) && !(TopLevel_Flags & MENU_CF_VERTICAL) && (Key == KeyOpen) && (!Item_hSubmenu)) ||
             ((hObjTopLevel != hObj) && !(TopLevel_Flags & MENU_CF_VERTICAL) && (Key == KeyBack) && (hObjTopLevel == hOwner))) {
    /*
     *  If the current menu is not the top level menu and the top level menu is horizontal
     *  and <GUI_KEY_RIGHT> or <GUI_KEY_LEFT> has been pressed close the current submenus,
     *  move the selection of the top level menu to next/previous item and open it
     */
    pObj = MENU_LOCK_H(hObj);
    pObj->Flags &= ~MENU_SF_ACTIVE;
    GUI_UNLOCK_H(pObj);
    /* Set focus to top level menu */
    WM_SetFocus(hObjTopLevel);
    /* Move selection of top level menu */
    if (Key == KeyOpen) {
      _MoveSel(hObjTopLevel, +1, 1);
      _GetTopLevelMenu(hObj, &hObjTopLevel, &SubSel); /* Pointer recalculation required after sending a notification message */
    } else {
      _MoveSel(hObjTopLevel, -1, 1);
      _GetTopLevelMenu(hObj, &hObjTopLevel, &SubSel); /* Pointer recalculation required after sending a notification message */
    }
    /* Open top level menu */
    pObjTopLevel = MENU_LOCK_H(hObjTopLevel);
    TopLevel_Sel   = pObjTopLevel->Sel;
    GUI_UNLOCK_H(pObjTopLevel);
    _OpenMenu(hObjTopLevel, TopLevel_Sel, (Key == KeyBack) ? SubSel : 0);
  } else if ((hObjTopLevel != hObj) && 
             ((Key == GUI_KEY_ESCAPE) || ((Key == KeyBack) && ((!(TopLevel_Flags & MENU_CF_VERTICAL)) && (!Item_hSubmenu) && (hObjTopLevel != hOwner)) || (TopLevel_Flags & MENU_CF_VERTICAL)))) {
    /*
     *  If the current menu is not the top level menu and the top level menu is vertical and <GUI_KEY_LEFT> has been pressed
     *  or the top level menu is vertical or horizontal and <GUI_KEY_ESCAPE> has been pressed,
     *  close the current submenu
     */
    WM_SetFocus(hOwner);
    if (hOwner) {
      MENU_Obj  * pObjOwner;
      MENU_ITEM * pItemOwner;
      U16         IdOwner;
      U16         FlagsOwner;
      pObjOwner = MENU_LOCK_H(hOwner);
      pItemOwner = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObjOwner->ItemArray, pObjOwner->Sel);
      GUI_UNLOCK_H(pObjOwner);
      IdOwner    = pItemOwner->Id;
      FlagsOwner = pItemOwner->Flags;
      GUI_UNLOCK_H(pItemOwner);
      if ((FlagsOwner & MENU_IF_SEPARATOR) == 0) {
        MENU__SendMenuMessage(hObj, hOwner, MENU_ON_ITEMACTIVATE, IdOwner);
      }
    }
  } else if ((hObjTopLevel == hObj) && (Key == GUI_KEY_ESCAPE) && (!(Flags & MENU_SF_POPUP))) {
    /*
     *  If the current menu is the top level menu and <GUI_KEY_ESCAPE> has been pressed,
     *  close submenu, unselect the menu and release capture
     */
    _CloseSubmenu(hObj);
    _SetSelection(hObj, -1);
    _ReleaseCapture(hObj);
  } else if ((hObjTopLevel == hObj) && (Key == GUI_KEY_ESCAPE) && (Flags & MENU_SF_POPUP)) {
    /*
     *  If the current menu is a top level popup menu and <GUI_KEY_ESCAPE> has been pressed,
     *  close the popup menu
     */
    _ClosePopup(hObj);
  } else if (Key == KeyNext) {
    /*
     *  If KeyNext has been pressed move the selection to the next menu item
     */
    _CloseSubmenu(hObj);
    _MoveSel(hObj, +1, 0);
  } else if (Key == KeyPrev) {
    /*
     *  If KeyPrev has been pressed move the selection to the previous menu item
     */
    _CloseSubmenu(hObj);
    _MoveSel(hObj, -1, 0);
  } else {
    return 1;
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
*       MENU_LockH
*/
#if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
MENU_Obj * MENU_LockH(MENU_Handle h) {
  MENU_Obj * p = (MENU_Obj *)GUI_LOCK_H(h);
  if (p) {
    if (p->DebugId != MENU_ID) {
      GUI_DEBUG_ERROROUT("MENU.c: Wrong handle type or Object not init'ed");
      return 0;
    }
  }
  return p;
}
#endif

/*********************************************************************
*
*       Public code, Callback
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU_Callback
*/
void MENU_Callback(WM_MESSAGE * pMsg) {
  MENU_Handle hObj;
  MENU_Obj * pObj;
  GUI_ARRAY   ItemArray;
  int         Sel;
  int OldWidgetState, Result, HasFocus;

  hObj = pMsg->hWin;
  pObj = (MENU_Obj *)GUI_LOCK_H(hObj);
  OldWidgetState = pObj->Widget.State;
  GUI_UNLOCK_H(pObj);
  Result = 1;
  if (pMsg->MsgId != WM_PID_STATE_CHANGED) {
    /* Let widget handle the standard messages */
    Result = WIDGET_HandleActive(hObj, pMsg);
  }
  pObj = (MENU_Obj *)GUI_LOCK_H(hObj);
  ItemArray = pObj->ItemArray;
  Sel       = pObj->Sel;
  HasFocus  = pObj->Widget.State & WIDGET_STATE_FOCUS;
  GUI_UNLOCK_H(pObj);
  /* React on focus change */
  if ((OldWidgetState & WIDGET_STATE_FOCUS) != HasFocus) {
    if (HasFocus) {
      _CloseSubmenu(hObj);
      pObj = MENU_LOCK_H(hObj);
      pObj->Flags &= ~MENU_SF_ACTIVE;
      GUI_UNLOCK_H(pObj);
      if (Sel <= 0) {
        _SetSelection(hObj, 0);
        _SetCapture(hObj);
      }
    } else {
      _ReleaseCapture(hObj);
    }
  }
  /* Return if message already has been processed */
  if (!Result) {
    return;
  }
  switch (pMsg->MsgId) {
  case WM_PID_STATE_CHANGED:
    _ForwardPIDStateChange(hObj, pMsg);
    break;
  case WM_MENU:
    _OnMenu(hObj, pMsg);
    return;     /* Message handled, do not call WM_DefaultProc() here. */
  case WM_TOUCH:
    #if GUI_NUM_LAYERS > 1
      WM__SelectTopLevelLayer(hObj);
    #endif
    if (_OnTouch(hObj, pMsg)) {
      if (_ForwardPIDMsgToOwner(hObj, pMsg)) {
        _ForwardPIDMsg(hObj, pMsg);
      }
    }
    break;
#if (GUI_SUPPORT_MOUSE)
  case WM_MOUSEOVER:
    if (_OnMouseOver(hObj, pMsg)) {
      _ForwardPIDMsgToOwner(hObj, pMsg);
    }
    break;
#endif
  case WM_KEY:
    if (((const WM_KEY_INFO *)(pMsg->Data.p))->PressedCnt > 0) {
      if (!_OnKey(hObj, ((const WM_KEY_INFO *)(pMsg->Data.p))->Key)) {
        return;
      }
    }
    break;
  case WM_PAINT:
    _OnPaint(hObj);
    break;
  case WM_DELETE:
    GUI_ARRAY_Delete(ItemArray);
    break;      /* No return here ... WM_DefaultProc needs to be called */
  }
  WM_DefaultProc(pMsg);
}

/*********************************************************************
*
*       Public code, Create
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU_CreateEx
*/
MENU_Handle MENU_CreateEx(int x0, int y0, int xSize, int ySize, WM_HWIN hParent, int WinFlags, int ExFlags, int Id) {
  MENU_Handle hObj;
  /* Create the window */
  WM_LOCK();
  hObj = WM_CreateWindowAsChild(x0, y0, xSize, ySize, hParent, WM_CF_SHOW | WM_CF_STAYONTOP | WinFlags, &MENU_Callback,
                                sizeof(MENU_Obj) - sizeof(WM_Obj));
  if (hObj) {
    MENU_Obj * pObj;
    GUI_ARRAY ItemArray;
    ItemArray = GUI_ARRAY_Create();
    if (ItemArray) {
      pObj = (MENU_Obj *)GUI_LOCK_H(hObj);
       /* Init sub-classes */
      pObj->ItemArray = ItemArray;
      /* init widget specific variables */
      WIDGET__Init(&pObj->Widget, Id, WIDGET_STATE_FOCUSSABLE);
      /* init member variables */
      MENU_INIT_ID(pObj);
      if (ExFlags & MENU_SF_OPEN_ON_POINTEROVER) {
        ExFlags |= MENU_SF_ACTIVE;
      } else {
        ExFlags &= ~(MENU_SF_ACTIVE);
      }
      pObj->Props           = MENU__DefaultProps;
      pObj->Flags           = ExFlags;
      pObj->Width           = ((xSize > 0) ? xSize : 0);
      pObj->Height          = ((ySize > 0) ? ySize : 0);
      pObj->Sel             = -1;
      pObj->hOwner          = 0;
      pObj->IsSubmenuActive = 0;
      GUI_UNLOCK_H(pObj);
      WIDGET_SetEffect(hObj, MENU__pDefaultEffect);
    } else {
      WM_DeleteWindow(hObj);
      hObj = 0;
      GUI_DEBUG_ERROROUT_IF(hObj==0, "MENU_CreateEx failed")
    }
  } else {
    GUI_DEBUG_ERROROUT_IF(hObj==0, "MENU_CreateEx failed")
  }
  WM_UNLOCK();
  return hObj;
}

/*********************************************************************
*
*       Public code, modul internal functions
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU__GetNumItems
*/
unsigned MENU__GetNumItems(MENU_Obj * pObj) {
  return GUI_ARRAY_GetNumItems(pObj->ItemArray);
}

/*********************************************************************
*
*       MENU__InvalidateItem
*/
void MENU__InvalidateItem(MENU_Handle hObj, unsigned Index) {
  GUI_USE_PARA(Index);
  WM_InvalidateWindow(hObj);  /* Can be optimized, no need to invalidate all items */
}

/*********************************************************************
*
*       MENU__RecalcTextWidthOfItems
*/
void MENU__RecalcTextWidthOfItems(MENU_Obj * pObj) {
  const GUI_FONT GUI_UNI_PTR * pOldFont;
  MENU_ITEM * pItem;
  unsigned i, NumItems;
  NumItems = MENU__GetNumItems(pObj);
  pOldFont  = GUI_SetFont(pObj->Props.pFont);
  for (i = 0; i < NumItems; i++) {
    pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, i);
    pItem->TextWidth = GUI_GetStringDistX(pItem->acText);
    GUI_UNLOCK_H(pItem);
  }
  GUI_SetFont(pOldFont);
}

/*********************************************************************
*
*       MENU__ResizeMenu
*/
void MENU__ResizeMenu(MENU_Handle hObj) {
  MENU_Obj * pObj;
  int xSize, ySize;
  pObj = MENU_LOCK_H(hObj);
  xSize = _CalcWindowSizeX(hObj, pObj);
  ySize = _CalcWindowSizeY(hObj, pObj);
  GUI_UNLOCK_H(pObj);
  WM_SetSize(hObj, xSize, ySize);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       MENU__SetItem
*/
char MENU__SetItem(MENU_Handle hObj, unsigned Index, const MENU_ITEM_DATA * pItemData) {
  MENU_Obj * pObj;
  GUI_ARRAY ItemArray;
  MENU_ITEM Item = {0};
  const char * pText;
  char r = 0;
  pText = pItemData->pText;
  if (!pText) {
    pText = "";
  }
  pObj = MENU_LOCK_H(hObj);
  Item.Id        = pItemData->Id;
  Item.Flags     = pItemData->Flags;
  Item.hSubmenu  = pItemData->hSubmenu;
  Item.TextWidth = _CalcTextWidth(pObj, pText);
  ItemArray      = pObj->ItemArray;
  GUI_UNLOCK_H(pObj);
  if (Item.Flags & MENU_IF_SEPARATOR) {
    Item.hSubmenu = 0;   /* Ensures that no separator is a submenu */
  }
  if (GUI_ARRAY_SetItem(ItemArray, Index, &Item, sizeof(MENU_ITEM) + strlen(pText)) != 0) {
    MENU_ITEM * pItem;
    pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(ItemArray, Index);
    strcpy(pItem->acText, pText);
    GUI_UNLOCK_H(pItem);
    MENU_SetOwner(Item.hSubmenu, hObj);
    WM_DetachWindow(Item.hSubmenu);   /* Ensures all submenus are initially closed */
    r = 1;
  }
  return r;
}

/*********************************************************************
*
*       MENU__SetItemFlags
*/
void MENU__SetItemFlags(MENU_Obj * pObj, unsigned Index, U16 Mask, U16 Flags) {
  MENU_ITEM * pItem = (MENU_ITEM *)GUI_ARRAY_GetpItemLocked(pObj->ItemArray, Index);
  pItem->Flags &= ~(int)Mask;
  pItem->Flags |= Flags;
  GUI_UNLOCK_H(pItem);
}

/*********************************************************************
*
*       MENU__SendMenuMessage
*/
int MENU__SendMenuMessage(MENU_Handle hObj, WM_HWIN hDestWin, U16 MsgType, U16 ItemId) {
  MENU_MSG_DATA MsgData;
  WM_MESSAGE    Msg = {0};
  MsgData.MsgType   = MsgType;
  MsgData.ItemId    = ItemId;
  Msg.MsgId         = WM_MENU;
  Msg.Data.p        = &MsgData;
  Msg.hWinSrc       = hObj;
  if (!hDestWin) {
    hDestWin = WM_GetParent(hObj);
  }
  if (hDestWin) {
    WM__SendMessage(hDestWin, &Msg);
    return Msg.Data.v;
  }
  return 0;
}

/*********************************************************************
*
*       Public code, member functions
*
**********************************************************************
*/
/*********************************************************************
*
*       MENU_AddItem
*/
void MENU_AddItem(MENU_Handle hObj, const MENU_ITEM_DATA * pItemData) {
  if (hObj && pItemData) {
    MENU_Obj * pObj;
    GUI_ARRAY ItemArray;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    ItemArray = pObj->ItemArray;
    GUI_UNLOCK_H(pObj);
    if (GUI_ARRAY_AddItem(ItemArray, NULL, 0) == 0) {
      unsigned Index;
      Index = GUI_ARRAY_GetNumItems(ItemArray) - 1;
      if (MENU__SetItem(hObj, Index, pItemData) == 0) {
        GUI_ARRAY_DeleteItem(ItemArray, Index);
      } else {
        MENU__ResizeMenu(hObj);
      }
    }
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MENU_SetOwner
*/
void MENU_SetOwner(MENU_Handle hObj, WM_HWIN hOwner) {
  if (hObj) {
    MENU_Obj * pObj;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    if (pObj) {
      pObj->hOwner = hOwner;
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
}

/*********************************************************************
*
*       MENU_SetSel
*/
int MENU_SetSel(MENU_Handle hObj, int Sel) {
  int OldSel;
  OldSel = 0;
  if (hObj) {
    MENU_Obj * pObj;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    if (pObj) {
      int NumItems;
      OldSel = pObj->Sel;
      NumItems = GUI_ARRAY_GetNumItems(pObj->ItemArray);
      if (Sel < NumItems) {
        pObj->Sel = Sel;
        WM_InvalidateWindow(hObj);
      }
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return OldSel;
}

/*********************************************************************
*
*       MENU_GetOwner
*/
WM_HWIN MENU_GetOwner(MENU_Handle hObj) {
  WM_HWIN hOwner = 0;
  if (hObj) {
    MENU_Obj * pObj;
    WM_LOCK();
    pObj = MENU_LOCK_H(hObj);
    if (pObj) {
      hOwner = pObj->hOwner ? pObj->hOwner : WM_GetParent(hObj);
    }
    GUI_UNLOCK_H(pObj);
    WM_UNLOCK();
  }
  return hOwner;
}

#else  /* avoid empty object files */
  void Menu_C(void);
  void Menu_C(void) {}
#endif

/*************************** End of file ****************************/
