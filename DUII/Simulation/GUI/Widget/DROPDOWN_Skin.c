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
File        : DROPDOWN_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"
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
static void _Paint(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  GUI_RECT RectClient;
  int Sel;
  const char * s;

  //
  // Get properties
  //
  pObj = DROPDOWN_LOCK_H(hObj);
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  Sel = pObj->Sel;
  GUI_UNLOCK_H(pObj);
  WM_GetClientRectEx(hObj, &RectClient);
  //
  // Draw background
  //
  ItemInfo.Cmd  = WIDGET_ITEM_DRAW_BACKGROUND;
  ItemInfo.hWin = hObj;
  ItemInfo.x0   = RectClient.x0;
  ItemInfo.y0   = RectClient.y0;
  ItemInfo.x1   = RectClient.x1;
  ItemInfo.y1   = RectClient.y1;
  pfDrawSkin(&ItemInfo);
  //
  // Draw text
  //
  s = DROPDOWN__GetpItemLocked(hObj, Sel);
  if (s) {
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
    ItemInfo.p   = (void *)s;
    pfDrawSkin(&ItemInfo);
    GUI_UNLOCK_H(s);
  }
  //
  // Draw arrow
  //
  ItemInfo.Cmd = WIDGET_ITEM_DRAW_ARROW;
  pfDrawSkin(&ItemInfo);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(DROPDOWN_Handle hObj) {
  DROPDOWN_Obj * pObj;
  DROPDOWN_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = DROPDOWN_LOCK_H(hObj);
  pSkinPrivate = (DROPDOWN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
  &DROPDOWN__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_SetSkin
*/
void DROPDOWN_SetSkin(DROPDOWN_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  DROPDOWN_Obj * pObj;

  WM_LOCK();
  pObj = DROPDOWN_LOCK_H(hObj);
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
*       DROPDOWN_SetDefaultSkin
*/
void DROPDOWN_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  DROPDOWN__pSkinDefault = &_Skin;
  DROPDOWN__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void DROPDOWN_Skin_C(void);
  void DROPDOWN_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
