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
File        : CHECKBOX_SkinFlex.c
Purpose     : Implementation of framewindow skin
---------------------------END-OF-HEADER------------------------------
*/

#include "CHECKBOX_Private.h"
#include "GUI_Private.h"
#include "WM_Intern.h"

#if GUI_WINSUPPORT

/*********************************************************************
*
*       Defines (configuration defaults)
*
**********************************************************************
*/
#ifndef   CHECKBOX_SKINFLEX_SIZE
  #define CHECKBOX_SKINFLEX_SIZE 15
#endif

#ifndef   CHECKBOX_SKINPROPS_ENABLED
  static CHECKBOX_SKINFLEX_PROPS _PropsEnabled = {
    { 0x00707070, 0x00FCFCFC, 0x00B9B3AE },
    { 0x00D2D2D2, 0x00F3F3F3},
    0x00404040,
    CHECKBOX_SKINFLEX_SIZE,
  };
  #define CHECKBOX_SKINPROPS_ENABLED     &_PropsEnabled
#endif

#ifndef   CHECKBOX_SKINPROPS_DISABLED
  static CHECKBOX_SKINFLEX_PROPS _PropsDisabled = {
    { 0x00B5B2AD, 0x00FCFCFC, 0x00F4F4F4 },
    { 0x00F4F4F4, 0x00F4F4F4},
    0x008B622C,
    CHECKBOX_SKINFLEX_SIZE,
  };
  #define CHECKBOX_SKINPROPS_DISABLED    &_PropsDisabled
#endif

typedef struct {
  GUI_BITMAP     aBitmap[2];
  GUI_LOGPALETTE Palette[2];
  U8           * apData[2];
  GUI_COLOR      aColor[2];
} SKINPROP_BITMAP;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static CHECKBOX_SKINFLEX_PROPS * const _apProps[] = {
  CHECKBOX_SKINPROPS_ENABLED,
  CHECKBOX_SKINPROPS_DISABLED,
};

static SKINPROP_BITMAP _aBitmap[2];

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
static void _DrawBackground(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const CHECKBOX_SKINFLEX_PROPS * pProps) {
  int i;

  for (i = 0; i < GUI_COUNTOF(pProps->aColorFrame); i++) {
    GUI_SetColor(pProps->aColorFrame[i]);
    GUI_DrawRect(pDrawItemInfo->x0 + i, 
                 pDrawItemInfo->y0 + i, 
                 pDrawItemInfo->x0 + pProps->ButtonSize - i - 1, 
                 pDrawItemInfo->x0 + pProps->ButtonSize - i - 1);
  }
  GUI_DrawGradientV(pDrawItemInfo->x0 + i, 
                    pDrawItemInfo->y0 + i, 
                    pDrawItemInfo->x0 + pProps->ButtonSize - i - 1, 
                    pDrawItemInfo->x0 + pProps->ButtonSize - i - 1, 
                    pProps->aColorInner[0], pProps->aColorInner[1]);
}

/*********************************************************************
*
*       _SetBitmapPixel_1bpp
*/
static void _SetBitmapPixel_1bpp(int x, int y, int BytesPerLine,  U8 * pData) {
  U32 Off;
  U8 Mask;

  Off = BytesPerLine * y + (x >> 3);
  Mask = 0x80 >> (x & 7);
  *(pData + Off) |= Mask;
}

