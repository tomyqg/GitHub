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
File        : HEADER_Skin.c
Purpose     : Implementation of HEADER widget
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Paint
*/
static void _Paint(HEADER_Handle hObj) {
  HEADER_Obj * pObj;
  GUI_ARRAY Columns;
  GUI_RECT Rect, RectClient;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  WIDGET * pWidget;
  char * pText;
  HEADER_COLUMN * pColumn;
  WM_HMEM hDrawObj;
  int Fixed, ScrollPos, Width, i, NumItems, xPos, DirIndicatorColumn;

  //
  // Get header properties
  //
  pObj = HEADER_LOCK_H(hObj);
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  pWidget = &pObj->Widget;
  Columns = pObj->Columns;
  Fixed = pObj->Fixed;
  ScrollPos = pObj->ScrollPos;
  DirIndicatorColumn = pObj->DirIndicatorColumn;
  GUI_UNLOCK_H(pObj);
  NumItems = GUI_ARRAY_GetNumItems(Columns);
  //
  // Get size
  //
  WIDGET__GetClientRect(pWidget, &RectClient);
  //
  // Fill item info structure
  //
  ItemInfo.hWin = hObj;
  //
  // Iterate over columns
  //
  xPos = -ScrollPos;
  Rect.y0 = ItemInfo.y0 = RectClient.y0;
  Rect.y1 = ItemInfo.y1 = RectClient.y1;
  for (i = 0; i < NumItems; i++) {
    //
    // Get column properties
    //
    pColumn  = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, i);
    Width    = pColumn->Width;
    hDrawObj = pColumn->hDrawObj;
    pText    = pColumn->acText;
    GUI_UNLOCK_H(pColumn);
    //
    //
    //
    xPos += (i < Fixed) ? ScrollPos : 0;
    Rect.x0 = ItemInfo.x0 = xPos;
    Rect.x1 = ItemInfo.x1 = ItemInfo.x0 + Width - 1;
    WM_SetUserClipRect(&Rect);
    ItemInfo.ItemIndex = i;
    //
    // Background
    //
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_BACKGROUND;
    pfDrawSkin(&ItemInfo);
    //
    // Bitmap
    //
    if (hDrawObj) {
      ItemInfo.Cmd = WIDGET_ITEM_DRAW_BITMAP;
      pfDrawSkin(&ItemInfo);
    }
    //
    // Text
    //
    if (pText) {
      ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
      pfDrawSkin(&ItemInfo);
    }
    //
    // Arrow
    //
    if (DirIndicatorColumn == i) {
      ItemInfo.Cmd = WIDGET_ITEM_DRAW_ARROW;
      pfDrawSkin(&ItemInfo);
    }
    //
    // Release clipping
    //
    WM_SetUserClipRect(NULL);
    //
    // Adjust position
    //
    xPos += ItemInfo.x1 - ItemInfo.x0 + 1;
  }
  //
  // Overlap
  //
  if (ItemInfo.x1 < RectClient.x1) {
    Rect.x0 = ItemInfo.x0 = xPos;
    Rect.x1 = ItemInfo.x1 = RectClient.x1;
    WM_SetUserClipRect(&Rect);
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_OVERLAP;
    pfDrawSkin(&ItemInfo);
    WM_SetUserClipRect(NULL);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(HEADER_Handle hObj) {
  HEADER_Obj * pObj;
  HEADER_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = HEADER_LOCK_H(hObj);
  pSkinPrivate = (HEADER_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  GUI_UNLOCK_H(pObj);
  //
  // Create
  //
  ItemInfo.Cmd = WIDGET_ITEM_CREATE;
  ItemInfo.hWin = hObj;
  pSkinPrivate->pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       _Skin
*/
static WIDGET_SKIN _Skin = {
  _Paint,
  _Create,
  &HEADER__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_SetSkin
*/
void HEADER_SetSkin(HEADER_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  HEADER_Obj * pObj;

  WM_LOCK();
  pObj = HEADER_LOCK_H(hObj);
  //
  // Enable skinning
  //
  pObj->pWidgetSkin = &_Skin;
  //
  // Set private callback function
  //
  pObj->Props.SkinPrivate.pfDrawSkin = pfDrawSkin;
  GUI_UNLOCK_H(pObj);
  //
  // Invalidate widget
  //
  WM_SetHasTrans(hObj);
  WM_InvalidateWindow(hObj);
  WM_UNLOCK();
}

/*********************************************************************
*
*       HEADER_SetDefaultSkin
*/
void HEADER_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  HEADER__pSkinDefault = &_Skin;
  HEADER__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void HEADER_Skin_C(void);
  void HEADER_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
