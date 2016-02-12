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
File        : SLIDER_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "SLIDER_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   SLIDER_SKINPROPS_TICKSIZE
  #define SLIDER_SKINPROPS_TICKSIZE    3
#endif
#ifndef   SLIDER_SKINPROPS_SHAFTSIZE
  #define SLIDER_SKINPROPS_SHAFTSIZE   3
#endif

#ifndef   SLIDER_SKINPROPS_PRESSED
  static SLIDER_SKINFLEX_PROPS _PropsPressed = {
    { 0x008B622C, 0x00BAB09E },
    { 0x00FCF4E5, 0x00DBB368 },
    { 0x00959595, 0x00F3F3F3, 0x00C0C0C0 },
    0x00404040,
    0x00000000,
    SLIDER_SKINPROPS_TICKSIZE,
    SLIDER_SKINPROPS_SHAFTSIZE
  };
  #define SLIDER_SKINPROPS_PRESSED     &_PropsPressed
#endif

#ifndef   SLIDER_SKINPROPS_UNPRESSED
  static SLIDER_SKINFLEX_PROPS _PropsUnpressed = {
    { 0x00707070, 0x00FCFCFC },
    { 0x00F2F2F2, 0x00BCBCBC  },
    { 0x00959595, 0x00F3F3F3, 0x00C0C0C0 },
    0x00404040,
    0x00000000,
    SLIDER_SKINPROPS_TICKSIZE,
    SLIDER_SKINPROPS_SHAFTSIZE
  };
  #define SLIDER_SKINPROPS_UNPRESSED   &_PropsUnpressed
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static SLIDER_SKINFLEX_PROPS * const _apProps[] = {
  SLIDER_SKINPROPS_PRESSED,
  SLIDER_SKINPROPS_UNPRESSED
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawShaftH
*/
static void _DrawShaftH(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  int y0, y1, h;

  h  = pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1 - pProps->TickSize - 1;
  y0 = pDrawItemInfo->y0 + pProps->TickSize + 1 + (h - pProps->ShaftSize) / 2;
  y1 = y0 + pProps->ShaftSize - 1;
  GUI_SetColor(pProps->aColorShaft[0]);
  GUI_DrawVLine(pDrawItemInfo->x0, y0, y1);
  GUI_DrawHLine(y0, pDrawItemInfo->x0 + 1, pDrawItemInfo->x1);
  GUI_SetColor(pProps->aColorShaft[1]);
  GUI_DrawVLine(pDrawItemInfo->x1, y0 + 1, y1 - 1);
  GUI_DrawHLine(y1, pDrawItemInfo->x0 + 1, pDrawItemInfo->x1);
  GUI_SetColor(pProps->aColorShaft[2]);
  GUI_FillRect(pDrawItemInfo->x0 + 1, y0 + 1, pDrawItemInfo->x1 - 1, y1 - 1);
}

/*********************************************************************
*
*       _DrawShaftV
*/
static void _DrawShaftV(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  int x0, x1, h;

  h  = pDrawItemInfo->x1 - pDrawItemInfo->x0 + 1 - pProps->TickSize - 1;
  x0 = pDrawItemInfo->x0 + (h - pProps->ShaftSize) / 2;
  x1 = x0 + pProps->ShaftSize - 1;
  GUI_SetColor(pProps->aColorShaft[0]);
  GUI_DrawHLine(pDrawItemInfo->y0, x0, x1);
  GUI_DrawVLine(x0, pDrawItemInfo->y0 + 1, pDrawItemInfo->y1);
  GUI_SetColor(pProps->aColorShaft[1]);
  GUI_DrawHLine(pDrawItemInfo->y1, x0 + 1, x1 - 1);
  GUI_DrawVLine(x1, pDrawItemInfo->y0 + 1, pDrawItemInfo->y1);
  GUI_SetColor(pProps->aColorShaft[2]);
  GUI_FillRect(x0 + 1, pDrawItemInfo->y0 + 1, x1 - 1, pDrawItemInfo->y1 - 1);
}

/*********************************************************************
*
*       _DrawThumbH
*/
static void _DrawThumbH(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  SLIDER_SKINFLEX_INFO * pSkinInfo;
  int x0, x1, y, i, w2;

  pSkinInfo = (SLIDER_SKINFLEX_INFO *)pDrawItemInfo->p;
  w2 = (pSkinInfo->Width + 1) >> 1;
  y  = pDrawItemInfo->y0 + pProps->TickSize + 1;
  GUI_SetColor(pProps->aColorFrame[0]);
  GUI_DrawLine(pDrawItemInfo->x0 + w2 - 1, y, pDrawItemInfo->x0, y + w2 - 1);
  GUI_DrawLine(pDrawItemInfo->x1 - w2 + 1, y, pDrawItemInfo->x1, y + w2 - 1);
  GUI_DrawVLine(pDrawItemInfo->x0, y + w2, pDrawItemInfo->y1);
  GUI_DrawVLine(pDrawItemInfo->x1, y + w2, pDrawItemInfo->y1);
  GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0 + 1, pDrawItemInfo->x1 - 1);
  GUI_SetColor(pProps->aColorFrame[1]);
  GUI_DrawLine(pDrawItemInfo->x0 + w2 - 1, y + 1, pDrawItemInfo->x0 + 1, y + w2 - 1);
  GUI_DrawLine(pDrawItemInfo->x1 - w2 + 1, y + 1, pDrawItemInfo->x1 - 1, y + w2 - 1);
  GUI_DrawVLine(pDrawItemInfo->x0 + 1, y + w2, pDrawItemInfo->y1 - 1);
  GUI_DrawVLine(pDrawItemInfo->x1 - 1, y + w2, pDrawItemInfo->y1 - 1);
  GUI_DrawHLine(pDrawItemInfo->y1 - 1, pDrawItemInfo->x0 + 2, pDrawItemInfo->x1 - 2);
  GUI_DrawGradientV(pDrawItemInfo->x0 + 2, y + w2 - 1, pDrawItemInfo->x1 - 2, pDrawItemInfo->y1 - 2, pProps->aColorInner[0], pProps->aColorInner[1]);
  GUI_SetColor(pProps->aColorInner[0]);
  for (i = 2; i < w2 - 1; i++) {
    x0 = pDrawItemInfo->x0 + w2 - i + 1;
    x1 = pDrawItemInfo->x1 - w2 + i - 1; 
    GUI_DrawHLine(y + i, x0, x1);
  }
}

