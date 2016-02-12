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
File        : GUIDRV_NoOpt_1_8.c
Purpose     : Non optimized drawing functions
---------------------------END-OF-HEADER------------------------------
*/

#include "GUIDRV_NoOpt_1_8.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       Draw Bitmap 1 BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, unsigned x, unsigned y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX IndexMask, Index0, Index1, Pixel;
  unsigned (* pfGetPixelIndex)(GUI_DEVICE * pDevice, int x, int y);
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    do {
      pfSetPixelIndex(pDevice, x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*p & (0x80 >> Diff)) {
        pfSetPixelIndex(pDevice, x, y, Index1);
      }
      x++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    pfGetPixelIndex = pDevice->pDeviceAPI->pfGetPixelIndex;
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      if (*p & (0x80 >> Diff)) {
        Pixel = pfGetPixelIndex(pDevice, x, y);
        pfSetPixelIndex(pDevice, x, y, Pixel ^ IndexMask);
      }
      x++;
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
*       Draw Bitmap 2 BPP
*/
static void _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        pfSetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        pfSetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          pfSetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          pfSetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 4 BPP
*/
static void _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        pfSetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        pfSetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          pfSetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          pfSetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 8 BPP
*/
static void _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        pfSetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        pfSetPixelIndex(pDevice, x, y, *p);
      }
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          pfSetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          pfSetPixelIndex(pDevice, x, y, Pixel);
        }
      }
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 16 BPP
*/
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const GUI_UNI_PTR * p, int xSize) {
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  for (; xSize > 0; xSize--, x++, p++) {
    pfSetPixelIndex(pDevice, x, y, *p);
  }
}

/*********************************************************************
*
*       Draw Bitmap 32 BPP
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xSize) {
  void     (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  for (; xSize > 0; xSize--, x++, p++) {
    pfSetPixelIndex(pDevice, x, y, *p);
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDRV__NoOpt_XorPixel
*/
void GUIDRV__NoOpt_XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  LCD_PIXELINDEX PixelIndex;
  LCD_PIXELINDEX IndexMask;
  
  PixelIndex = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, PixelIndex ^ IndexMask);
}

/*********************************************************************
*
*       GUIDRV__NoOpt_DrawHLine
*/
void GUIDRV__NoOpt_DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  LCD_PIXELINDEX ColorIndex;
  void (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);
  void (* pfXorPixel     )(GUI_DEVICE * pDevice, int x, int y);

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    pfXorPixel = pDevice->pDeviceAPI->pfXorPixel;
    for (; x0 <= x1; x0++) {
      pfXorPixel(pDevice, x0, y);
    }
  } else {
    pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
    ColorIndex = LCD__GetColorIndex();
    for (; x0 <= x1; x0++) {
      pfSetPixelIndex(pDevice, x0, y, ColorIndex);
    }
  }
}

/*********************************************************************
*
*       GUIDRV__NoOpt_DrawVLine, not optimized
*/
void GUIDRV__NoOpt_DrawVLine  (GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  LCD_PIXELINDEX ColorIndex;
  void (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);
  void (* pfXorPixel     )(GUI_DEVICE * pDevice, int x, int y);

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    pfXorPixel = pDevice->pDeviceAPI->pfXorPixel;
    for (; y0 <= y1; y0++) {
      pfXorPixel(pDevice, x, y0);
    }
  } else {
    pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
    ColorIndex = LCD__GetColorIndex();
    for (; y0 <= y1; y0++) {
      pfSetPixelIndex(pDevice, x, y0, ColorIndex);
    }
  }
}

/*********************************************************************
*
*       GUIDRV__NoOpt_FillRect
*/
void GUIDRV__NoOpt_FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  for (; y0 <= y1; y0++) {
    GUIDRV__NoOpt_DrawHLine(pDevice, x0, y0, x1);
  }
}

/*********************************************************************
*
*       GUIDRV__NoOpt_DrawBitmap
*/
void GUIDRV__NoOpt_DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans) {
  int i;

  switch (BitsPerPixel) {
  case 1:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 2:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 4:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 8:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 16:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine16BPP(pDevice, x0, i + y0, (const U16 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  case 32:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/*************************** End of file ****************************/
