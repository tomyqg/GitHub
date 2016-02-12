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
File        : CHECKBOX_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"
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
static void _Paint(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  int xSize, ySize, CurrentState;
  U16 State;
  WM_HMEM hText;

  //
  // Get properties
  //
  pObj = CHECKBOX_LOCK_H(hObj);
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  CurrentState = pObj->CurrentState;
  hText = pObj->hpText;
  State = pObj->Widget.State;
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
  //
  // Draw background
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_BUTTON;
  pfDrawSkin(&ItemInfo);
  //
  // Draw bitmap
  //
  if (CurrentState) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_BITMAP;
    ItemInfo.ItemIndex = CurrentState;
    pfDrawSkin(&ItemInfo);
  }
  //
  // Draw text
  //
  if (hText) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
    ItemInfo.p = GUI_LOCK_H(hText);
    pfDrawSkin(&ItemInfo);
    GUI_UNLOCK_H(ItemInfo.p);
  }
  //
  // Draw focus frame
  //
  if ((State & WIDGET_STATE_FOCUS) && (hText)) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_FOCUS;
    ItemInfo.p = GUI_LOCK_H(hText);
    pfDrawSkin(&ItemInfo);
    GUI_UNLOCK_H(ItemInfo.p);
  }
}

/*********************************************************************
*
*       _Create
*/
static void _Create(CHECKBOX_Handle hObj) {
  CHECKBOX_Obj * pObj;
  CHECKBOX_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = CHECKBOX_LOCK_H(hObj);
  pSkinPrivate = (CHECKBOX_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
  &CHECKBOX__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_SetSkin
*/
void CHECKBOX_SetSkin(CHECKBOX_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  CHECKBOX_Obj * pObj;

  WM_LOCK();
  pObj = CHECKBOX_LOCK_H(hObj);
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
*       CHECKBOX_SetDefaultSkin
*/
void CHECKBOX_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  CHECKBOX__pSkinDefault = &_Skin;
  CHECKBOX__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void CHECKBOX_Skin_C(void);
  void CHECKBOX_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
