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
File        : PROGBAR_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "PROGBAR_Private.h"
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
*       _GetPos
*/
static int _GetPos(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, int IsVertical) {
  PROGBAR_Obj * pObj;
  int Size, Min, Max, v;

  //
  // Get properties
  //
  pObj = PROGBAR_LOCK_H(pDrawItemInfo->hWin);
  Min  = pObj->Min;
  Max  = pObj->Max;
  v    = pObj->v;
  GUI_UNLOCK_H(pObj);
  //
  // Calculate value
  //
  Size = IsVertical ? pDrawItemInfo->y1 - pDrawItemInfo->y0 - 1 : pDrawItemInfo->x1 - pDrawItemInfo->x0 - 1;
  if (v < Min) {
	  v = Min;
  }
  if (v > Max) {
	  v = Max;
  }
  return (Size * (I32)(IsVertical ? Max - v : v - Min)) / (Max - Min);
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;
  GUI_RECT Rect;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  PROGBAR_SKINFLEX_INFO SkinInfo;
  const char * pText;
  const GUI_FONT GUI_UNI_PTR * pFont;
  int Pos;
  
  //
  // Get properties
  //
  pObj = PROGBAR_LOCK_H(hObj);
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  pFont = pObj->Props.pFont;
  SkinInfo.IsVertical = (pObj->Flags & PROGBAR_CF_VERTICAL) ? 1 : 0;
  GUI_UNLOCK_H(pObj);
  ItemInfo.p = &SkinInfo;
  WM_GetClientRectEx(hObj, &Rect);
  //
  // Draw frame
  //
  ItemInfo.Cmd  = WIDGET_ITEM_DRAW_FRAME;
  ItemInfo.hWin = hObj;
  ItemInfo.x0   = Rect.x0;
  ItemInfo.y0   = Rect.y0;
  ItemInfo.x1   = Rect.x1;
  ItemInfo.y1   = Rect.y1;
  pfDrawSkin(&ItemInfo);
  //
  // Draw background, first part
  //
  Pos = _GetPos(&ItemInfo, SkinInfo.IsVertical);
  ItemInfo.Cmd  = WIDGET_ITEM_DRAW_BACKGROUND;
  ItemInfo.hWin = hObj;
  if (SkinInfo.IsVertical) {
    ItemInfo.x0 = Rect.x0;
    ItemInfo.y0 = Rect.y0;
    ItemInfo.x1 = Rect.x1;
    ItemInfo.y1 = Rect.y0 + Pos;
  } else {
    ItemInfo.x0 = Rect.x0;
    ItemInfo.y0 = Rect.y0;
    ItemInfo.x1 = Rect.x0 + Pos;
    ItemInfo.y1 = Rect.y1;
  }
  SkinInfo.Index = PROGBAR_SKINFLEX_L;
  pfDrawSkin(&ItemInfo);
  //
  // Draw background, second part
  //
  if (SkinInfo.IsVertical) {
    ItemInfo.x0 = Rect.x0;
    ItemInfo.y0 = Rect.y0 + Pos + 1;
    ItemInfo.x1 = Rect.x1;
    ItemInfo.y1 = Rect.y1;
  } else {
    ItemInfo.x0 = Rect.x0 + Pos + 1;
    ItemInfo.y0 = Rect.y0;
    ItemInfo.x1 = Rect.x1;
    ItemInfo.y1 = Rect.y1;
  }
  SkinInfo.Index = PROGBAR_SKINFLEX_R;
  pfDrawSkin(&ItemInfo);
  //
  // Draw text
  //
  pObj = PROGBAR_LOCK_H(hObj);
  pText = PROGBAR__GetTextLocked(pObj);
  if (pText) {
    GUI_SetFont(pFont);
    PROGBAR__GetTextRect(pObj, &Rect, pText);
    GUI_UNLOCK_H(pObj);
    SkinInfo.pText = pText;
    ItemInfo.Cmd  = WIDGET_ITEM_DRAW_TEXT;
    ItemInfo.hWin = hObj;
    ItemInfo.x0   = Rect.x0;
    ItemInfo.y0   = Rect.y0;
    ItemInfo.x1   = Rect.x1;
    ItemInfo.y1   = Rect.y1;
    pfDrawSkin(&ItemInfo);
    GUI_UNLOCK_H(pText);
  } else {
    GUI_UNLOCK_H(pObj);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(PROGBAR_Handle hObj) {
  PROGBAR_Obj * pObj;
  PROGBAR_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = PROGBAR_LOCK_H(hObj);
  pSkinPrivate = (PROGBAR_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
  &PROGBAR__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       PROGBAR_SetSkin
*/
void PROGBAR_SetSkin(PROGBAR_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  PROGBAR_Obj * pObj;

  WM_LOCK();
  pObj = PROGBAR_LOCK_H(hObj);
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
*       PROGBAR_SetDefaultSkin
*/
void PROGBAR_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  PROGBAR__pSkinDefault = &_Skin;
  PROGBAR__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void PROGBAR_Skin_C(void);
  void PROGBAR_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
