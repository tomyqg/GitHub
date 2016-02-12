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
File        : DROPDOWN_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "DROPDOWN_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   DROPDOWN_SKINFLEX_RADIUS
  #define DROPDOWN_SKINFLEX_RADIUS    2
#endif

#ifndef   DROPDOWN_SKINPROPS_OPEN
  static DROPDOWN_SKINFLEX_PROPS _PropsOpen = {
    { 0x008B622C, 0x00BAB09E, 0x00EFD198 },
    { 0x00FCF4E5, 0x00F6E5C4 },
    { 0x00EFD198, 0x00DBB368 },
    0x00404040,
    0x00000000,
    0x008B622C,
    DROPDOWN_SKINFLEX_RADIUS
  };
  #define DROPDOWN_SKINPROPS_OPEN     &_PropsOpen
#endif

#ifndef   DROPDOWN_SKINPROPS_FOCUSSED
  static DROPDOWN_SKINFLEX_PROPS _PropsFocussed = {
    { 0x00B17F3C, 0x00FBD846, 0x00DFDFDF },
    { 0x00F3F3F3, 0x00ECECEC },
    { 0x00DFDFDF, 0x00D0D0D0 },
    0x00404040,
    0x00000000,
    0x00B17F3C,
    DROPDOWN_SKINFLEX_RADIUS
  };
  #define DROPDOWN_SKINPROPS_FOCUSSED &_PropsFocussed
#endif

#ifndef   DROPDOWN_SKINPROPS_ENABLED
  static DROPDOWN_SKINFLEX_PROPS _PropsEnabled = {
    { 0x00707070, 0x00FCFCFC, 0x00DFDFDF },
    { 0x00F3F3F3, 0x00ECECEC },
    { 0x00DFDFDF, 0x00D0D0D0 },
    0x00404040,
    0x00000000,
    0x00707070,
    DROPDOWN_SKINFLEX_RADIUS
  };
  #define DROPDOWN_SKINPROPS_ENABLED  &_PropsEnabled
#endif

#ifndef   DROPDOWN_SKINPROPS_DISABLED
  static DROPDOWN_SKINFLEX_PROPS _PropsDisabled = {
    { 0x00B5B2AD, 0x00FCFCFC, 0x00F4F4F4 },
    { 0x00F4F4F4, 0x00F4F4F4 },
    { 0x00F4F4F4, 0x00F4F4F4 },
    0x00404040,
    0x00000000,
    0x00B5B2AD,
    DROPDOWN_SKINFLEX_RADIUS
  };
  #define DROPDOWN_SKINPROPS_DISABLED &_PropsDisabled
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static DROPDOWN_SKINFLEX_PROPS * const _apProps[] = {
  DROPDOWN_SKINPROPS_OPEN,
  DROPDOWN_SKINPROPS_FOCUSSED,
  DROPDOWN_SKINPROPS_ENABLED,
  DROPDOWN_SKINPROPS_DISABLED
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawTriangleDown
*/
static void _DrawTriangleDown(int x, int y, int Size) {
  for (; Size >= 0; Size--, y++ ) {
    GUI_DrawHLine(y, x - Size, x + Size);
  }
}

/*********************************************************************
*
*       _DrawBackground
*/
static void _DrawBackground(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const DROPDOWN_SKINFLEX_PROPS * pProps) {
  int y;
  GUI_COLOR Color;
  
  //
  // Edges
  //
  GUI__DrawTwinArc4(pDrawItemInfo->x0, pDrawItemInfo->y0, pDrawItemInfo->x1, pDrawItemInfo->y1, pProps->Radius, pProps->aColorFrame[0], pProps->aColorFrame[1], pProps->aColorFrame[2]);
  //
  // Frame
  //
  for (y = 0; y < 2; y++) {
    Color = *(pProps->aColorFrame + y);
    GUI_SetColor(Color);
    GUI_DrawHLine(pDrawItemInfo->y0 + y, pDrawItemInfo->x0 + pProps->Radius, pDrawItemInfo->x1 - pProps->Radius);
    GUI_DrawHLine(pDrawItemInfo->y1 - y, pDrawItemInfo->x0 + pProps->Radius, pDrawItemInfo->x1 - pProps->Radius);
    GUI_DrawVLine(pDrawItemInfo->x0 + y, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->y1 - pProps->Radius);
    GUI_DrawVLine(pDrawItemInfo->x1 - y, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->y1 - pProps->Radius);
  }
  if (pProps->Radius > 2) {
    GUI_SetColor(pProps->aColorFrame[2]);
    GUI_FillRect(pDrawItemInfo->x0 + 2, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->x0 + pProps->Radius - 1, pDrawItemInfo->y1 - pProps->Radius);
    GUI_FillRect(pDrawItemInfo->x1 - pProps->Radius + 1, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->x1 - 2, pDrawItemInfo->y1 - pProps->Radius);
  }
  //
  // Background
  //
  GUI_DrawGradientV(pDrawItemInfo->x0 + pProps->Radius,
                    pDrawItemInfo->y0 + 2,
                    pDrawItemInfo->x1 - pProps->Radius,
                    ((pDrawItemInfo->y0 + pDrawItemInfo->y1) >> 1),
                    pProps->aColorUpper[0], pProps->aColorUpper[1]);
  GUI_DrawGradientV(pDrawItemInfo->x0 + pProps->Radius,
                    ((pDrawItemInfo->y0 + pDrawItemInfo->y1) >> 1) + 1,
                    pDrawItemInfo->x1 - pProps->Radius,
                    pDrawItemInfo->y1 - 2,
                    pProps->aColorLower[0], pProps->aColorLower[1]);
  //
  // Separator
  //
  GUI_SetColor(pProps->ColorSep);
  GUI_DrawVLine(pDrawItemInfo->x1 - (pDrawItemInfo->y1 - pDrawItemInfo->y0) + 1, pDrawItemInfo->y0 + 4, pDrawItemInfo->y1 - 4);
}

/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const DROPDOWN_SKINFLEX_PROPS * pProps) {
  const char * s;
  DROPDOWN_Obj * pObj;
  const GUI_FONT GUI_UNI_PTR * pFont;
  int Align, TextBorderSize;
  GUI_RECT Rect;

  //
  // Get object properties
  //
  pObj = DROPDOWN_LOCK_H(pDrawItemInfo->hWin);
  TextBorderSize = pObj->Props.TextBorderSize;
  pFont = pObj->Props.pFont;
  Align = pObj->Props.Align;
  GUI_UNLOCK_H(pObj);
  //
  // Draw
  //
  Rect.x0 = pDrawItemInfo->x0 + 2 + TextBorderSize;
  Rect.y0 = pDrawItemInfo->y0 + 2;
  Rect.x1 = pDrawItemInfo->x1 - 2 - TextBorderSize - (pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1);
  Rect.y1 = pDrawItemInfo->y1 - 2;
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetFont(pFont);
  GUI_SetColor(pProps->ColorText);
  s = (const char *)pDrawItemInfo->p;
  GUI_DispStringInRect(s, &Rect, Align);
}

