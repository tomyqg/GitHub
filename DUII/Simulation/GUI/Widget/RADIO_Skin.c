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
File        : RADIO_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "RADIO_Private.h"
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
static void _Paint(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  RADIO_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  int i, Sel, HasFocus, FontDistY, NumItems, Spacing, ButtonSize, Height, StringDistX, GroupId, DrawFocus;
  GUI_RECT RectClient, RectFocus;
  const char * pText;
  GUI_ARRAY TextArray;
  GUI_FONTINFO FontInfo;
  const GUI_FONT GUI_UNI_PTR * pFont;
  U8 SpaceAbove, CHeight, FocusBorder;

  //
  // Get properties
  //
  pObj = RADIO_LOCK_H(hObj);
  pSkinPrivate = (RADIO_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  NumItems     = pObj->NumItems;
  Spacing      = pObj->Spacing;
  TextArray    = pObj->TextArray;
  pFont        = pObj->Props.pFont;
  Height       = RADIO__GetButtonSize(hObj) + RADIO_BORDER * 2;
  Sel          = pObj->Sel;
  GroupId      = pObj->GroupId;
  HasFocus     = (pObj->Widget.State & WIDGET_STATE_FOCUS) ? 1 : 0;
  GUI_UNLOCK_H(pObj);
  WM_GetClientRect(&RectClient);
  ButtonSize = pSkinPrivate->pfGetButtonSize(hObj);
  //
  // Fill item info structure
  //
  ItemInfo.hWin = hObj;
  //
  // Draw buttons
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BUTTON;
  ItemInfo.x0 = RectClient.x0 + RADIO_BORDER;
  ItemInfo.y0 = RectClient.y0 + RADIO_BORDER;
  ItemInfo.x1 = ItemInfo.x0 + ButtonSize - 1;
  ItemInfo.y1 = ItemInfo.y0 + ButtonSize - 1;
  for (i = DrawFocus = 0; i < NumItems; i++) {
    ItemInfo.ItemIndex = i;
    pfDrawSkin(&ItemInfo);
    ItemInfo.y0 += Spacing;
    ItemInfo.y1 += Spacing;
  }
  //
  // Draw text
  //
  GUI_GetFontInfo(pFont, &FontInfo);
  GUI_SetFont(pFont);
  GUI_SetTextMode(GUI_TM_TRANS);
  FontDistY    = GUI_GetFontDistY();
  CHeight      = FontInfo.CHeight;
  SpaceAbove   = FontInfo.Baseline - CHeight;
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
  ItemInfo.x0  = RectClient.x0 + RADIO_BORDER * 2 + 2 + ButtonSize;
  ItemInfo.y0  = RectClient.y0 + ((CHeight <= Height) ? ((Height - CHeight) / 2) : 0) - SpaceAbove;
  for (i = 0; i < NumItems; i++) {
    pText = (const char *)GUI_ARRAY_GetpItemLocked(TextArray, i);
    if (pText) {
      ItemInfo.ItemIndex = i;
      StringDistX        = GUI_GetStringDistX(pText);
      ItemInfo.x1        = ItemInfo.x0 + StringDistX - 2;
      ItemInfo.p         = (void *)pText;
      pfDrawSkin(&ItemInfo);
      GUI_UNLOCK_H(pText);
      //
      // Get focus rect
      //
      if (HasFocus && ((Sel == i) || (GroupId))) {
        FocusBorder  = (FontDistY <= 12) ? 2 : 3;
        if ((ItemInfo.y0 + SpaceAbove) < FocusBorder) {
          FocusBorder = ItemInfo.y0;
        }
        RectFocus.x0 = ItemInfo.x0 - FocusBorder;
        RectFocus.y0 = ItemInfo.y0;
        RectFocus.x1 = ItemInfo.x1 + FocusBorder;
        RectFocus.y1 = ItemInfo.y0 + CHeight - 1 + SpaceAbove + FocusBorder;
		DrawFocus = 1;
      }
    }
    ItemInfo.y0 += Spacing;
    ItemInfo.y1 += Spacing;
  }
  //
  // Draw focus
  //
  if (HasFocus & DrawFocus) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_FOCUS;
    ItemInfo.x0 = RectFocus.x0;
    ItemInfo.y0 = RectFocus.y0;
    ItemInfo.x1 = RectFocus.x1;
    ItemInfo.y1 = RectFocus.y1;
    pfDrawSkin(&ItemInfo);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(RADIO_Handle hObj) {
  RADIO_Obj * pObj;
  RADIO_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = RADIO_LOCK_H(hObj);
  pSkinPrivate = (RADIO_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
  &RADIO__DefaultProps.SkinPrivate,
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       RADIO_SetSkin
*/
void RADIO_SetSkin(RADIO_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  RADIO_Obj * pObj;

  WM_LOCK();
  pObj = RADIO_LOCK_H(hObj);
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
*       RADIO_SetDefaultSkin
*/
void RADIO_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  RADIO__pSkinDefault = &_Skin;
  RADIO__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void RADIO_Skin_C(void);
  void RADIO_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
