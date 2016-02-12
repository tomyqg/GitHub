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
File        : LCD_RotateCW.c
Purpose     : Implementation of GUI_SetRotation
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

#if GUI_SUPPORT_ROTATION

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetpfSetPixel
*/
static tLCD_SetPixelAA * _GetpfSetPixel(void) {
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    return &LCD_SetPixelAA_NoTrans;
  case LCD_DRAWMODE_TRANS:
    return &LCD_SetPixelAA;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    return &LCD_SetPixelAA_Xor;
  }
  return NULL; // Never reach...
}

/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void  _DrawBitLine1BPP(int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Index0, Index1;
  unsigned IndexMask, Pixel;

  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  y += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    do {
      LCDDEV_L0_SetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
      y++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*p & (0x80 >> Diff)) {
        LCDDEV_L0_SetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y, Index1);
      }
      y++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_XOR:;
    IndexMask = GUI_Context.apDevice[GUI_Context.SelLayer]->pColorConvAPI->pfGetIndexMask();
    do {
      if (*p & (0x80 >> Diff)) {
        Pixel = LCDDEV_L0_GetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y);
        LCDDEV_L0_SetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y, Pixel ^ IndexMask);
      }
      y++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void  _DrawBitLine2BPP(int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;
  tLCD_SetPixelAA * pfSetPixelAA;
  const int aConvTable[4] = {0, 5, 10, 15};

  Pixels = *p;
  CurrentPixel = Diff;
  y += Diff;
  pfSetPixelAA = _GetpfSetPixel();
  do {
    Shift = (3 - CurrentPixel) << 1;
    Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
    PixelIndex = *(aConvTable + Index);
    pfSetPixelAA(x, y, PixelIndex);
    y++;
    if (++CurrentPixel == 4) {
      CurrentPixel = 0;
      Pixels = *(++p);
    }
  } while (--xsize);
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void  _DrawBitLine4BPP(int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize) {
  LCD_PIXELINDEX Pixels;
  int CurrentPixel, Shift, Index;
  tLCD_SetPixelAA * pfSetPixelAA;

  Pixels = *p;
  CurrentPixel = Diff;
  y += Diff;
  pfSetPixelAA = _GetpfSetPixel();
  do {
    Shift = (1 - CurrentPixel) << 2;
    Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
    pfSetPixelAA(x, y, Index);
    y++;
    if (++CurrentPixel == 2) {
      CurrentPixel = 0;
      Pixels = *(++p);
    }
  } while (--xsize);
}

/*********************************************************************
*
*       _DrawBitmap
*
* Purpose:
*   Draws a bitmap (1bpp) clockwise.
*/
static void _DrawBitmap(int x0, int y0,
                        int xsize, int ysize,
                        int BitsPerPixel, 
                        int BytesPerLine,
                        const U8 GUI_UNI_PTR * pData, int Diff,
                        const LCD_PIXELINDEX * pTrans)
{
  int i;
  /* Use _DrawBitLineXBPP */
  for (i = 0; i < ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(x0 - i, y0, pData, Diff, xsize, pTrans);
      break;
    case 2:
      _DrawBitLine2BPP(x0 - i, y0, pData, Diff, xsize);
      break;
    case 4:
      _DrawBitLine4BPP(x0 - i, y0, pData, Diff, xsize);
      break;
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       _Rect2TextRect
*
* Purpose:
*   This function transforms a given rectangle (window coordinates)
*   to the rectangle used to clip the text.
*/
static void _Rect2TextRect(GUI_RECT * pRect) {
  int x1, y1;
  x1 = pRect->x1;
  y1 = pRect->y1;
  pRect->x1 = pRect->x0 + (y1 - pRect->y0);
  pRect->y1 = pRect->y0 + (x1 - pRect->x0);
}

/*********************************************************************
*
*       _TransformPointCW
*
* Purpose:
*   This function transforms an unrotated point (window
*   coordinates) into a rotated point in desktop coordinates
*   and handles the rotation of the current text rectangle.
*/
static void _TransformPointCW(int * pXPos, int * pYPos) {
  GUI_RECT ClientRect = {0};
  int xPos, yPos, xNumPixel, yNumPixel;
  /* Get the client rectangle */
  #if GUI_WINSUPPORT
    WM_GetWindowRect(&ClientRect);
  #else
    GUI_GetClientRect(&ClientRect);
  #endif
  xNumPixel = LCD_GetXSize() - 1;
  yNumPixel = LCD_GetYSize() - 1;
  if (ClientRect.x1 > xNumPixel) {
    ClientRect.x1 = xNumPixel;
  }
  if (ClientRect.y1 > yNumPixel) {
    ClientRect.y1 = yNumPixel;
  }
  /* Save old positions */
  xPos = *pXPos;
  yPos = *pYPos;
  /* Rotate and add window origin */
  *pXPos = ClientRect.x0 - yPos;
  *pYPos = ClientRect.y0 + xPos;
  /* Handle rotation of text rectangle */
  *pXPos = *pXPos + GUI_RectDispString.x1 + GUI_RectDispString.y0;
  *pYPos = *pYPos + GUI_RectDispString.y0 - GUI_RectDispString.x0;
}

/*********************************************************************
*
*       _DrawBitmapCW
*/
static void _DrawBitmapCW(int x0, int y0, int xsize, int ysize, int xMul, int yMul,
                           int BitsPerPixel, int BytesPerLine,
                           const U8 GUI_UNI_PTR * pPixel, const LCD_PIXELINDEX * pTrans)
{
  U8  Data = 0;
  int x1, y1, Diff;
  /* Handle the optional Y-magnification */
  y1 = y0 + ysize - 1;
  x1 = x0 + xsize - 1;
  /* Rotate positions */
  _TransformPointCW(&x0, &y0);
  _TransformPointCW(&x1, &y1);
  /*  Handle BITMAP without magnification */
  if ((xMul == 1) && (yMul == 1)) {
    /* Clip right side */
    if (x0 > GUI_Context.ClipRect.x1) {
      Diff = x0 - GUI_Context.ClipRect.x1;
      ysize -= Diff;
      x0 -= Diff;
      pPixel += Diff * BytesPerLine;
    }
    /* Clip left side */
    if (x1 < GUI_Context.ClipRect.x0) {
      Diff = GUI_Context.ClipRect.x0 - x1;
      x1 += Diff;
      ysize -= Diff;
    }
    Diff = 0;
    /* Clip top */
    if (y1 > GUI_Context.ClipRect.y1) {
      Diff = y1 - GUI_Context.ClipRect.y1; 
      xsize -= Diff;
    }
    /* Clip bottom */
    if (y0 < GUI_Context.ClipRect.y0) {
      Diff = GUI_Context.ClipRect.y0 - y0;
      xsize -= Diff;
      switch (BitsPerPixel) {
      case 1:
        pPixel += (Diff >> 3); y0 -= (Diff >> 3) << 3; Diff &= 7;
        break;
      }
    }
    if (xsize <=0) {
      return;
    }
    if (ysize <= 0) {
      return;
    }
    _DrawBitmap(x0, y0, xsize, ysize, BitsPerPixel, BytesPerLine, pPixel, Diff, pTrans);
  } else {
    /* Handle BITMAP with magnification */
    int x, y, xi, yi;
    int Shift = 8 - BitsPerPixel;
    for (x = x0, xi = 0; xi < ysize; xi++, x -= yMul, pPixel += BytesPerLine) {
      int xMax = x - yMul + 1;
      if ((xMax >= GUI_Context.ClipRect.x0) && (x <= GUI_Context.ClipRect.x1)) {
        int BitsLeft = 0;
        const U8 GUI_UNI_PTR * pDataLine = pPixel;
        for (y = y0, yi = 0; yi < xsize; yi++, y += xMul) {
          U8  Index;
          if (!BitsLeft) {
            Data = *pDataLine++;
            BitsLeft =8;
          }
          Index = Data >> Shift;
          Data    <<= BitsPerPixel;
          BitsLeft -= BitsPerPixel;
          if (Index || ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0)) {
            LCD_PIXELINDEX OldColor = LCD__GetColorIndex();
            if (pTrans) {
              LCD__SetColorIndex(*(pTrans + Index));
            } else {
              LCD__SetColorIndex(Index);
            }
            LCD_FillRect(xMax, y, x, y + xMul - 1);
            LCD__SetColorIndex(OldColor);
          }
        }
      }
    }
  }
}

/*********************************************************************
*
*       Global data
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_APIListCW
*
* Purpose:
*   Function pointer table for rotating text CW
*/
tLCD_APIList LCD_APIListCW = {
  (tLCD_DrawBitmap*)&_DrawBitmapCW,
  &_Rect2TextRect
};

#else
void LCD_RotateCW_C(void);
void LCD_RotateCW_C(void){}
#endif

