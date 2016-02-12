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
File        : GUIDEV_Rotate.c
Purpose     : Rotation of memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <math.h>

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define NUM_BITS 12

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U32 r, g, b, a, Sum, SumAlpha;
} PIXEL;

typedef struct {
  int xMin, xMax, yMin, yMax;
} RECT_MIN_MAX;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetMinMax
*/
static void _GetMinMax(I32 x0SrcHQ, I32 y0SrcHQ, I32 sinHQ, I32 cosHQ, int xSize, int ySize, int Mag, RECT_MIN_MAX * pRectMinMax) {
  int xMinHQ, xMaxHQ, yMinHQ, yMaxHQ;
  I32 x0HQ, x1HQ, x2HQ, x3HQ, y0HQ, y1HQ, y2HQ, y3HQ;
  I32 cosMagHQ;
  I32 sinMagHQ;

  cosMagHQ = cosHQ * Mag / 1000;
  sinMagHQ = sinHQ * Mag / 1000;
  x0HQ = x0SrcHQ;
  y0HQ = y0SrcHQ;
  x1HQ = x0HQ + (cosMagHQ * xSize);
  y1HQ = y0HQ - (sinMagHQ * xSize);
  x2HQ = x1HQ + (sinMagHQ * ySize);
  y2HQ = y1HQ + (cosMagHQ * ySize);
  x3HQ = x2HQ - (cosMagHQ * xSize);
  y3HQ = y2HQ + (sinMagHQ * xSize);
  xMinHQ = x0HQ;
  if (xMinHQ > x1HQ) {
    xMinHQ = x1HQ;
  }
  if (xMinHQ > x2HQ) {
    xMinHQ = x2HQ;
  }
  if (xMinHQ > x3HQ) {
    xMinHQ = x3HQ;
  }
  xMaxHQ = x0HQ;
  if (xMaxHQ < x1HQ) {
    xMaxHQ = x1HQ;
  }
  if (xMaxHQ < x2HQ) {
    xMaxHQ = x2HQ;
  }
  if (xMaxHQ < x3HQ) {
    xMaxHQ = x3HQ;
  }
  yMinHQ = y0HQ;
  if (yMinHQ > y1HQ) {
    yMinHQ = y1HQ;
  }
  if (yMinHQ > y2HQ) {
    yMinHQ = y2HQ;
  }
  if (yMinHQ > y3HQ) {
    yMinHQ = y3HQ;
  }
  yMaxHQ = y0HQ;
  if (yMaxHQ < y1HQ) {
    yMaxHQ = y1HQ;
  }
  if (yMaxHQ < y2HQ) {
    yMaxHQ = y2HQ;
  }
  if (yMaxHQ < y3HQ) {
    yMaxHQ = y3HQ;
  }
  pRectMinMax->xMin = (xMinHQ + (1L << 15)) >> 16;
  pRectMinMax->yMin = (yMinHQ + (1L << 15)) >> 16;
  pRectMinMax->xMax = (xMaxHQ - (1L << 15)) >> 16;
  pRectMinMax->yMax = (yMaxHQ - (1L << 15)) >> 16;
}

/*********************************************************************
*
*       _GetColor
*/
static U32 _GetColor(GUI_MEMDEV * pMem, U32 * pData, I32 xOffHQ, I32 yOffHQ) {
  U32 Color;
  int xOff, yOff;
  Color = 0xFF000000;
  /*
  * Calculate real values
  */
  xOff = (xOffHQ + (1 << 15)) >> 16;
  yOff = (yOffHQ + (1 << 15)) >> 16;
  /*
  * Return transparency if position is outside
  */
  if ((xOff < 0) || (yOff < 0)) {
    return Color;
  }
  if ((xOff >= pMem->XSize) || (yOff >= pMem->YSize)) {
    return Color;
  }
  /*
  * Return color value if position is inside
  */
  Color = *(pData + yOff * pMem->XSize + xOff);
  return Color;
}

