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
File        : SCROLLBAR_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SCROLLBAR_Private.h"
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
*       _CalcPositions
*
*  Calculates all positions required for drawing or for mouse / touch
*  evaluation.
*/
static void _CalcPositions(SCROLLBAR_Handle hObj, SCROLLBAR_POSITIONS * pPos) {
  SCROLLBAR_OBJ * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  SCROLLBAR_SKINFLEX_INFO SkinInfo;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  int xSizeButton, xSize, xSizeMoveable, ThumbSize, NumItems, xSizeThumbArea;
  WM_HWIN hWin;
  GUI_RECT r, rSub;
  int x0, y0;
  
  pObj = SCROLLBAR_LOCK_H(hObj);
  r = pObj->Widget.Win.Rect;
  x0 = r.x0;
  y0 = r.y0;
  pPos->x1  = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? r.y1 : r.x1;
  //
  // Get button size
  //
  SkinInfo.IsVertical = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? 1 : 0;
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  ItemInfo.Cmd = WIDGET_ITEM_GET_BUTTONSIZE;
  ItemInfo.hWin = hObj;
  ItemInfo.x0 = r.x0;
  ItemInfo.y0 = r.y0;
  ItemInfo.x1 = r.x1;
  ItemInfo.y1 = r.y1;
  ItemInfo.p = &SkinInfo;
  xSizeButton = pfDrawSkin(&ItemInfo);
  //
  // Subtract the rectangle of the other scrollbar (if existing and visible)
  //
  if (pObj->Widget.Id == GUI_ID_HSCROLL) {
    hWin = WM_GetScrollbarV(pObj->Widget.Win.hParent);
    if (hWin) {
      WM_GetWindowRectEx(hWin, &rSub);
      if (r.x1 == rSub.x1) {
        r.x1 = rSub.x0 -1;
      }
    }
  }
  if (pObj->Widget.Id == GUI_ID_VSCROLL) {
    hWin = WM_GetScrollbarH(pObj->Widget.Win.hParent);
    if (hWin) {
      WM_GetWindowRectEx(hWin, &rSub);
      if (r.y1 == rSub.y1) {
        r.y1 = rSub.y0 -1;
      }
    }
  }
  //
  // Convert coordinates of this window
  //
  GUI_MoveRect(&r, -x0, -y0);
  //
  // Convert real into virtual coordinates
  //
  SCROLLBAR__Rect2VRect(&pObj->Widget, &r);
  NumItems      = pObj->NumItems;
  xSize         = r.x1 - r.x0 + 1;
  xSizeThumbArea= xSize - 2 * xSizeButton;     // Number of pixels available for thumb and movement
  ThumbSize     = (int)GUI__DivideRound32((I32)xSizeThumbArea * pObj->PageSize, NumItems);
  if (ThumbSize < SCROLLBAR_THUMB_SIZE_MIN_DEFAULT + 2) {
    ThumbSize = SCROLLBAR_THUMB_SIZE_MIN_DEFAULT + 2;
  }
  if (ThumbSize > xSizeThumbArea) {
    ThumbSize = xSizeThumbArea;
  }
  xSizeMoveable = xSizeThumbArea - ThumbSize;
  pPos->x0_LeftArrow  = r.x0;
  pPos->x1_LeftArrow  = xSizeButton - 1;
  pPos->x1_RightArrow = xSize - 1;
  pPos->x0_RightArrow = xSize - xSizeButton;
  pPos->x0_Thumb      = pPos->x1_LeftArrow + 1 + GUI__DivideRound32((I32)xSizeMoveable * pObj->v, NumItems - pObj->PageSize);
  pPos->x1_Thumb      = pPos->x0_Thumb + ThumbSize - 1;
  pPos->xSizeMoveable = xSizeMoveable;
  pPos->ThumbSize     = ThumbSize;
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;
  GUI_RECT RectClient;
  SCROLLBAR_POSITIONS Pos;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  SCROLLBAR_SKINFLEX_INFO SkinInfo;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  WIDGET * pWidget;

  //
  // Get properties
  //
  pObj = SCROLLBAR_LOCK_H(hObj);
  pWidget = &pObj->Widget;
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  SkinInfo.State = pObj->State;
  SkinInfo.IsVertical = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? 1 : 0;
  _CalcPositions(hObj, &Pos);
  GUI_UNLOCK_H(pObj);
  //
  // Get size
  //
  WIDGET__GetClientRect(pWidget, &RectClient);
  //
  // Fill item info structure
  //
  ItemInfo.hWin = hObj;
  ItemInfo.y0 = RectClient.y0;
  ItemInfo.y1 = RectClient.y1;
  ItemInfo.p  = &SkinInfo;
  //
  // Draw left button
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BUTTON_L;
  ItemInfo.x0 = Pos.x0_LeftArrow;
  ItemInfo.x1 = Pos.x1_LeftArrow;
  pfDrawSkin(&ItemInfo);
  //
  // Draw shaft
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_SHAFT_L;
  ItemInfo.x0 = Pos.x1_LeftArrow + 1;
  ItemInfo.x1 = Pos.x0_Thumb - 1;
  pfDrawSkin(&ItemInfo);
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_SHAFT_R;
  ItemInfo.x0 = Pos.x1_Thumb + 1;
  ItemInfo.x1 = Pos.x0_RightArrow - 1;
  pfDrawSkin(&ItemInfo);
  //
  // Draw thumb
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_THUMB;
  ItemInfo.x0 = Pos.x0_Thumb;
  ItemInfo.x1 = Pos.x1_Thumb;
  pfDrawSkin(&ItemInfo);
  //
  // Draw right button
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BUTTON_R;
  ItemInfo.x0 = Pos.x0_RightArrow;
  ItemInfo.x1 = Pos.x1_RightArrow;
  pfDrawSkin(&ItemInfo);
  //
  // Draw overlapping area
  //
  if (Pos.x1_RightArrow != Pos.x1) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_OVERLAP;
    ItemInfo.x0 = Pos.x1_RightArrow + 1;
    ItemInfo.x1 = Pos.x1;
    pfDrawSkin(&ItemInfo);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(SCROLLBAR_Handle hObj) {
  SCROLLBAR_SKINFLEX_INFO SkinInfo;
  SCROLLBAR_OBJ * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = SCROLLBAR_LOCK_H(hObj);
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  SkinInfo.IsVertical = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? 1 : 0;
  pObj->pfCalcPositions = _CalcPositions;
  GUI_UNLOCK_H(pObj);
  //
  // Create
  //
  ItemInfo.Cmd = WIDGET_ITEM_CREATE;
  ItemInfo.hWin = hObj;
  ItemInfo.p  = &SkinInfo;
  pfDrawSkin(&ItemInfo);
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
  &SCROLLBAR__DefaultProps.SkinPrivate,
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_SetSkin
*/
void SCROLLBAR_SetSkin(SCROLLBAR_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  SCROLLBAR_OBJ * pObj;

  WM_LOCK();
  pObj = SCROLLBAR_LOCK_H(hObj);
  //
  // Enable skinning
  //
  pObj->pWidgetSkin = &_Skin;
  pObj->pfCalcPositions = _CalcPositions;
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
*       SCROLLBAR_SetDefaultSkin
*/
void SCROLLBAR_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  SCROLLBAR__pSkinDefault = &_Skin;
  SCROLLBAR__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void SCROLLBAR_Skin_C(void);
  void SCROLLBAR_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
