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
File        : BUTTON_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"
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
static void _Paint(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  int xSize, ySize, State;
  U16 Status;
  
  //
  // Get properties
  //
  pObj = BUTTON_LOCK_H(hObj);
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  State = pObj->Widget.State;
  Status = pObj->Widget.Win.Status;
  GUI_UNLOCK_H(pObj);
  //
  // Get size
  //
  xSize = WM_GetWindowSizeX(hObj);
  ySize = WM_GetWindowSizeY(hObj);
  //
  // Fill item info structure
  //
  ItemInfo.hWin = hObj;
  ItemInfo.x0 = ItemInfo.y0 = 0;
  ItemInfo.x1 = xSize - 1;
  ItemInfo.y1 = ySize - 1;
  if        (State & BUTTON_STATE_PRESSED) {
    ItemInfo.ItemIndex = BUTTON_SKINFLEX_PI_PRESSED;
  } else if (State & BUTTON_STATE_FOCUS) {
    ItemInfo.ItemIndex = BUTTON_SKINFLEX_PI_FOCUSSED;
  } else if (Status & WM_SF_DISABLED) {
    ItemInfo.ItemIndex = BUTTON_SKINFLEX_PI_DISABLED;
  } else {
    ItemInfo.ItemIndex = BUTTON_SKINFLEX_PI_ENABLED;
  }
  //
  // Draw background
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BACKGROUND;
  pfDrawSkin(&ItemInfo);
  //
  // Draw bitmap
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BITMAP;
  pfDrawSkin(&ItemInfo);
  //
  // Draw text
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
  pfDrawSkin(&ItemInfo);
  //
  // Draw focus frame
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_FOCUS;
  pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(BUTTON_Handle hObj) {
  BUTTON_Obj * pObj;
  BUTTON_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo = {0};

  //
  // Get properties
  //
  pObj = BUTTON_LOCK_H(hObj);
  pSkinPrivate = (BUTTON_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
  &BUTTON__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_SetSkin
*/
void BUTTON_SetSkin(BUTTON_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  BUTTON_Obj * pObj;

  WM_LOCK();
  pObj = BUTTON_LOCK_H(hObj);
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
*       BUTTON_SetDefaultSkin
*/
void BUTTON_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  BUTTON__pSkinDefault = &_Skin;
  BUTTON__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void BUTTON_Skin_C(void);
  void BUTTON_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