/*********************************************************************
*
*       _Rotate32
*/
static void _Rotate32(GUI_MEMDEV_Handle hMemSrc, GUI_MEMDEV_Handle hMemDst, int dx, int dy, int a, int Mag, U32 AndMask) {
  I32 cosMagHQ, sinMagHQ;
  I32 cosHQ, sinHQ;
  I32 xPosHQ, yPosHQ;
  int xSizeSrc, ySizeSrc;
  I32 x0SrcHQ, y0SrcHQ;
  int xDst, yDst;
  GUI_MEMDEV * pMemSrc;
  GUI_MEMDEV * pMemDst;
  U32 * pDataSrc;
  U32 * pDataDst;
  RECT_MIN_MAX RectMinMax;
  I32 x0DstHQ, y0DstHQ;
  int xStart, yStart, xEnd, yEnd;

  if (Mag == 0) {
    return;
  }
  /*
  * Get data pointers
  */
  pMemSrc   = (GUI_MEMDEV *)GUI_LOCK_H(hMemSrc);
  pMemDst   = (GUI_MEMDEV *)GUI_LOCK_H(hMemDst);
  pDataSrc  = (U32 *)GUI_MEMDEV_GetDataPtr(hMemSrc);
  pDataDst  = (U32 *)GUI_MEMDEV_GetDataPtr(hMemDst);
  /*
  * Calculate start position in source device
  */
  xSizeSrc    = pMemSrc->XSize - 1;
  ySizeSrc    = pMemSrc->YSize - 1;
  cosHQ       = GUI__CosHQ(a);
  sinHQ       = GUI__SinHQ(a);
  cosMagHQ    = (cosHQ * 1000) / Mag;
  sinMagHQ    = (sinHQ * 1000) / Mag;
  x0SrcHQ     = (xSizeSrc << 15) 
              + ((sinMagHQ * ySizeSrc) >> 1) 
              - ((cosMagHQ * xSizeSrc) >> 1) 
              + sinMagHQ * dy 
              - cosMagHQ * dx;
  y0SrcHQ     = (ySizeSrc << 15) 
              - ((cosMagHQ * ySizeSrc) >> 1) 
              - ((sinMagHQ * xSizeSrc) >> 1) 
              - cosMagHQ * dy 
              - sinMagHQ * dx;
  x0DstHQ     = (xSizeSrc << 15)
              - ((((sinHQ * Mag) / 1000) * pMemSrc->YSize) >> 1) 
              - ((((cosHQ * Mag) / 1000) * pMemSrc->XSize) >> 1)
              + ((U32)dx << 16);
  y0DstHQ     = (ySizeSrc << 15)
              - ((((cosHQ * Mag) / 1000) * pMemSrc->YSize) >> 1) 
              + ((((sinHQ * Mag) / 1000) * pMemSrc->XSize) >> 1)
              + ((U32)dy << 16);
  _GetMinMax(x0DstHQ, y0DstHQ, sinHQ, cosHQ, pMemSrc->XSize, pMemSrc->YSize, Mag, &RectMinMax);
  /*
  * Clip y
  */
  yStart = RectMinMax.yMin;
  yEnd   = RectMinMax.yMax;
  if (yStart < 0) {
    yStart = 0;
  }
  if (yEnd >= pMemDst->YSize) {
    yEnd = pMemDst->YSize - 1;
  }
  x0SrcHQ -= sinMagHQ * yStart;
  y0SrcHQ += cosMagHQ * yStart;
  /*
  * Clip x
  */
  xStart = RectMinMax.xMin;
  xEnd   = RectMinMax.xMax;
  if (xStart < 0) {
    xStart = 0;
  }
  if (xEnd >= pMemDst->XSize) {
    xEnd = pMemDst->XSize - 1;
  }
  x0SrcHQ  += cosMagHQ * xStart;
  y0SrcHQ  += sinMagHQ * xStart;
  /*
  * Iterate over the lines of the destination device
  */
  pDataDst += pMemDst->XSize * yStart;
  for (yDst = yStart; yDst <= yEnd; yDst++) {
    xPosHQ = x0SrcHQ;
    yPosHQ = y0SrcHQ;
    /*
    * Iterate over all pixels of the line
    */
    pDataDst += xStart;
    for (xDst = xStart; xDst <= xEnd; xDst++) {
      U32 Color, BkColor;
      U8 Alpha;
      Color = _GetColor(pMemSrc, pDataSrc, xPosHQ, yPosHQ);
      Alpha = Color >> 24;
      if (Alpha == 0) {
        /*
        * No alpha blending
        */
        *pDataDst = Color;
      } else if (Alpha < 255) {
        if (Alpha > 0) {
          /*
          * Manage alpha blending
          */
          BkColor   = *pDataDst;
          Color     = GUI__MixColors(Color, BkColor, 255 - Alpha);
          Color    &= AndMask;
          *pDataDst = Color;
        }
      }
      #if 0 /* Please leave code for test purpose */
      if (xDst == RectMinMax.xMin) {
        *pDataDst = GUI_RED;
      }
      if (xDst == RectMinMax.xMax) {
        *pDataDst = GUI_RED;
      }
      if (yDst == RectMinMax.yMin) {
        *pDataDst = GUI_RED;
      }
      if (yDst == RectMinMax.yMax) {
        *pDataDst = GUI_RED;
      }
      #endif
      xPosHQ += cosMagHQ;
      yPosHQ += sinMagHQ;
      pDataDst++;
    }
    pDataDst += (pMemDst->XSize - xEnd) - 1;
    x0SrcHQ -= sinMagHQ;
    y0SrcHQ += cosMagHQ;
  }
  /*
  * Unlock pointers
  */
  GUI_UNLOCK_H(pMemSrc);
  GUI_UNLOCK_H(pMemDst);
}

