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
File        : BUTTON_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "BUTTON_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   BUTTON_SKINFLEX_RADIUS
  #define BUTTON_SKINFLEX_RADIUS    4
#endif

#ifndef   BUTTON_SKINPROPS_PRESSED
  static BUTTON_SKINFLEX_PROPS _PropsPressed = {
    { 0x008B622C, 0x00BAB09E, 0x00EFD198 },
    { 0x00FCF4E5, 0x00F6E5C4 },
    { 0x00EFD198, 0x00DBB368 },
    BUTTON_SKINFLEX_RADIUS
  };
  #define BUTTON_SKINPROPS_PRESSED  &_PropsPressed
#endif

#ifndef   BUTTON_SKINPROPS_FOCUSSED
  static BUTTON_SKINFLEX_PROPS _PropsFocussed = {
    { 0x00B17F3C, 0x00FBD846, 0x00DFDFDF },
    { 0x00F3F3F3, 0x00ECECEC },
    { 0x00DFDFDF, 0x00D0D0D0 },
    BUTTON_SKINFLEX_RADIUS
  };
  #define BUTTON_SKINPROPS_FOCUSSED &_PropsFocussed
#endif

#ifndef   BUTTON_SKINPROPS_ENABLED
  static BUTTON_SKINFLEX_PROPS _PropsEnabled = {
    { 0x00707070, 0x00FCFCFC, 0x00DFDFDF },
    { 0x00F3F3F3, 0x00ECECEC },
    { 0x00DFDFDF, 0x00D0D0D0 },
    BUTTON_SKINFLEX_RADIUS
  };
  #define BUTTON_SKINPROPS_ENABLED  &_PropsEnabled
#endif

#ifndef   BUTTON_SKINPROPS_DISABLED
  static BUTTON_SKINFLEX_PROPS _PropsDisabled = {
    { 0x00B5B2AD, 0x00FCFCFC, 0x00F4F4F4 },
    { 0x00F4F4F4, 0x00F4F4F4 },
    { 0x00F4F4F4, 0x00F4F4F4 },
    BUTTON_SKINFLEX_RADIUS
  };
  #define BUTTON_SKINPROPS_DISABLED &_PropsDisabled
#endif

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static BUTTON_SKINFLEX_PROPS * const _apProps[] = {
  BUTTON_SKINPROPS_PRESSED,
  BUTTON_SKINPROPS_FOCUSSED,
  BUTTON_SKINPROPS_ENABLED,
  BUTTON_SKINPROPS_DISABLED
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       BUTTON_DrawSkinFlex
*/
int BUTTON_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  BUTTON_Obj * pObj;
  int y, BitmapIndex, ColorIndex, TextAlign, Radius;
  GUI_COLOR Color;
  WM_HMEM hDrawObj;
  GUI_RECT Rect;
  const BUTTON_SKINFLEX_PROPS * pProps;
  GUI_HMEM hText;
  const char * s;

  //
  // Get property pointer
  //
  pProps = _apProps[pDrawItemInfo->ItemIndex];
  //
  // Draw
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BACKGROUND:
    if (pProps->Radius) {
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
      GUI_SetColor(pProps->aColorFrame[2]);
      GUI_FillRect(pDrawItemInfo->x0 + 2, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->x0 + pProps->Radius - 1, pDrawItemInfo->y1 - pProps->Radius);
      GUI_FillRect(pDrawItemInfo->x1 - pProps->Radius + 1, pDrawItemInfo->y0 + pProps->Radius, pDrawItemInfo->x1 - 2, pDrawItemInfo->y1 - pProps->Radius);
      Radius = pProps->Radius;
    } else {
      //
      // Frame
      //
      for (y = 0; y < 2; y++) {
        Color = *(pProps->aColorFrame + y);
        GUI_SetColor(Color);
        GUI_DrawRect(pDrawItemInfo->x0 + y, pDrawItemInfo->y0 + y, pDrawItemInfo->x1 - y, pDrawItemInfo->y1 - y);
      }
      Radius = 2;
    }
    //
    // Background
    //
    GUI_DrawGradientV(pDrawItemInfo->x0 + Radius,
                      pDrawItemInfo->y0 + 2,
                      pDrawItemInfo->x1 - Radius,
                      ((pDrawItemInfo->y0 + pDrawItemInfo->y1) >> 1),
                      pProps->aColorUpper[0], pProps->aColorUpper[1]);
    GUI_DrawGradientV(pDrawItemInfo->x0 + Radius,
                      ((pDrawItemInfo->y0 + pDrawItemInfo->y1) >> 1) + 1,
                      pDrawItemInfo->x1 - Radius,
                      pDrawItemInfo->y1 - 2,
                      pProps->aColorLower[0], pProps->aColorLower[1]);
    break;
  case WIDGET_ITEM_DRAW_BITMAP:
    //
    // Bitmap
    //
    pObj = BUTTON_LOCK_H(pDrawItemInfo->hWin);
    if (pDrawItemInfo->ItemIndex == BUTTON_SKINFLEX_PI_PRESSED) {
      BitmapIndex = pObj->ahDrawObj[BUTTON_BI_PRESSED]  ? BUTTON_BI_PRESSED  : BUTTON_BI_UNPRESSED;
    } else {
      BitmapIndex = pObj->ahDrawObj[BUTTON_BI_DISABLED] ? BUTTON_BI_DISABLED : BUTTON_BI_UNPRESSED;
    }
    hDrawObj = pObj->ahDrawObj[BitmapIndex];
    GUI_UNLOCK_H(pObj);
    GUI_DRAW__Draw(hDrawObj, pDrawItemInfo->hWin, 0, 0);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    //
    // Button text
    //
    pObj = BUTTON_LOCK_H(pDrawItemInfo->hWin);
    hText = pObj->hpText;
    if (hText) {
      //
      // Set right text color
      //
      TextAlign = pObj->Props.Align;
      ColorIndex = (pDrawItemInfo->ItemIndex == BUTTON_SKINFLEX_PI_PRESSED) ? 1 : 
                   (pDrawItemInfo->ItemIndex == BUTTON_SKINFLEX_PI_ENABLED) ? 0 :
                   2;
      Color = pObj->Props.aTextColor[ColorIndex];
      GUI_SetFont(pObj->Props.pFont);
      GUI_UNLOCK_H(pObj);
      GUI_SetColor(Color);
      //
      // Calculate text rectangle
      //
      GUI_GetClientRect(&Rect);
      Rect.x0 += pProps->Radius;
      Rect.x1 -= pProps->Radius;
      Rect.y0 += 2;
      Rect.y1 -= 2;
      //
      // Draw text
      //
      GUI_SetTextMode(GUI_TM_TRANS);
      s = (const char *)GUI_LOCK_H(hText);
      GUI_DispStringInRect(s, &Rect, TextAlign);
      GUI_UNLOCK_H(s);
    } else {
      GUI_UNLOCK_H(pObj);
    }
    break;
  case WIDGET_ITEM_CREATE:
    WM_SetHasTrans(pDrawItemInfo->hWin);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       BUTTON_SetSkinFlexProps
*/
void BUTTON_SetSkinFlexProps(const BUTTON_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       BUTTON_GetSkinFlexProps
*/
void BUTTON_GetSkinFlexProps(BUTTON_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void BUTTON_SkinFlex_C(void);
  void BUTTON_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