/*********************************************************************
*
*       _DrawArrow
*/
static void _DrawArrow(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const DROPDOWN_SKINFLEX_PROPS * pProps) {
  int xPos, yPos, ySize;

  ySize = ((pDrawItemInfo->y1 - pDrawItemInfo->y0 - 8) >> 1) - 1;
  xPos  = pDrawItemInfo->x1 - ((pDrawItemInfo->y1 - pDrawItemInfo->y0 + 1) >> 1);
  yPos  = ((pDrawItemInfo->y1 - pDrawItemInfo->y0 - ySize) >> 1);
  GUI_SetColor(pProps->ColorArrow);
  _DrawTriangleDown(xPos, yPos, ySize);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       DROPDOWN_DrawSkinFlex
*/
int DROPDOWN_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  DROPDOWN_Obj * pObj;
  const DROPDOWN_SKINFLEX_PROPS * pProps;
  WM_HWIN hListWin;
  int State, IsOpen, IsFocussed, IsEnabled, Index;

  //
  // Get object properties
  //
  pObj = DROPDOWN_LOCK_H(pDrawItemInfo->hWin);
  State = pObj->Widget.State;
  hListWin = pObj->hListWin;
  GUI_UNLOCK_H(pObj);
  //
  // Get state
  //
  IsOpen     = hListWin ? 1 : 0;
  IsFocussed = (State & WIDGET_STATE_FOCUS) ? 1 : 0;
  IsEnabled  = WM__IsEnabled(pDrawItemInfo->hWin);
  //
  // Get property pointer
  //
  if (IsOpen) {
    Index = DROPDOWN_SKINFLEX_PI_EXPANDED;
  } else if (IsFocussed) {
    Index = DROPDOWN_SKINFLEX_PI_FOCUSSED;
  } else if (IsEnabled) {
    Index = DROPDOWN_SKINFLEX_PI_ENABLED;
  } else {
    Index = DROPDOWN_SKINFLEX_PI_DISABLED;
  }
  pProps = _apProps[Index];
  //
  // Draw
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    _DrawBackground(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    _DrawText(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_DRAW_ARROW:
    _DrawArrow(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_CREATE:
    WM_SetHasTrans(pDrawItemInfo->hWin);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       DROPDOWN_SetSkinFlexProps
*/
void DROPDOWN_SetSkinFlexProps(const DROPDOWN_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       DROPDOWN_GetSkinFlexProps
*/
void DROPDOWN_GetSkinFlexProps(DROPDOWN_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void DROPDOWN_SkinFlex_C(void);
  void DROPDOWN_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