/*********************************************************************
*
*       _DrawThumbV
*/
static void _DrawThumbV(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  SLIDER_SKINFLEX_INFO * pSkinInfo;
  int y0, y1, x, i, w2;

  pSkinInfo = (SLIDER_SKINFLEX_INFO *)pDrawItemInfo->p;
  w2 = (pSkinInfo->Width + 1) >> 1;
  x  = pDrawItemInfo->x1 - pProps->TickSize - 1;
  GUI_SetColor(pProps->aColorFrame[0]);
  GUI_DrawLine(x - w2 + 1, pDrawItemInfo->y0, x, pDrawItemInfo->y0 + w2 - 1);
  GUI_DrawLine(x - w2 + 1, pDrawItemInfo->y1, x, pDrawItemInfo->y1 - w2 + 1);
  GUI_DrawHLine(pDrawItemInfo->y0, pDrawItemInfo->x0, x - w2);
  GUI_DrawHLine(pDrawItemInfo->y1, pDrawItemInfo->x0, x - w2);
  GUI_DrawVLine(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->y1);
  GUI_SetColor(pProps->aColorFrame[1]);
  GUI_DrawLine(x - w2 + 1, pDrawItemInfo->y0 + 1, x - 1, pDrawItemInfo->y0 + w2 - 1);
  GUI_DrawLine(x - w2 + 1, pDrawItemInfo->y1 - 1, x - 1, pDrawItemInfo->y1 - w2 + 1);
  GUI_DrawHLine(pDrawItemInfo->y0 + 1, pDrawItemInfo->x0 + 1, x - w2);
  GUI_DrawHLine(pDrawItemInfo->y1 - 1, pDrawItemInfo->x0 + 1, x - w2);
  GUI_DrawVLine(pDrawItemInfo->x0 + 1, pDrawItemInfo->y0 + 1, pDrawItemInfo->y1 - 1);
  GUI_DrawGradientH(pDrawItemInfo->x0 + 2, pDrawItemInfo->y0 + 2, x - w2 + 1, pDrawItemInfo->y1 - 2, pProps->aColorInner[0], pProps->aColorInner[1]);
  GUI_SetColor(pProps->aColorInner[1]);
  for (i = 2; i < w2 - 1; i++) {
    y0 = pDrawItemInfo->y0 + w2 - i + 1;
    y1 = pDrawItemInfo->y1 - w2 + i - 1; 
    GUI_DrawVLine(x - i, y0, y1);
  }
}

