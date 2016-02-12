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
File        : HEADER_SkinFlex.c
Purpose     : Implementation of framewindow default skin
---------------------------END-OF-HEADER------------------------------
*/

#include "HEADER_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   HEADER_SKINPROPS
  static HEADER_SKINFLEX_PROPS _Props = {
    { 0x00707070, 0x00FCFCFC },
    { 0x00F3F3F3, 0x00ECECEC },
    { 0x00DFDFDF, 0x00D0D0D0 },
    0x00404040,
  };
  #define HEADER_SKINPROPS &_Props
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static HEADER_SKINFLEX_PROPS * const _pProps = HEADER_SKINPROPS;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBackground
*/
static void _DrawBackground(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int ym;

  ym = (pDrawItemInfo->y1 + pDrawItemInfo->y0) >> 1;
  GUI_SetColor(_pProps->aColorFrame[0]);
  GUI_DrawVLine(pDrawItemInfo->x1, pDrawItemInfo->y0, pDrawItemInfo->y1 - 1);
  GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0, pDrawItemInfo->x1);
  GUI_SetColor(_pProps->aColorFrame[1]);
  GUI_DrawVLine(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->y1 - 1);
  GUI_DrawGradientV(pDrawItemInfo->x0 + 1, pDrawItemInfo->y0, pDrawItemInfo->x1 - 1, ym - 1, _pProps->aColorUpper[0],  _pProps->aColorUpper[1]);
  GUI_DrawGradientV(pDrawItemInfo->x0 + 1, ym, pDrawItemInfo->x1 - 1, pDrawItemInfo->y1 - 1, _pProps->aColorLower[0],  _pProps->aColorLower[1]);
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  HEADER_Obj * pObj;
  HEADER_COLUMN * pColumn;
  GUI_ARRAY Columns;
  WM_HMEM hDrawObj;
  int xOff, yOff, Align, Width, xSize, ySize;

  //
  // Get widget properties
  //
  pObj    = HEADER_LOCK_H(pDrawItemInfo->hWin);
  Columns = pObj->Columns;
  GUI_UNLOCK_H(pObj);
  //
  // Get column properties
  //
  pColumn  = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, pDrawItemInfo->ItemIndex);
  Align    = pColumn->Align;
  Width    = pColumn->Width;
  hDrawObj = pColumn->hDrawObj;
  GUI_UNLOCK_H(pColumn);
  //
  // Get draw object properties
  //
  xSize = GUI_DRAW__GetXSize(hDrawObj);
  ySize = GUI_DRAW__GetYSize(hDrawObj);
  //
  // Calculate offset
  //
  xOff = yOff = 0;
  switch (Align & GUI_TA_HORIZONTAL) {
  case GUI_TA_RIGHT:
    xOff = (Width - xSize);
    break;
  case GUI_TA_HCENTER:
    xOff = (Width - xSize) / 2;
    break;
  }
  switch (Align & GUI_TA_VERTICAL) {
	case GUI_TA_BOTTOM:
    yOff = ((pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1) - ySize);
    break;
	case GUI_TA_VCENTER:
    yOff = ((pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1) - ySize) / 2;
    break;
  }
  //
  // Draw bitmap
  //
  GUI_DRAW__Draw(hDrawObj, pDrawItemInfo->hWin, pDrawItemInfo->x0 + xOff, pDrawItemInfo->y0 + yOff);
}

/*********************************************************************
*
*       _GetArrowPara
*/
static void _GetArrowPara(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, int * pArrowSize, int * pArrowPos) {
  *pArrowSize = ((pDrawItemInfo->y1 - pDrawItemInfo->y0 - 4) / 3) - 1;
  *pArrowPos  = pDrawItemInfo->x1 - 4 - *pArrowSize;
}

