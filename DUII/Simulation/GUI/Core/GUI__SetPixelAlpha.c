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
File        : GUI__SetPixelAlpha.c
Purpose     : Implementation of said function
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       GUI__MixColors
*/
LCD_COLOR GUI__MixColors(LCD_COLOR Color, LCD_COLOR BkColor, U8 Intens) {
  #if 1

  U32 r, g, b, a;
  U8 IntensBk;

  if ((BkColor & 0xFF000000) != 0xFF000000) {
    //
    // Calculate Color separations for FgColor first
    //
    r = (Color & 0x000000ff) * Intens;
    g = (Color & 0x0000ff00) * Intens;
    b = (Color & 0x00ff0000) * Intens;
    a = ((Color & 0xff000000) >> 24) * Intens;
    //
    // Add Color separations for BkColor
    //
    IntensBk = 255 - Intens;
    r += (BkColor & 0x000000ff) * IntensBk;
    g += (BkColor & 0x0000ff00) * IntensBk;
    b += (BkColor & 0x00ff0000) * IntensBk;
    a += ((BkColor & 0xff000000) >> 24) * IntensBk;
    r = (r >> 8) & 0x000000ff;
    g = (g >> 8) & 0x0000ff00;
    b = (b >> 8) & 0x00ff0000;
    a = (a << (24 - 8)) & 0xff000000;
    Color = r | g | b | a;
  }
  return Color;

  #else

  U32 r, g, b;
  U8 IntensBk;

  //
  // Calculate Color separations for FgColor first
  //
  r = (Color & 0x0000ff) * Intens;
  g = (Color & 0x00ff00) * Intens;
  b = (Color & 0xff0000) * Intens;
  //
  // Add Color separations for BkColor
  //
  IntensBk = 255 - Intens;
  //Intens = 255 - Intens;
  r += (BkColor & 0x0000ff) * IntensBk;
  g += (BkColor & 0x00ff00) * IntensBk;
  b += (BkColor & 0xff0000) * IntensBk;
  r = (r >> 8) & 0x0000ff;
  g = (g >> 8) & 0x00ff00;
  b = (b >> 8) & 0xff0000;
  #if 0
  {
    U8 Alpha;

    Color = r + g + b;
    Alpha = BkColor >> 24;
    if (Alpha == 0xFF) {
      Color |= (U32)IntensBk << 24;
    } else {
      Color |= (U32)((Alpha + IntensBk) >> 1) << 24;
    }
  }
  #else
    Color = r | g | b | (BkColor & 0xFF000000);
  #endif
  return Color;

  #endif
}

/*********************************************************************
*
*       GUI__SetPixelAlpha
*/
void GUI__SetPixelAlpha(int x, int y, U8 Alpha, LCD_COLOR Color) {
  LCD_COLOR BkColor;
  LCD_PIXELINDEX Index;

  if ((y >= GUI_Context.ClipRect.y0) && (y <= GUI_Context.ClipRect.y1)) {
    if ((x >= GUI_Context.ClipRect.x0) && (x <= GUI_Context.ClipRect.x1)) {
      BkColor = LCD_GetPixelColor(x, y);
      Color   = GUI__MixColors(Color, BkColor, Alpha);
      Index   = LCD_Color2Index(Color);
      LCDDEV_L0_SetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y, Index);
    }
  }
}

/*************************** End of file ****************************/
