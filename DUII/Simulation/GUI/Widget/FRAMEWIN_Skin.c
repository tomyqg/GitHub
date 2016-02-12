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
File        : FRAMEWIN_Skin.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Configuration defaults
*
**********************************************************************
*/

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
static void _Paint(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  WIDGET_DRAW_ITEM_FUNC * pfDrawSkin;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  const GUI_FONT GUI_UNI_PTR * pFont;
  WIDGET_ITEM_DRAW_INFO ItemInfo;
  int BorderSizeL;
  int BorderSizeR;
  int BorderSizeT;
  int xSize, ySize;
  int TitleHeight;
  int SepHeight;
  
  //
  // Get properties
  //
  pObj = FRAMEWIN_LOCK_H(hObj);
  pFont = pObj->Props.pFont;
  pfDrawSkin = pObj->Props.SkinPrivate.pfDrawSkin;
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  TitleHeight = FRAMEWIN__CalcTitleHeight(pObj);
  SepHeight = pObj->Props.IBorderSize;
  GUI_UNLOCK_H(pObj);
  //
  // Get size
  //
  xSize = WM_GetWindowSizeX(hObj);
  ySize = WM_GetWindowSizeY(hObj);
  BorderSizeL = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_L);
  BorderSizeR = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_R);
  BorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
  //
  // Fill item info structure
  //
  ItemInfo.hWin = hObj;
  //
  // Drawing
  //
  WM_ITERATE_START(NULL) {
    ItemInfo.x0 = ItemInfo.y0 = 0;
    ItemInfo.x1 = xSize - 1;
    ItemInfo.y1 = ySize - 1;
    //
    // Draw frame
    //
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_FRAME;
    pfDrawSkin(&ItemInfo);
    //
    // Draw separator
    //
    ItemInfo.x0 = BorderSizeL;
    ItemInfo.y0 = BorderSizeT + TitleHeight;
    ItemInfo.x1 = xSize - BorderSizeR - 1;
    ItemInfo.y1 = BorderSizeT + TitleHeight + SepHeight - 1;
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_SEP;
    pfDrawSkin(&ItemInfo);
    //
    // Draw title bar
    //
    ItemInfo.x0 = BorderSizeL;
    ItemInfo.y0 = BorderSizeT;
    ItemInfo.x1 = xSize - BorderSizeR - 1;
    ItemInfo.y1 = BorderSizeT + TitleHeight - 1;
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_BACKGROUND;
    pfDrawSkin(&ItemInfo);
    //
    // Draw title text
    //
    FRAMEWIN__GetTitleLimits(hObj, &ItemInfo.x0, &ItemInfo.x1);
    GUI_SetTextMode(GUI_TM_TRANS);
    GUI_SetFont(pFont);
    ItemInfo.Cmd = WIDGET_ITEM_DRAW_TEXT;
    pfDrawSkin(&ItemInfo);
  } WM_ITERATE_END();
}

/*********************************************************************
*
*       _Create
*/
static void _Create(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  //
  // Get properties
  //
  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
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
*       Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       _Skin
*/
WIDGET_SKIN _Skin = {
  _Paint,
  _Create,
  &FRAMEWIN__DefaultProps.SkinPrivate
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_SetSkin
*/
void FRAMEWIN_SetSkin(FRAMEWIN_Handle hObj, WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  int OldHeight, OldBorderSizeL, OldBorderSizeR, OldBorderSizeT;

  GUI_LOCK();
  pObj = FRAMEWIN_LOCK_H(hObj);
  pSkinPrivate = (FRAMEWIN_SKIN_PRIVATE *)pObj->pWidgetSkin->pSkinPrivate;
  OldHeight = FRAMEWIN__CalcTitleHeight(pObj);
  GUI_UNLOCK_H(pObj);
  //
  // Get old border size
  //
  OldBorderSizeL = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_L);
  OldBorderSizeR = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_R);
  OldBorderSizeT = pSkinPrivate->pfGetBordersize(hObj, FRAMEWIN_GET_BORDERSIZE_T);
  //
  // Enable skinning
  //
  pObj = FRAMEWIN_LOCK_H(hObj);
  pObj->pWidgetSkin = &_Skin;
  //
  // Set private callback function
  //
  pObj->Props.SkinPrivate.pfDrawSkin = pfDrawSkin;
  GUI_UNLOCK_H(pObj);
  //
  // Recalc positions and invalidate
  //
  FRAMEWIN__UpdatePositions(hObj);
  FRAMEWIN__UpdateButtons(hObj, OldHeight, OldBorderSizeL, OldBorderSizeR, OldBorderSizeT);
  WM_SetHasTrans(hObj);
  WM_InvalidateWindow(hObj);
  GUI_UNLOCK();
}

/*********************************************************************
*
*       FRAMEWIN_SetDefaultSkin
*/
void FRAMEWIN_SetDefaultSkin(WIDGET_DRAW_ITEM_FUNC * pfDrawSkin) {
  FRAMEWIN__pSkinDefault = &_Skin;
  FRAMEWIN__DefaultProps.SkinPrivate.pfDrawSkin = pfDrawSkin;
}

#else
  void FRAMEWIN_Skin_C(void);
  void FRAMEWIN_Skin_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
