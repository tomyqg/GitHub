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
File        : GUIAA_Char2.c
Purpose     : Display antialiased characters 2bpp
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawCharAA2_Trans
*
* Purpose:
*   Draws one character with transparency pixel by pixel
*/
static void _DrawCharAA2_Trans(int x0, int y0, int XSize, int YSize, int BytesPerLine, const U8 * pData, void (* pfSetPixelAA)(int x, int y, U8 Intens)) {
  const U8 * pData0;
  int x, y, xPos, yPos, RemPixels;
  U8 Data;
  const int aConvTable[4] = {0, 5, 10, 15};

  for (y = 0; y < YSize; y++) {
    x         = 0;
    pData0    = pData;
    RemPixels = XSize;
    yPos      = y0 + y;
    while (RemPixels >= 4) {
      xPos = x + x0;
      Data = *pData0++;
      pfSetPixelAA(xPos + 0, yPos, aConvTable[Data >> 6]);
      Data <<= 2;
      pfSetPixelAA(xPos + 1, yPos, aConvTable[Data >> 6]);
      Data <<= 2;
      pfSetPixelAA(xPos + 2, yPos, aConvTable[Data >> 6]);
      Data <<= 2;
      pfSetPixelAA(xPos + 3, yPos, aConvTable[Data >> 6]);
      RemPixels -= 4;
      x         += 4;
    }
    if (RemPixels) {
      Data = *pData0;
      while (RemPixels--) {
        pfSetPixelAA(x++ + x0, yPos, aConvTable[Data >> 6]);
        Data <<= 2;
      }
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       _DrawCharAA2_NoTrans
*
* Purpose:
*   Draws one character without transparency as a bitmap
*/
static void _DrawCharAA2_NoTrans(int x0, int y0, int XSize, int YSize, int BytesPerLine, const U8 * pData) {
  static LCD_COLOR      _aColor[4];
  static LCD_PIXELINDEX _OldColorIndex, _OldBkColorIndex;
  static GUI_LOGPALETTE _Palette = {4, 0, &_aColor[0]};
  static GUI_BITMAP     _Bitmap = {0, 0, 0, 2, 0, &_Palette, 0};
  int i;
  LCD_PIXELINDEX ColorIndex, BkColorIndex;
  LCD_COLOR Color, BkColor;

  //
  // Get current color indices
  //
  BkColorIndex = LCD__GetBkColorIndex();
  ColorIndex   = LCD__GetColorIndex();
  //
  // Calculate new palette for bitmap if required
  //
  if ((_OldColorIndex   != ColorIndex) || 
      (_OldBkColorIndex != BkColorIndex)) {
    BkColor    = LCD_Index2Color(BkColorIndex);
    Color      = LCD_Index2Color(ColorIndex);
    _aColor[0] = BkColor;
    for (i = 1; i < 3; i++) {
      _aColor[i] = LCD_AA_MixColors(Color, BkColor, 5 * i);
    }
    _aColor[3] = Color;
    LCD_GetpPalConvTableUncached(&_Palette);
    _OldColorIndex = ColorIndex;
    _OldBkColorIndex = BkColorIndex;
  }
  //
  // Set bitmap properties
  //
  _Bitmap.XSize        = XSize;
  _Bitmap.YSize        = YSize;
  _Bitmap.BytesPerLine = BytesPerLine;
  _Bitmap.pData        = pData;
  //
  // Set transparency flag
  //
  _Palette.HasTrans = GUI__CharHasTrans;
  //
  // Draw bitmap
  //
  GL_DrawBitmap(&_Bitmap, x0, y0);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AA__DrawCharAA2
*/
void GUI_AA__DrawCharAA2(int x0, int y0, int XSize, int YSize, int BytesPerLine, const U8 * pData) {
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    _DrawCharAA2_NoTrans(x0, y0, XSize, YSize, BytesPerLine, pData);
    break;
  case LCD_DRAWMODE_TRANS:
    _DrawCharAA2_Trans(x0, y0, XSize, YSize, BytesPerLine, pData, LCD_SetPixelAA);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    _DrawCharAA2_Trans(x0, y0, XSize, YSize, BytesPerLine, pData, LCD_SetPixelAA_Xor);
    break;
  }
}

/*********************************************************************
*
*       GUIPROP_AA2_DispChar
*/
void GUIPROP_AA2_DispChar(U16P c) {
  int BytesPerLine;
  GUI_DRAWMODE DrawMode, OldDrawMode;
  const GUI_FONT_PROP * pProp;
  const GUI_CHARINFO  * pCharInfo;

  DrawMode = GUI_Context.TextMode;
  pProp    = GUIPROP__FindChar(GUI_Context.pAFont->p.pProp, c);
  if (pProp) {
    pCharInfo    = pProp->paCharInfo + (c - pProp->First);
    BytesPerLine = pCharInfo->BytesPerLine;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    #if GUI_SUPPORT_ROTATION
    if (GUI_pLCD_APIList) {
      GUI_pLCD_APIList->pfDrawBitmap(GUI_Context.DispPosX,      // XPos
                                     GUI_Context.DispPosY,      // YPos
                                     pCharInfo->XSize,          // XSize
                                     GUI_Context.pAFont->YSize, // YSize
                                     1,                         // XMag
                                     1,                         // YMag
                                     2,                         // BitsPerPixel
                                     BytesPerLine,              // BytesPerLine
                                     pCharInfo->pData,          // Pointer to pixel data
                                     NULL);                     // Translation table
    } else {
    #else
    {
    #endif
      GUI_AA__DrawCharAA2(GUI_Context.DispPosX,          // XPos                 
                          GUI_Context.DispPosY,          // YPos                 
                          pCharInfo->XSize,              // XSize                
                          GUI_Context.pAFont->YSize,     // YSize                
                          BytesPerLine,                  // BytesPerLine         
                          (U8 const *)pCharInfo->pData); // Pointer to pixel data
    }
    LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
    GUI_Context.DispPosX += pCharInfo->XDist;
  }
}

/*********************************************************************
*
*       GUIPROP_AA2_GetCharDistX
*/
int GUIPROP_AA2_GetCharDistX(U16P c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp;

  pProp = GUIPROP__FindChar(GUI_Context.pAFont->p.pProp, c);
  return (pProp) ? (pProp->paCharInfo + (c - pProp->First))->XSize : 0;
}

/*********************************************************************
*
*       GUIPROP_AA2_GetFontInfo
*/
void GUIPROP_AA2_GetFontInfo(const GUI_FONT * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_PROP | GUI_FONTINFO_FLAG_AA2;
}

/*********************************************************************
*
*       GUIPROP_AA2_IsInFont
*/
char GUIPROP_AA2_IsInFont(const GUI_FONT * pFont, U16 c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp;

  pProp = GUIPROP__FindChar(pFont->p.pProp, c);
  return (pProp==NULL) ? 0 : 1;
}

/*************************** End of file ****************************/