/*********************************************************************
*
*       _GenCheck
*/
static void _GenCheck(int b, int h, int s, int w, int Frame, int Dither, GUI_COLOR * pColor, GUI_BITMAP * pBitmap, GUI_LOGPALETTE * pPalette) {
  U8 * pData;
  int BytesPerLine, BytesRequired;
  int b2, h2, x, y, xs, i, State;

  //
  // Calculate check mark
  //
  b2 = b / 2;
  h2 = h / 2;
  y  = h2 - h2 / 2 + s - w / 2 + Frame;
  xs = b2 - s;
  //
  // Get memory for pixels
  //
  BytesPerLine  = (b + 7) / 8;
  BytesRequired = BytesPerLine * h;
  pData = (U8 *)GUI_ALLOC_GetFixedBlock(BytesRequired);
  GUI_MEMSET(pData, 0, BytesRequired);
  //
  // Set pixels
  //
  for (x = Frame; x < xs; x++, y++) {
    State = 1;
    for (i = y; i < (y + w); i++) {
      if (State && (i < h)) {
        _SetBitmapPixel_1bpp(x, i, BytesPerLine, pData);
      }
      State ^= Dither;
    }
  }
  for (; x < b - Frame; x++, y--) {
    State = 1;
    for (i = y; i < (y + w); i++) {
      if (State && (i < h)) {
        _SetBitmapPixel_1bpp(x, i, BytesPerLine, pData);
      }
      State ^= Dither;
    }
  }
  //
  // Setup palette structure
  //
  pPalette->NumEntries  = 2;
  pPalette->HasTrans    = 1;
  pPalette->pPalEntries = pColor;
  //
  // Setup bitmap structure
  //
  pBitmap->BitsPerPixel = 1;
  pBitmap->BytesPerLine = BytesPerLine;
  pBitmap->pData = pData;
  pBitmap->pMethods = NULL;
  pBitmap->pPal = pPalette;
  pBitmap->XSize = b;
  pBitmap->YSize = h;
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const CHECKBOX_SKINFLEX_PROPS * pProps, SKINPROP_BITMAP * pBitmap) {
  int Index, Size;

  Index = pDrawItemInfo->ItemIndex - 1;
  Size = pProps->ButtonSize - 6;
  //
  // Initialize bitmap on first draw
  //
  if (pBitmap->aBitmap[Index].pData == NULL) {
    pBitmap->aColor[1] = pProps->ColorCheck;
    _GenCheck(Size, Size, Size / 5, 3, 1, Index, pBitmap->aColor, &pBitmap->aBitmap[Index], &pBitmap->Palette[Index]);
  }
  //
  // Draw bitmap
  //
  GUI_DrawBitmap(&pBitmap->aBitmap[Index], 3, 3);
}

/*********************************************************************
*
*       _DrawText
*/
static void _DrawText(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const CHECKBOX_SKINFLEX_PROPS * pProps) {
  CHECKBOX_Obj * pObj;
  int Align, Spacing;
  GUI_RECT Rect;
  const char * s;
  const GUI_FONT GUI_UNI_PTR * pFont;
  GUI_COLOR Color;

  //
  // Get object properties
  //
  pObj = CHECKBOX_LOCK_H(pDrawItemInfo->hWin);
  Align = pObj->Props.Align;
  pFont = pObj->Props.pFont;
  Spacing = pObj->Props.Spacing;
  Color = pObj->Props.TextColor;
  GUI_UNLOCK_H(pObj);
  //
  // Draw text
  //
  WM_GetClientRect(&Rect);
  Rect.x0 += pProps->ButtonSize + Spacing;
  s = (const char *)pDrawItemInfo->p;
  GUI_SetFont(pFont);
  GUI_SetTextMode(GUI_TM_TRANS);
  GUI_SetColor(Color);
  GUI_DispStringInRect(s, &Rect, Align);
}