/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  HEADER_Obj * pObj;
  HEADER_COLUMN * pColumn;
  GUI_ARRAY Columns;
  int Align, DirIndicatorColumn, ArrowSize, ArrowPos;
  GUI_RECT Rect;
  GUI_COLOR TextColor;
  const GUI_FONT GUI_UNI_PTR * pFont;
  char * pText;

  //
  // Get widget properties
  //
  pObj               = HEADER_LOCK_H(pDrawItemInfo->hWin);
  Columns            = pObj->Columns;
  TextColor          = pObj->Props.TextColor;
  pFont              = pObj->Props.pFont;
  DirIndicatorColumn = pObj->DirIndicatorColumn;
  GUI_UNLOCK_H(pObj);
  //
  // Get column properties
  //
  pColumn = (HEADER_COLUMN *)GUI_ARRAY_GetpItemLocked(Columns, pDrawItemInfo->ItemIndex);
  Align   = pColumn->Align;
  pText   = pColumn->acText;
  GUI_UNLOCK_H(pColumn);
  //
  // 
  //
  Rect.x0 = pDrawItemInfo->x0;
  Rect.y0 = pDrawItemInfo->y0;
  Rect.x1 = pDrawItemInfo->x1;
  Rect.y1 = pDrawItemInfo->y1;
  GUI__ReduceRect(&Rect, &Rect, 2);
  if (DirIndicatorColumn == pDrawItemInfo->ItemIndex) {
    _GetArrowPara(pDrawItemInfo, &ArrowSize, &ArrowPos);
    Rect.x1 -= (ArrowSize << 1) + 1;
  }
  GUI_SetColor(TextColor);
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetFont(pFont);
  GUI_DispStringInRect(pText, &Rect, Align);
}

/*********************************************************************
*
*       _DrawTriangle
*/
static void _DrawTriangle(int x, int y, int Size, int Inc) {
  y += -Inc * (Size >> 1);
  for (; Size >= 0; Size--, y += Inc) {
    GUI_DrawHLine(y, x - Size, x + Size);
  }
}

/*********************************************************************
*
*       _DrawArrow
*/
static void _DrawArrow(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  HEADER_Obj * pObj;
  int DirIndicatorReverse, ArrowSize, ArrowPos;

  //
  // Get widget properties
  //
  pObj                = HEADER_LOCK_H(pDrawItemInfo->hWin);
  DirIndicatorReverse = pObj->DirIndicatorReverse;
  GUI_UNLOCK_H(pObj);
  //
  // Calculate arrow position
  //
  _GetArrowPara(pDrawItemInfo, &ArrowSize, &ArrowPos);
  //
  // Draw arrow
  //
  GUI_SetColor(_pProps->ColorArrow);
  _DrawTriangle(ArrowPos, ((pDrawItemInfo->y1 - pDrawItemInfo->y0) >> 1), ArrowSize, DirIndicatorReverse + 1 - DirIndicatorReverse * 3);
}

/*********************************************************************
*
*       _DrawOverlap
*/
static void _DrawOverlap(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  int ym;

  ym = (pDrawItemInfo->y1 + pDrawItemInfo->y0) >> 1;
  GUI_SetColor(_pProps->aColorFrame[0]);
  GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0, pDrawItemInfo->x1);
  GUI_SetColor(_pProps->aColorFrame[1]);
  GUI_DrawVLine(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->y1 - 1);
  GUI_DrawGradientV(pDrawItemInfo->x0 + 1, pDrawItemInfo->y0, pDrawItemInfo->x1, ym - 1, _pProps->aColorUpper[0],  _pProps->aColorUpper[1]);
  GUI_DrawGradientV(pDrawItemInfo->x0 + 1, ym, pDrawItemInfo->x1, pDrawItemInfo->y1 - 1, _pProps->aColorLower[0],  _pProps->aColorLower[1]);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       HEADER_DrawSkinFlex
*/
int HEADER_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    _DrawBackground(pDrawItemInfo);
    break;
  case WIDGET_ITEM_DRAW_BITMAP:
    _DrawBitmap(pDrawItemInfo);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    _DrawText(pDrawItemInfo);
    break;
  case WIDGET_ITEM_DRAW_ARROW:
    _DrawArrow(pDrawItemInfo);
    break;
  case WIDGET_ITEM_DRAW_OVERLAP:
    _DrawOverlap(pDrawItemInfo);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       HEADER_SetSkinFlexProps
*/
void HEADER_SetSkinFlexProps(const HEADER_SKINFLEX_PROPS * pProps, int Index) {
  if (Index == 0) {
    *_pProps = *pProps;
  }
}

/*********************************************************************
*
*       HEADER_GetSkinFlexProps
*/
void HEADER_GetSkinFlexProps(HEADER_SKINFLEX_PROPS * pProps, int Index) {
  if (Index == 0) {
    *pProps = *_pProps;
  }
}

#else
  void HEADER_SkinFlex_C(void);
  void HEADER_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
