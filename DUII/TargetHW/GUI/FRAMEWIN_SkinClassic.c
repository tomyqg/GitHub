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
File        : FRAMEWIN_SkinRouned.c
Purpose     : Implementation of framewindow widget
---------------------------END-OF-HEADER------------------------------
*/

#include "FRAMEWIN_Private.h"
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
static void _Paint(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  int xsize;
  int ysize;
  int BorderSize;
  int y0;
  FRAMEWIN_POSITIONS Pos;
  GUI_RECT r;
  WIDGET_ITEM_DRAW_INFO ItemInfo;

  pObj = FRAMEWIN_LOCK_H(hObj);
  xsize = WM_GetWindowSizeX(hObj);
  ysize = WM_GetWindowSizeY(hObj);
  BorderSize = pObj->Props.BorderSize;
  //
  // Perform computations
  //
  FRAMEWIN__CalcPositions(hObj, &Pos);
  y0 = Pos.TitleHeight + BorderSize;
  //
  // Fill item info structure
  //
  ItemInfo.Cmd  = WIDGET_ITEM_DRAW;
  ItemInfo.hWin = hObj;
  ItemInfo.p  = &Pos;
  //
  // Calculate the clip rectangle for the title bar
  //
  ItemInfo.x0 = r.x0 = Pos.rClient.x0;
  ItemInfo.x1 = r.x1 = Pos.rClient.x1;
  ItemInfo.y0 = r.y0 = Pos.rTitleText.y0 - 1;
  ItemInfo.y1 = r.y1 = Pos.rTitleText.y1;
  ItemInfo.y0++;
  //
  // Perform frame drawing operations
  //
  WM_ITERATE_START(NULL) {          // Let window manager iterate for every rectangle. This is optional; the idea is to accelerate drawing
    //
    // Draw Title bar
    //
    WM_SetUserClipRect(&r);
    if (pObj->pfDrawItem) {
      pObj->pfDrawItem(&ItemInfo);
    }
    WM_SetUserClipRect(NULL);
    //
    // Draw Frame
    //
    LCD_SetColor(FRAMEWIN_FRAMECOLOR_DEFAULT);
    GUI_FillRect(0, 0, xsize - 1, BorderSize - 1);
	  GUI_FillRect(0, 0, Pos.rClient.x0 - 1, ysize - 1);
	  GUI_FillRect(Pos.rClient.x1 + 1, 0, xsize - 1, ysize - 1);
    GUI_FillRect(0, Pos.rClient.y1 + 1, xsize - 1, ysize - 1);
    if (pObj->Widget.State & FRAMEWIN_SF_TITLEVIS) {
      GUI_FillRect(0, y0, xsize - 1, y0 + pObj->Props.IBorderSize - 1);
    }
    //
    // Draw the 3D effect (if configured)
    //
    if (BorderSize >= 2) {
//      pObj->Widget.pEffect->pfDrawUp(); // WIDGET_EFFECT_3D_DrawUp();
      WIDGET_EFFECT_3D_DrawUp();  // pObj->Widget.pEffect->pfDrawUp();
    }
  } WM_ITERATE_END();
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       FRAMEWIN_OwnerDraw
*/
int FRAMEWIN_OwnerDraw(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_POSITIONS    * pPos;
  const char   * pText;
  int Index;
  GUI_RECT r, rText, rTitleText;

  pObj = FRAMEWIN_LOCK_H(pDrawItemInfo->hWin);
  pPos = (FRAMEWIN_POSITIONS *)pDrawItemInfo->p;
  Index = (pObj->Flags & FRAMEWIN_SF_ACTIVE) ? 1 : 0;
  if (pObj->hText) {
    pText = (const char *)GUI_LOCK_H(pObj->hText);
  } else {
    pText = NULL;
  }
  rTitleText = pPos->rTitleText;
  r.x0 = pPos->rClient.x0 - 1;
  r.y0 = pPos->rTitleText.y0;
  r.x1 = pPos->rClient.x1 + 1;
  r.y1 = pPos->rTitleText.y1;
  rTitleText.x0++;
  rTitleText.y0++;
  rTitleText.x1--;
  GUI_SetFont(pObj->Props.pFont);
  GUI__CalcTextRect(pText, &rTitleText, &rText, pObj->Props.TextAlign);
  LCD_SetBkColor(pObj->Props.aBarColor[Index]);
  LCD_SetColor(pObj->Props.aTextColor[Index]);
  WIDGET__FillStringInRect(pText, &r, &rTitleText, &rText);
  GUI_UNLOCK_H(pObj);
  if (pText) {
    GUI_UNLOCK_H(pText);
  }
  return 0;
}

/*********************************************************************
*
*       _Create
*/
static void _Create(FRAMEWIN_Handle hObj) {
  FRAMEWIN_SetOwnerDraw(hObj, FRAMEWIN_OwnerDraw);
}

/*********************************************************************
*
*       _GetBorderSize
*/
static unsigned _GetBorderSize(FRAMEWIN_Handle hObj, unsigned Index) {
  unsigned BorderSize;
  FRAMEWIN_Obj * pObj;

  GUI_USE_PARA(Index);
  pObj = FRAMEWIN_LOCK_H(hObj);
  BorderSize = pObj->Props.BorderSize;
  GUI_UNLOCK_H(pObj);
  return BorderSize;
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
/*********************************************************************
*
*       _SkinPrivate
*/
static FRAMEWIN_SKIN_PRIVATE _SkinPrivate = {
  _GetBorderSize
};

/*********************************************************************
*
*       FRAMEWIN__SkinClassic
*/
const WIDGET_SKIN FRAMEWIN__SkinClassic = {
  _Paint,
  _Create,
  &_SkinPrivate
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       FRAMEWIN_SetSkinClassic
*/
void FRAMEWIN_SetSkinClassic(FRAMEWIN_Handle hObj) {
  FRAMEWIN_Obj * pObj;
  FRAMEWIN_SKIN_PRIVATE * pSkinPrivate;
  int OldHeight, OldBorderSizeL, OldBorderSizeR, OldBorderSizeT;

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
  pObj->pWidgetSkin = &FRAMEWIN__SkinClassic;
  GUI_UNLOCK_H(pObj);
  //
  // Recalc positions and invalidate
  //
  FRAMEWIN__UpdatePositions(hObj);
  FRAMEWIN__UpdateButtons(hObj, OldHeight, OldBorderSizeL, OldBorderSizeR, OldBorderSizeT);
  FRAMEWIN_SetOwnerDraw(hObj, FRAMEWIN_OwnerDraw);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       FRAMEWIN_SetDefaultSkinClassic
*/
void FRAMEWIN_SetDefaultSkinClassic(void) {
  FRAMEWIN__pSkinDefault = &FRAMEWIN__SkinClassic;
}

#else
  void FRAMEWIN_SkinClassic_C(void);
  void FRAMEWIN_SkinClassic_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