/*********************************************************************
*
*       _DrawFocus
*/
static void _DrawFocus(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo, const CHECKBOX_SKINFLEX_PROPS * pProps) {
  CHECKBOX_Obj * pObj;
  int Align, Spacing, xSizeText, ySizeText;
  GUI_RECT Rect;
  GUI_COLOR FocusColor;
  const char * s;
  const GUI_FONT GUI_UNI_PTR * pFont;

  //
  // Get object properties
  //
  pObj = CHECKBOX_LOCK_H(pDrawItemInfo->hWin);
  Align = pObj->Props.Align;
  pFont = pObj->Props.pFont;
  Spacing = pObj->Props.Spacing;
  FocusColor = pObj->Props.FocusColor;
  GUI_UNLOCK_H(pObj);
  //
  // Calculate rectangle
  //
  WM_GetClientRect(&Rect);
  Rect.x0 += pProps->ButtonSize + Spacing;
  s = (const char *)pDrawItemInfo->p;
  xSizeText = GUI_GetStringDistX(s);
  ySizeText = GUI_GetYSizeOfFont(pFont);
  switch (Align & ~(GUI_TA_HORIZONTAL)) {
  case GUI_TA_VCENTER:
    Rect.y0 = (Rect.y1 - ySizeText + 1) / 2;
    break;
  case GUI_TA_BOTTOM:
    Rect.y0 = Rect.y1 - ySizeText;
    break;
  }
  switch (Align & ~(GUI_TA_VERTICAL)) {
  case GUI_TA_HCENTER:
    Rect.x0 += ((Rect.x1 - Rect.x0) - xSizeText) / 2;
    break;
  case GUI_TA_RIGHT:
    Rect.x0 += (Rect.x1 - Rect.x0) - xSizeText;
    break;
  }
  Rect.x1 = Rect.x0 + xSizeText - 1;
  Rect.y1 = Rect.y0 + ySizeText - 1;
  //
  // Draw focus rectangle
  //
  GUI_SetColor(FocusColor);
  GUI_DrawFocusRect(&Rect, -1);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       CHECKBOX_DrawSkinFlex
*/
int CHECKBOX_DrawSkinFlex(const WIDGET_ITEM_DRAW_INFO * pDrawItemInfo) {
  const CHECKBOX_SKINFLEX_PROPS * pProps;
  SKINPROP_BITMAP * pBitmap;
  int Index, Size;

  //
  // Get property and bitmap index
  //
  if (pDrawItemInfo->hWin) {
    Index = WM__IsEnabled(pDrawItemInfo->hWin) ? 0 : 1;
  } else {
    Index = 0;
  }
  //
  // Get property pointers
  //
  pProps = _apProps[Index];
  pBitmap = &_aBitmap[Index];
  //
  // Initialize bitmap if required
  //
  if (pBitmap->aBitmap[Index].pData == NULL) {
    Size = pProps->ButtonSize - 6;
    pBitmap->aColor[1] = pProps->ColorCheck;
    _GenCheck(Size, Size, Size / 5, 3, 1, Index, pBitmap->aColor, &pBitmap->aBitmap[Index], &pBitmap->Palette[Index]);
  }
  //
  // Draw
  //
  switch (pDrawItemInfo->Cmd) {
  case WIDGET_ITEM_DRAW_BUTTON:
    _DrawBackground(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_DRAW_BITMAP:
    _DrawBitmap(pDrawItemInfo, pProps, pBitmap);
    break;
  case WIDGET_ITEM_DRAW_TEXT:
    _DrawText(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_DRAW_FOCUS:
    _DrawFocus(pDrawItemInfo, pProps);
    break;
  case WIDGET_ITEM_GET_BUTTONSIZE:
    return pProps->ButtonSize;
  case WIDGET_ITEM_CREATE:
    WM_SetHasTrans(pDrawItemInfo->hWin);
    break;
  }
  return 0;
}

/*********************************************************************
*
*       CHECKBOX_SetSkinFlexProps
*/
void CHECKBOX_SetSkinFlexProps(const CHECKBOX_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *_apProps[Index] = *pProps;
  }
}

/*********************************************************************
*
*       CHECKBOX_GetSkinFlexProps
*/
void CHECKBOX_GetSkinFlexProps(CHECKBOX_SKINFLEX_PROPS * pProps, int Index) {
  if (Index < GUI_COUNTOF(_apProps)) {
    *pProps = *_apProps[Index];
  }
}

#else
  void CHECKBOX_SkinFlex_C(void);
  void CHECKBOX_SkinFlex_C(void) {} /* avoid empty object files */
#endif /* GUI_WINSUPPORT */

/*************************** End of file ****************************/
