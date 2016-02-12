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
File        : SLIDER_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SLIDER_Private.h"
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
*       _CopyItemInfo2Rect
*/
static void _CopyItemInfo2Rect(WIDGET_ITEM_DRAW_INFO * pItemInfo, GUI_RECT * pRect) {
  pRect->x0 = pItemInfo->x0;
  pRect->y0 = pItemInfo->y0;
  pRect->x1 = pItemInfo->x1;
  pRect->y1 = pItemInfo->y1;
}

/*********************************************************************
*
*       _Paint
*/
static void _Paint(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  GUI_RECT RectClient, RectClip;
  SLIDER_SKINFLEX_INFO SkinInfo;
  int Min, Max, v, Range;
  int s0, s1, HasFocus;
  
  //
  // Get properties
  //
  pObj = SLIDER_LOCK_H(hObj);
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  Min = pObj->Min;
  Max = pObj->Max;
  v = pObj->v;
  SkinInfo.NumTicks = pObj->NumTicks;
  SkinInfo.Width = pObj->Width;
  SkinInfo.IsPressed = (pObj->Widget.State & SLIDER_STATE_PRESSED) ? 1 : 0;
  SkinInfo.IsVertical = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? 1 : 0;
  HasFocus = pObj->Widget.State & WIDGET_STATE_FOCUS;
  GUI_UNLOCK_H(pObj);
  WM_GetClientRectEx(hObj, &RectClient);
  Range = (Max - Min) ? Max - Min : 1;
  SkinInfo.Size = SkinInfo.IsVertical ? 
                  RectClient.y1 - RectClient.y0 + 1 - SkinInfo.Width - 2 : 
                  RectClient.x1 - RectClient.x0 + 1 - SkinInfo.Width - 2;
  if (SkinInfo.NumTicks < 0) {
    SkinInfo.NumTicks = Range + 1;
    if (SkinInfo.NumTicks > (SkinInfo.Size / 5)) {
      SkinInfo.NumTicks = 11;
    }
  }
  //
  // Calculate slider position
  //
  if (SkinInfo.IsVertical) {
    s0 = RectClient.y0 + 1 + ((RectClient.y1 - RectClient.y0 + 1) - 2 - SkinInfo.Width) * (v - Min) / Range;
  } else {
    s0 = RectClient.x0 + 1 + ((RectClient.x1 - RectClient.x0 + 1) - 2 - SkinInfo.Width) * (v - Min) / Range;
  }
  s1 = s0 + SkinInfo.Width - 1;
  //
  // Initialize ItemInfo structure
  //
  ItemInfo.hWin = hObj;
  ItemInfo.p    = &SkinInfo;
  //
  // Draw shaft (complete)
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_SHAFT;
  ItemInfo.y0 = RectClient.y0 + 1;
  ItemInfo.y1 = RectClient.y1 - 1;
  ItemInfo.x0 = RectClient.x0 + 1;
  ItemInfo.x1 = RectClient.x1 - 1;
  _CopyItemInfo2Rect(&ItemInfo, &RectClip);
  WM_SetUserClipRect(&RectClip);
  pfDrawSkin(&ItemInfo);
  WM_SetUserClipRect(NULL);
  //
  // Draw thumb
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_THUMB;
  if (SkinInfo.IsVertical) {
    ItemInfo.y0 = s0;
    ItemInfo.y1 = s1;
  } else {
    ItemInfo.x0 = s0;
    ItemInfo.x1 = s1;
  }
  _CopyItemInfo2Rect(&ItemInfo, &RectClip);
  WM_SetUserClipRect(&RectClip);
  pfDrawSkin(&ItemInfo);
  WM_SetUserClipRect(NULL);
  //
  // Draw ticks
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_TICKS;
  ItemInfo.y0 = RectClient.y0 + 1;
  ItemInfo.y1 = RectClient.y1 - 1;
  ItemInfo.x0 = RectClient.x0 + 1;
  ItemInfo.x1 = RectClient.x1 - 1;
  _CopyItemInfo2Rect(&ItemInfo, &RectClip);
  WM_SetUserClipRect(&RectClip);
  pfDrawSkin(&ItemInfo);
  WM_SetUserClipRect(NULL);
  //
  // Draw focus
  //
  if (HasFocus) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_FOCUS;
    ItemInfo.y0 = RectClient.y0;
    ItemInfo.y1 = RectClient.y1;
    ItemInfo.x0 = RectClient.x0;
    ItemInfo.x1 = RectClient.x1;
    _CopyItemInfo2Rect(&ItemInfo, &RectClip);
    WM_SetUserClipRect(&RectClip);
    pfDrawSkin(&ItemInfo);
    WM_SetUserClipRect(NULL);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(SLIDER_Handle hObj) {
  SLIDER_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = SLIDER_LOCK_H(hObj);
  pfDrawSkin   = pObj->Props.SkinPrivate.pfDrawSkin;
  GUI_UNLOCK_H(pObj);
  //
  // Create
  //
  ItemInfo.Cmd = WIDGET_ITEM_CREATE;
  ItemInfo.hWin = hObj;
  ItemInfo.p = NULL;
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
  &SLIDER__DefaultProps.SkinPrivate,
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SLIDER_SetSkin
*/
void SLIDER_SetSkin(SLIDER_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  SLIDER_Obj * pObj;

  WM_LOCK();
  pObj = SLIDER_LOCK_H(hObj);
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
*       SLIDER_SetDefaultSkin
*/
void SLIDER_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  SLIDER__pSkinDefault = &_Skin;
  SLIDER__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void SLIDER_Skin_C(void);
  void SLIDER_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