/*********************************************************************
*
*       _GetColorHQ
*/
static U32 _GetColorHQ(GUI_MEMDEV * pMem, U32 * pData, I32 xOffHQ, I32 yOffHQ, I32 Size) {
  U32 Color;
  int xOff, yOff;
  I32 x0HQ, y0HQ, x1HQ, y1HQ, xPosHQ, yPosHQ, xNextHQ, yNextHQ;
  U32 xPart, yPart;
  PIXEL Pixel = {0};
  U32 r, g, b, a, f;

  x0HQ = xOffHQ - (Size >> 1);
  y0HQ = yOffHQ - (Size >> 1);
  x1HQ = x0HQ + Size - 1;
  y1HQ = y0HQ + Size - 1;
  /*
  * Iterate over the lines
  */
  yPosHQ = y0HQ;
  do {
    /*
    * Calculate next y position
    */
    yOff    = (yPosHQ >> 16);
    yNextHQ = ((I32)yOff + 1) << 16;
    if (yNextHQ > y1HQ) {
      yNextHQ = y1HQ + 1;
    }
    /*
    * Calculate y part
    */
    yPart = (yNextHQ - yPosHQ) >> (16 - NUM_BITS);
    /*
    * Iterate over one line
    */
    xPosHQ = x0HQ;
    do {
      /*
      * Calculate next x position
      */
      xOff    = (xPosHQ >> 16);
      xNextHQ = ((I32)xOff + 1) << 16;
      if (xNextHQ > x1HQ) {
        xNextHQ = x1HQ + 1;
      }
      /*
      * Calculate x part
      */
      xPart = (xNextHQ - xPosHQ) >> (16 - NUM_BITS);
      if (yPart < (1 << NUM_BITS)) {
        xPart = (xPart * yPart) >> NUM_BITS;
      }
      /*
      * Add current color
      */
      if ((xPosHQ >= 0) && (xOff < pMem->XSize) && (yPosHQ >= 0) && (yOff < pMem->YSize)) {
        /*
        * We are inside of the source memory device, so use its data
        */
        Color = *(pData + yOff * pMem->XSize + xOff);
      } else {
        /*
        * We are outside, so use transparency
        */
        Color = GUI_TRANSPARENT;
      }
      a = (Color & GUI_TRANSPARENT) >> 24;
      if (a < 255) {
        r = (Color & 0x000000FF) >>  0;
        g = (Color & 0x0000FF00) >>  8;
        b = (Color & 0x00FF0000) >> 16;
        if (a == 0) {
          /*
          * No alpha value, so use full intensity
          */
          r = (r * xPart);
          g = (g * xPart);
          b = (b * xPart);
          Pixel.r += r;
          Pixel.g += g;
          Pixel.b += b;
          Pixel.Sum      += xPart;
          Pixel.SumAlpha += xPart;
        } else {
          /*
          * Reduce part by alpha value
          */
          f = GUI__DivideRound32(xPart * a, 255);
          r = (r * f);
          g = (g * f);
          b = (b * f);
          a = (a * f);
          Pixel.r += r;
          Pixel.g += g;
          Pixel.b += b;
          Pixel.a += a;
          Pixel.Sum      += f;
          Pixel.SumAlpha += f;
        }
      } else {
        /*
        * Completely transparent
        */
        a = (a * xPart);
        Pixel.a += a;
        Pixel.SumAlpha += xPart;
      }
      /*
      * Next x positon
      */
      xPosHQ = xNextHQ;
    } while (xPosHQ < x1HQ);
    /*
    * Next y positon
    */
    yPosHQ = yNextHQ;
  } while (yPosHQ < y1HQ);
  /*
  * Calculate resulting color
  */
  if (Pixel.Sum) {
    Pixel.r = GUI__DivideRound32(Pixel.r, Pixel.Sum);
    Pixel.g = GUI__DivideRound32(Pixel.g, Pixel.Sum);
    Pixel.b = GUI__DivideRound32(Pixel.b, Pixel.Sum);
    if (Pixel.a) {
      Pixel.a = GUI__DivideRound32(Pixel.a, Pixel.SumAlpha);
    }
  } else {
    Pixel.a = 0xff;
  }
  if (Pixel.r > 0xff) {
    Pixel.r = 0xff;
  }
  if (Pixel.g > 0xff) {
    Pixel.g = 0xff;
  }
  if (Pixel.b > 0xff) {
    Pixel.b = 0xff;
  }
  Color = Pixel.r | (Pixel.g << 8) | (Pixel.b << 16) | (Pixel.a << 24);
  return Color;
}

