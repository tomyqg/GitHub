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
File        : SCROLLBAR_SkinClassic.c
Purpose     : Implementation of scrollbar widget
---------------------------END-OF-HEADER------------------------------
*/

#include "SCROLLBAR_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*        Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetArrowSize
*
*/
static int _GetArrowSize(SCROLLBAR_OBJ * pObj) {
  unsigned int r, xSize, ySize;

  xSize = WIDGET__GetXSize(&pObj->Widget);
  ySize = WIDGET__GetYSize(&pObj->Widget);
  r = (ySize / 2) + 5;
  if (r > (xSize - 5)) {
    r = xSize - 5;
  }
  return r;
}

/*********************************************************************
*
*       _CalcPositions
*
*  Calculates all positions required for drawing or for mouse / touch
*  evaluation.
*/
static void _CalcPositions(SCROLLBAR_Handle hObj, SCROLLBAR_POSITIONS * pPos) {
  SCROLLBAR_OBJ * pObj;
  int xSizeArrow, xSize, xSizeMoveable, ThumbSize, NumItems, xSizeThumbArea;
  WM_HWIN hWin;
  GUI_RECT r, rSub;
  int x0, y0;

  pObj = SCROLLBAR_LOCK_H(hObj);
  r = pObj->Widget.Win.Rect;
  x0 = r.x0;
  y0 = r.y0;
  pPos->x1  = (pObj->Widget.State & WIDGET_STATE_VERTICAL) ? r.y1 : r.x1;
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
  xSizeArrow    = _GetArrowSize(pObj);
  xSizeThumbArea= xSize - 2 * xSizeArrow;     // Number of pixels available for thumb and movement
  ThumbSize     = (int)GUI__DivideRound32((I32)xSizeThumbArea * pObj->PageSize, NumItems);
  if (ThumbSize < SCROLLBAR_THUMB_SIZE_MIN_DEFAULT) {
    ThumbSize = SCROLLBAR_THUMB_SIZE_MIN_DEFAULT;
  }
  if (ThumbSize > xSizeThumbArea) {
    ThumbSize = xSizeThumbArea;
  }
  xSizeMoveable = xSizeThumbArea - ThumbSize;
  pPos->x0_LeftArrow  = r.x0;
  pPos->x1_LeftArrow  = xSizeArrow - 1;
  pPos->x1_RightArrow = xSize - 1;
  pPos->x0_RightArrow = xSize - xSizeArrow;
  pPos->x0_Thumb      = pPos->x1_LeftArrow + 1+ GUI__DivideRound32((I32)xSizeMoveable * pObj->v, NumItems - pObj->PageSize);
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
  int ArrowSize, ArrowOff;
  SCROLLBAR_POSITIONS Pos;
  GUI_RECT r, rClient;
  pObj = SCROLLBAR_LOCK_H(hObj);
  //
  // Get / calc position info
  //
  pObj->pfCalcPositions(hObj, &Pos);
  //SCROLLBAR__CalcPositions(pObj, &Pos);
  WIDGET__GetClientRect(&pObj->Widget, &rClient);
  r = rClient;
  ArrowSize = ((r.y1 - r.y0) / 3) - 1;
  ArrowOff = 3 + ArrowSize + ArrowSize / 3;
  //
  // Draw left Arrow
  //
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_THUMB]);
  r = rClient;
  r.x0 = Pos.x0_LeftArrow;
  r.x1 = Pos.x1_LeftArrow;
  WIDGET__FillRectEx(&pObj->Widget, &r);
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_ARROW]);
  WIDGET__DrawTriangle(&pObj->Widget, r.x0 + ArrowOff, (r.y1 - r.y0) >> 1, ArrowSize, -1);
  WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &r);
  //
  // Draw the thumb area which is not covered by the thumb
  //
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_SHAFT]);
  r.x0 = Pos.x1_LeftArrow + 1;
  r.x1 = Pos.x0_Thumb - 1;
  WIDGET__FillRectEx(&pObj->Widget, &r);
  r = rClient;
  r.x0 = Pos.x1_Thumb + 1;
  r.x1 = Pos.x0_RightArrow - 1;
  WIDGET__FillRectEx(&pObj->Widget, &r);
  //
  // Draw Thumb
  //
  r = rClient;
  r.x0 = Pos.x0_Thumb;
  r.x1 = Pos.x1_Thumb;
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_THUMB]);
  WIDGET__FillRectEx(&pObj->Widget, &r);
  WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &r);
  //
  // Draw right Arrow
  //
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_THUMB]);
  r.x0 = Pos.x0_RightArrow;
  r.x1 = Pos.x1_RightArrow;
  WIDGET__FillRectEx(&pObj->Widget, &r);
  LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_ARROW]);
  WIDGET__DrawTriangle(&pObj->Widget, r.x1 - ArrowOff, (r.y1 - r.y0) >> 1, ArrowSize, 1);
  WIDGET__EFFECT_DrawUpRect(&pObj->Widget, &r);
  //
  // Draw overlap area (if any ...)
  //
  if (Pos.x1_RightArrow != Pos.x1) {
    r.x0 = Pos.x1_RightArrow + 1;
    r.x1 = Pos.x1;
    LCD_SetColor(pObj->Props.aColor[SCROLLBAR_CI_THUMB]);
    WIDGET__FillRectEx(&pObj->Widget, &r);
  }
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*       _Create
*/
static void _Create(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;

  WM_SetHasTrans(hObj);
  pObj = SCROLLBAR_LOCK_H(hObj);
  pObj->pfCalcPositions = _CalcPositions;
  GUI_UNLOCK_H(pObj);
}

/*********************************************************************
*
*        Private data
*
**********************************************************************
*/
const WIDGET_SKIN SCROLLBAR__SkinClassic = {
  _Paint,
  _Create
};

/*********************************************************************
*
*        Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_SetSkinClassic
*/
void SCROLLBAR_SetSkinClassic(SCROLLBAR_Handle hObj) {
  SCROLLBAR_OBJ * pObj;

  pObj = SCROLLBAR_LOCK_H(hObj);
  pObj->pWidgetSkin = &SCROLLBAR__SkinClassic;
  pObj->pfCalcPositions = _CalcPositions;
  GUI_UNLOCK_H(pObj);
  WM_InvalidateWindow(hObj);
}

/*********************************************************************
*
*       SCROLLBAR_SetDefaultSkinClassic
*/
void SCROLLBAR_SetDefaultSkinClassic(void) {
  SCROLLBAR__pSkinDefault = &SCROLLBAR__SkinClassic;
}

#else
  void SCROLLBAR_SkinClassic_C(void);
  void SCROLLBAR_SkinClassic_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