/*********************************************************************
*
*       _DrawTicksH
*/
static void _DrawTicksH(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  SLIDER_SKINFLEX_INFO * pSkinInfo;
  int i, x;

  GUI_SetColor(pProps->ColorTick);
  pSkinInfo = (SLIDER_SKINFLEX_INFO *)pDrawItemInfo->p;
  for (i = 0; i < pSkinInfo->NumTicks; i++) {
    x = pSkinInfo->Width / 2 + pDrawItemInfo->x0 + pSkinInfo->Size * i / (pSkinInfo->NumTicks - 1);
    GUI_DrawVLine(x, pDrawItemInfo->y0, pDrawItemInfo->y0 + pProps->TickSize - 1);
  }
}

/*********************************************************************
*
*       _DrawTicksV
*/
static void _DrawTicksV(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  SLIDER_SKINFLEX_INFO * pSkinInfo;
  int i, y;

  GUI_SetColor(pProps->ColorTick);
  pSkinInfo = (SLIDER_SKINFLEX_INFO *)pDrawItemInfo->p;
  for (i = 0; i < pSkinInfo->NumTicks; i++) {
    y = pSkinInfo->Width / 2 + pDrawItemInfo->y0 + pSkinInfo->Size * i / (pSkinInfo->NumTicks - 1);
    GUI_DrawHLine(y, pDrawItemInfo->x1 - pProps->TickSize + 1, pDrawItemInfo->x1);
  }
}

/*********************************************************************
*
*       _DrawFocusRect
*/
static void _DrawFocusRect(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const SLIDER_SKINFLEX_PROPS * pProps) {
  GUI_RECT Rect;

  GUI_SetColor(pProps->ColorFocus);
  Rect.x0 = pDrawItemInfo->x0;
  Rect.y0 = pDrawItemInfo->y0;
  Rect.x1 = pDrawItemInfo->x1;
  Rect.y1 = pDrawItemInfo->y1;
  GUI_DrawFocusRect(&Rect, 0);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SLIDER_DrawSkinFlex
*/
int SLIDER_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  const SLIDER_SKINFLEX_PROPS * pProps;
  SLIDER_SKINFLEX_INFO * pSkinInfo;

  pSkinInfo = (SLIDER_SKINFLEX_INFO *)pDrawItemInfo->p;
  if (pSkinInfo) {
    pProps = (pSkinInfo->IsPressed) ? _apProps[SLIDER_SKINFLEX_PI_PRESSED] : _apProps[SLIDER_SKINFLEX_PI_UNPRESSED];
  }
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_SHAFT:
    if (pSkinInfo->IsVertical) {
      _DrawShaftV(pDrawItemInfo, pProps);
    } else {
      _DrawShaftH(pDrawItemInfo, pProps);
    }
    break;
  case WIDGET_ITEM_DRAW_THUMB:
    if (pSkinInfo->IsVertical) {
      _DrawThumbV(pDrawItemInfo, pProps);
    } else {
      _DrawThumbH(pDrawItemInfo, pProps);
    }
    break;
  case WIDGET_ITEM_DRAW_TICKS:
    if (pSkinInfo->IsVertical) {
      _DrawTicksV(pDrawItemInfo, pProps);
    } else {
      _DrawTicksH(pDrawItemInfo, pProps);
    }
    break;
  case WIDGET_ITEM_DRAW_FOCUS:
    _DrawFocusRect(pDrawItemInfo, pProps);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       SLIDER_SetSkinFlexProps
*/
void SLIDER_SetSkinFlexProps(const SLIDER_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       SLIDER_GetSkinFlexProps
*/
void SLIDER_GetSkinFlexProps(SLIDER_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void SLIDER_SkinFlex_C(void);
  void SLIDER_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