/*********************************************************************
*
*       _Rotate32HQ
*/
static void _Rotate32HQ(GUI_MEMDEV_Handle hMemSrc, GUI_MEMDEV_Handle hMemDst, int dx, int dy, int a, int Mag) {
  I32 cosMagHQ, sinMagHQ;
  I32 cosHQ, sinHQ;
  I32 xPosHQ, yPosHQ;
  int xSizeSrc, ySizeSrc;
  I32 x0SrcHQ, y0SrcHQ;
  int xDst, yDst;
  I32 PixelSizeHQ;
  GUI_MEMDEV * pMemSrc;
  GUI_MEMDEV * pMemDst;
  U32 * pDataSrc;
  U32 * pDataDst;
  RECT_MIN_MAX RectMinMax;
  I32 x0DstHQ, y0DstHQ;
  int xStart, yStart, xEnd, yEnd;

  if (Mag == 0) {
    return;
  }
  /*
  * Get data pointers
  */
  pMemSrc   = (GUI_MEMDEV *)GUI_LOCK_H(hMemSrc);
  pMemDst   = (GUI_MEMDEV *)GUI_LOCK_H(hMemDst);
  pDataSrc  = (U32 *)GUI_MEMDEV_GetDataPtr(hMemSrc);
  pDataDst  = (U32 *)GUI_MEMDEV_GetDataPtr(hMemDst);
  /*
  * Calculate start position in source device
  */
  PixelSizeHQ = (1000L << 16) / Mag;
  xSizeSrc    = pMemSrc->XSize;
  ySizeSrc    = pMemSrc->YSize;
  cosHQ       = GUI__CosHQ(a);
  sinHQ       = GUI__SinHQ(a);
  cosMagHQ    = (cosHQ * 1000) / Mag;
  sinMagHQ    = (sinHQ * 1000) / Mag;
  x0SrcHQ     = (xSizeSrc << 15) 
              + ((sinMagHQ * ySizeSrc) >> 1) 
              - ((cosMagHQ * xSizeSrc) >> 1) 
              - (sinMagHQ >> 1)
              + (cosMagHQ >> 1)
              + sinMagHQ * dy 
              - cosMagHQ * dx;
  y0SrcHQ     = (ySizeSrc << 15) 
              - ((cosMagHQ * ySizeSrc) >> 1) 
              - ((sinMagHQ * xSizeSrc) >> 1) 
              + (cosMagHQ >> 1)
              + (sinMagHQ >> 1)
              - cosMagHQ * dy 
              - sinMagHQ * dx;

  x0DstHQ     = (xSizeSrc << 15)
              - ((((sinHQ * Mag) / 1000) * ySizeSrc) >> 1) 
              - ((((cosHQ * Mag) / 1000) * xSizeSrc) >> 1)
              + ((U32)dx << 16);
  y0DstHQ     = (ySizeSrc << 15)
              - ((((cosHQ * Mag) / 1000) * ySizeSrc) >> 1) 
              + ((((sinHQ * Mag) / 1000) * xSizeSrc) >> 1)
              + ((U32)dy << 16);
  _GetMinMax(x0DstHQ, y0DstHQ, sinHQ, cosHQ, xSizeSrc, ySizeSrc, Mag, &RectMinMax);
  /*
  * Clip y
  */
  yStart = RectMinMax.yMin;
  yEnd   = RectMinMax.yMax;
  if (yStart < 0) {
    yStart = 0;
  }
  if (yEnd >= pMemDst->YSize) {
    yEnd = pMemDst->YSize - 1;
  }
  x0SrcHQ -= sinMagHQ * yStart;
  y0SrcHQ += cosMagHQ * yStart;
  /*
  * Clip x
  */
  xStart = RectMinMax.xMin;
  xEnd   = RectMinMax.xMax;
  if (xStart < 0) {
    xStart = 0;
  }
  if (xEnd >= pMemDst->XSize) {
    xEnd = pMemDst->XSize - 1;
  }
  x0SrcHQ  += cosMagHQ * xStart;
  y0SrcHQ  += sinMagHQ * xStart;
  /*
  * Iterate over the lines of the destination device
  */
  pDataDst += pMemDst->XSize * yStart;
  for (yDst = yStart; yDst <= yEnd; yDst++) {
    xPosHQ = x0SrcHQ;
    yPosHQ = y0SrcHQ;
    /*
    * Iterate over all pixels of the line
    */
    pDataDst += xStart;
    for (xDst = xStart; xDst <= xEnd; xDst++) {
      U32 Color, BkColor;
      U8 Alpha;
      Color = _GetColorHQ(pMemSrc, pDataSrc, xPosHQ, yPosHQ, PixelSizeHQ);
      Alpha = Color >> 24;
      if (Alpha == 0) {
        /*
        * No alpha blending
        */
        *pDataDst = Color;
      } else if (Alpha < 255) {
        if (Alpha > 0) {
          /*
          * Manage alpha blending
          */
          BkColor   = *pDataDst;
          Color     = GUI__MixColors(Color, BkColor, 255 - Alpha);
          *pDataDst = Color;
        }
      }
      #if 0 /* Please leave code for test purpose */
      if (xDst == RectMinMax.xMin) {
        *pDataDst = GUI_RED;
      }
      if (xDst == RectMinMax.xMax) {
        *pDataDst = GUI_RED;
      }
      if (yDst == RectMinMax.yMin) {
        *pDataDst = GUI_RED;
      }
      if (yDst == RectMinMax.yMax) {
        *pDataDst = GUI_RED;
      }
      #endif
      xPosHQ += cosMagHQ;
      yPosHQ += sinMagHQ;
      pDataDst++;
    }
    pDataDst += (pMemDst->XSize - xEnd) - 1;
    x0SrcHQ -= sinMagHQ;
    y0SrcHQ += cosMagHQ;
  }
  /*
  * Unlock pointers
  */
  GUI_UNLOCK_H(pMemSrc);
  GUI_UNLOCK_H(pMemDst);
}

