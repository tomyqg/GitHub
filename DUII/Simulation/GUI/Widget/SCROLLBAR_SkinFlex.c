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
File        : SCROLLBAR_SkinFlex.c
Purpose     : Implementation of framewindow default skin
---------------------------END-OF-HEADER------------------------------
*/

#include "SCROLLBAR_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   SCROLLBAR_SKINPROPS_PRESSED
  static SCROLLBAR_SKINFLEX_PROPS _PropsPressed = {
    { 0x00B17F3C, 0x00FFFEFD, 0x00E0E0E0 },
    { 0x00FCF4E3, 0x00FBEED6 },
    { 0x00F6DBA9, 0x00E3CA9C },
    { 0x00E0E0E0, 0x00F0F0F0 },
    0x00000000,
    0x00A37D37
  };
  #define SCROLLBAR_SKINPROPS_PRESSED   &_PropsPressed
#endif

#ifndef   SCROLLBAR_SKINPROPS_UNPRESSED
  static SCROLLBAR_SKINFLEX_PROPS _PropsUnpressed = {
    { 0x00959595, 0x00F3F3F3, 0x00E0E0E0 },
    { 0x00F2F2F2, 0x00E8E8E8 },
    { 0x00D8D8D8, 0x00BCBCBC },
    { 0x00E0E0E0, 0x00F0F0F0 },
    0x00404040,
    0x00404040,
  };
  #define SCROLLBAR_SKINPROPS_UNPRESSED &_PropsUnpressed
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static SCROLLBAR_SKINFLEX_PROPS * const _apProps[] = {
  SCROLLBAR_SKINPROPS_PRESSED,
  SCROLLBAR_SKINPROPS_UNPRESSED
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawFrame
*/
static void _DrawFrame(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps) {
  GUI_SetColor(pProps->aColorFrame[0]);
  GUI_DrawHLine(pRect->y0, pRect->x0 + 1, pRect->x1 - 1);
  GUI_DrawHLine(pRect->y1, pRect->x0 + 1, pRect->x1 - 1);
  GUI_DrawVLine(pRect->x0, pRect->y0 + 1, pRect->y1 - 1);
  GUI_DrawVLine(pRect->x1, pRect->y0 + 1, pRect->y1 - 1);
  GUI_SetColor(pProps->aColorFrame[1]);
  GUI_DrawHLine(pRect->y0 + 1, pRect->x0 + 1, pRect->x1 - 1);
  GUI_DrawHLine(pRect->y1 - 1, pRect->x0 + 1, pRect->x1 - 1);
  GUI_DrawVLine(pRect->x0 + 1, pRect->y0 + 2, pRect->y1 - 2);
  GUI_DrawVLine(pRect->x1 - 1, pRect->y0 + 2, pRect->y1 - 2);
  GUI_SetColor(pProps->aColorFrame[2]);
  GUI_DrawPixel(pRect->x0, pRect->y0);
  GUI_DrawPixel(pRect->x0, pRect->y1);
  GUI_DrawPixel(pRect->x1, pRect->y0);
  GUI_DrawPixel(pRect->x1, pRect->y1);
}

/*********************************************************************
*
*       _DrawBackgroundV
*/
static void _DrawBackgroundV(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps) {
  _DrawFrame(pRect, pProps);
  GUI_DrawGradientH(pRect->x0 + 2,                      pRect->y0 + 2, ((pRect->x0 + pRect->x1) >> 1), pRect->y1 - 2, pProps->aColorUpper[0], pProps->aColorUpper[1]);
  GUI_DrawGradientH(((pRect->x0 + pRect->x1) >> 1) + 1, pRect->y0 + 2, pRect->x1 - 2,                  pRect->y1 - 2, pProps->aColorLower[0], pProps->aColorLower[1]);
}

/*********************************************************************
*
*       _DrawBackgroundH
*/
static void _DrawBackgroundH(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps) {
  _DrawFrame(pRect, pProps);
  GUI_DrawGradientV(pRect->x0 + 2, pRect->y0 + 2,                      pRect->x1 - 2, ((pRect->y0 + pRect->y1) >> 1), pProps->aColorUpper[0], pProps->aColorUpper[1]);
  GUI_DrawGradientV(pRect->x0 + 2, ((pRect->y0 + pRect->y1) >> 1) + 1, pRect->x1 - 2, pRect->y1 - 2,                  pProps->aColorLower[0], pProps->aColorLower[1]);
}

/*********************************************************************
*
*       _GetHeight
*/
static int _GetHeight(GUI_RECT * pRect, int IsVertical) {
  int r;

  if (IsVertical) {
    r = pRect->x1 - pRect->x0 + 1;
  } else {
    r = pRect->y1 - pRect->y0 + 1;
  }
  return r;
}

/*********************************************************************
*
*       _GetLength
*/
static int _GetLength(GUI_RECT * pRect, int IsVertical) {
  int r;

  if (IsVertical) {
    r = pRect->y1 - pRect->y0 + 1;
  } else {
    r = pRect->x1 - pRect->x0 + 1;
  }
  return r;
}

/*********************************************************************
*
*       _GetArrowPara
*/
static void _GetArrowPara(GUI_RECT * pRect, int IsVertical, int * pArrowSize, int * pArrowOff) {
  int ButtonSize;

  ButtonSize  = _GetHeight(pRect, IsVertical);
  *pArrowSize = (ButtonSize - 4) / 3;
  *pArrowOff  = ButtonSize - ((ButtonSize - *pArrowSize) >> 1) - 1;
}

/*********************************************************************
*
*       _DrawGraspH
*/
static void _DrawGraspH(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps) {
  int xm, o, y0, y1;

  xm = (pRect->x1 + pRect->x0) >> 1;
  o = ((pRect->y1 - pRect->y0) + 3) >> 2;
  y0 = pRect->y0 + o;
  y1 = pRect->y1 - o;
  GUI_SetColor(pProps->ColorGrasp);
  GUI_DrawVLine(xm + 0, y0, y1);
  GUI_DrawVLine(xm + 2, y0, y1);
  GUI_DrawVLine(xm + 4, y0, y1);
  GUI_DrawVLine(xm - 2, y0, y1);
  GUI_DrawVLine(xm - 4, y0, y1);
}

/*********************************************************************
*
*       _DrawGraspV
*/
static void _DrawGraspV(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps) {
  int ym, o, x0, x1;

  ym = (pRect->y1 + pRect->y0) >> 1;
  o = ((pRect->x1 - pRect->x0) + 3) >> 2;
  x0 = pRect->x0 + o;
  x1 = pRect->x1 - o;
  GUI_SetColor(pProps->ColorGrasp);
  GUI_DrawHLine(ym + 0, x0, x1);
  GUI_DrawHLine(ym + 2, x0, x1);
  GUI_DrawHLine(ym + 4, x0, x1);
  GUI_DrawHLine(ym - 2, x0, x1);
  GUI_DrawHLine(ym - 4, x0, x1);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       SCROLLBAR_DrawSkinFlex
*/
int SCROLLBAR_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  SCROLLBAR_OBJ * pObj;
  int ArrowSize, ArrowOff;
  const SCROLLBAR_SKINFLEX_PROPS * pProps;
  GUI_RECT Rect, RectOrg;
  void (* pfDrawBackground)(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps);
  void (* pfDrawGradient)(int x0, int y0, int x1, int y1, GUI_COLOR Color0, GUI_COLOR Color1);
  void (* pfDrawGrasp)(GUI_RECT * pRect, const SCROLLBAR_SKINFLEX_PROPS * pProps);
  WIDGET * pWidget;
  SCROLLBAR_SKINFLEX_INFO * pSkinInfo;

  //
  // Get object properties
  //
  pObj = SCROLLBAR_LOCK_H(pDrawItemInfo->hWin);
  pWidget = &pObj->Widget;
  GUI_UNLOCK_H(pObj);
  pSkinInfo = (SCROLLBAR_SKINFLEX_INFO *)pDrawItemInfo->p;
  //
  // Get rectangle to be filled
  //
  Rect.x0 = pDrawItemInfo->x0;
  Rect.y0 = pDrawItemInfo->y0;
  Rect.x1 = pDrawItemInfo->x1;
  Rect.y1 = pDrawItemInfo->y1;
  RectOrg = Rect;
  //
  // Get function pointers
  //
  if (pSkinInfo->IsVertical) {
    WIDGET__RotateRect90(pWidget, &Rect, &Rect);
    pfDrawBackground = _DrawBackgroundV;
    pfDrawGradient   = GUI_DrawGradientH;
    pfDrawGrasp      = _DrawGraspV;
  } else {
    pfDrawBackground = _DrawBackgroundH;
    pfDrawGradient   = GUI_DrawGradientV;
    pfDrawGrasp      = _DrawGraspH;
  }
  //
  // Draw
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_GET_BUTTONSIZE:
    if (pSkinInfo->IsVertical) {
      return pDrawItemInfo->x1 - pDrawItemInfo->x0 + 1;
    } else {
      return pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1;
    }
  case WIDGET_ITEM_DRAW_BUTTON_L:
    //
    // Get right parameter pointer
    //
    pProps = (pSkinInfo->State == PRESSED_STATE_LEFT) ? _apProps[SCROLLBAR_SKINFLEX_PI_PRESSED] : _apProps[SCROLLBAR_SKINFLEX_PI_UNPRESSED];
    //
    // Draw background
    //
    pfDrawBackground(&Rect, pProps);
    //
    // Draw arrow
    //
    _GetArrowPara(&RectOrg, pSkinInfo->IsVertical, &ArrowSize, &ArrowOff);
    GUI_SetColor(pProps->ColorArrow);
    WIDGET__DrawTriangle(pWidget, RectOrg.x0 + ArrowOff, (RectOrg.y1 - RectOrg.y0 + 1) >> 1, ArrowSize, -1);
    break;
  case WIDGET_ITEM_DRAW_OVERLAP:
  case WIDGET_ITEM_DRAW_SHAFT_L:
  case WIDGET_ITEM_DRAW_SHAFT_R:
    //
    // Get right parameter pointer
    //
    pProps = _apProps[SCROLLBAR_SKINFLEX_PI_UNPRESSED];
    //
    // Draw background
    //
    pfDrawGradient(Rect.x0, Rect.y0, Rect.x1, Rect.y1, pProps->aColorShaft[0], pProps->aColorShaft[1]);
    break;
  case WIDGET_ITEM_DRAW_THUMB:
    //
    // Get right parameter pointer
    //
    pProps = (pSkinInfo->State == PRESSED_STATE_THUMB) ? _apProps[SCROLLBAR_SKINFLEX_PI_PRESSED] : _apProps[SCROLLBAR_SKINFLEX_PI_UNPRESSED];
    //
    // Draw background
    //
    pfDrawBackground(&Rect, pProps);
    //
    // Draw grasp
    //
    if (_GetLength(&Rect, pSkinInfo->IsVertical) >= 15) {
      pfDrawGrasp(&Rect, pProps);
    }
    break;
  case WIDGET_ITEM_DRAW_BUTTON_R:
    //
    // Get right parameter pointer
    //
    pProps = (pSkinInfo->State == PRESSED_STATE_RIGHT) ? _apProps[SCROLLBAR_SKINFLEX_PI_PRESSED] : _apProps[SCROLLBAR_SKINFLEX_PI_UNPRESSED];
    //
    // Draw background
    //
    pfDrawBackground(&Rect, pProps);
    //
    // Draw arrow
    //
    _GetArrowPara(&RectOrg, pSkinInfo->IsVertical, &ArrowSize, &ArrowOff);
    GUI_SetColor(pProps->ColorArrow);
    WIDGET__DrawTriangle(pWidget, RectOrg.x1 - ArrowOff, (RectOrg.y1 - RectOrg.y0) >> 1, ArrowSize, 1);
    break;
  }

  return 0;
}

/*********************************************************************
*
*       SCROLLBAR_SetSkinFlexProps
*/
void SCROLLBAR_SetSkinFlexProps(const SCROLLBAR_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       SCROLLBAR_GetSkinFlexProps
*/
void SCROLLBAR_GetSkinFlexProps(SCROLLBAR_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void SCROLLBAR_SkinFlex_C(void);
  void SCROLLBAR_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