/*********************************************************************
*
*       _Check
*/
static int _Check(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst) {
  int BppSrc, BppDst;

  BppSrc = GUI_MEMDEV_GetBitsPerPixel(hSrc);
  BppDst = GUI_MEMDEV_GetBitsPerPixel(hDst);
  if ((BppSrc == 32) && (BppDst == 32)) {
    return 0;
  }
  return 1;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV__Rotate
*/
void GUI_MEMDEV__Rotate(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst, int dx, int dy, int a, int Mag, U32 AndMask) {
  GUI_LOCK();
  if (_Check(hSrc, hDst) == 0) {
    /*
    * Works only if both devices are 32bpp devices
    */
    _Rotate32(hSrc, hDst, dx, dy, a, Mag, AndMask);
  }
  GUI_UNLOCK();
  return;
}

/*********************************************************************
*
*       GUI_MEMDEV_Rotate
*/
void GUI_MEMDEV_Rotate(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst, int dx, int dy, int a, int Mag) {
  GUI_MEMDEV__Rotate(hSrc, hDst, dx, dy, a, Mag, 0xFFFFFFFF);
}

/*********************************************************************
*
*       GUI_MEMDEV_RotateHQ
*/
void GUI_MEMDEV_RotateHQ(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst, int dx, int dy, int a, int Mag) {
  GUI_LOCK();
  if (_Check(hSrc, hDst) == 0) {
    /*
    * Works only if both devices are 32bpp devices
    */
    _Rotate32HQ(hSrc, hDst, dx, dy, a, Mag);
  }
  GUI_UNLOCK();
  return;
}

#else

void GUIDEV_Rotate_C(void);
void GUIDEV_Rotate_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
