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
File        : GUI_SetAlpha.c
Purpose     : Implementation of GUI_SetAlpha
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include <string.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8 _aAlpha[GUI_NUM_LAYERS];

//
// Buffers for index values and RGB colors for one display line of data
//
static LCD_PIXELINDEX * _pIndex;
static LCD_COLOR      * _pColor;

static GUI_DEVICE * _apDevice[GUI_NUM_LAYERS];

static unsigned _SuppressMixing;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _MixColors
*/
static LCD_COLOR _MixColors(LCD_COLOR Color, LCD_COLOR BkColor) {
  #if 1
    U8 Intens;
    LCD_COLOR r;

    if (_SuppressMixing) {
      Intens = _aAlpha[GUI_Context.apDevice[GUI_Context.SelLayer]->LayerIndex];
      r = (Color & 0xFFFFFF) | ((U32)Intens << 24);
    } else {
      Intens = 255 - _aAlpha[GUI_Context.apDevice[GUI_Context.SelLayer]->LayerIndex];
      r = GUI__MixColors(Color, BkColor, Intens) & 0xFFFFFF;
    }
    return r;
  #else
    U32 r, g, b, a;
    U8 Intens;

    Intens = 255 - _aAlpha[GUI_Context.apDevice[GUI_Context.SelLayer]->LayerIndex];
    //
    // Calculate color separations for FgColor first
    //
    r = (Color & 0x000000ff) * Intens;
    g = (Color & 0x0000ff00) * Intens;
    b = (Color & 0x00ff0000) * Intens;
    a = ((Color & 0xff000000) >> 24) * Intens;
    //
    // Add color separations for BkColor
    //
    Intens = 255 - Intens;
    r += (BkColor & 0x000000ff) * Intens;
    g += (BkColor & 0x0000ff00) * Intens;
    b += (BkColor & 0x00ff0000) * Intens;
    a += ((BkColor & 0xff000000) >> 24) * Intens;
    r = (r >>  8) & 0x000000ff;
    g = (g >>  8) & 0x0000ff00;
    b = (b >>  8) & 0x00ff0000;
    a = (a << 16) & 0xff000000;
    Color = r + g + b + a;
    return Color;
  #endif
}

/*********************************************************************
*
*       _ReadRectFromMemdev
*
* Purpose:
*   Reads the data from a memory device
*/
#if (GUI_SUPPORT_MEMDEV)
static void _ReadRectFromMemdev(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer) {
  int BytesPerPixel, BitsPerPixel;
  U8 * p;
  int x0m, y0m, xSize, NumPixels;

  GUI_USE_PARA(y1);
  GUI_USE_PARA(pDevice);
  x0m = GUI_MEMDEV_GetXPos(GUI_Context.hDevData);
  y0m = GUI_MEMDEV_GetYPos(GUI_Context.hDevData);
  xSize = GUI_MEMDEV_GetXSize(GUI_Context.hDevData);
  if (y0 < y0m) {
    return;
  }
  if (x0 < x0m) {
    return;
  }
  BitsPerPixel = GUI_MEMDEV_GetBitsPerPixel(GUI_Context.hDevData);
  if (BitsPerPixel <= 8) {
    BytesPerPixel = 1;
  } else if (BitsPerPixel <= 16) {
    BytesPerPixel = 2;
  } else {
    BytesPerPixel = 4;
  }
  p = (U8 *)GUI_MEMDEV_GetDataPtr(GUI_Context.hDevData);
  p += (((U32)(y0 - y0m) * xSize) + (x0 - x0m)) * BytesPerPixel;
  NumPixels = (x1 - x0 + 1);
  GUI_MEMCPY(pBuffer, p, NumPixels * BytesPerPixel);
  GUI__ExpandPixelIndices(pBuffer, NumPixels, BitsPerPixel);
}
#endif

/*********************************************************************
*
*       _GetPointers
*/
static void _GetPointers(tLCDDEV_Index2Color ** ppfIndex2Color, tLCDDEV_Color2Index ** ppfColor2Index) {
  *ppfIndex2Color = GUI_GetpfIndex2ColorEx(GUI_Context.SelLayer);
  *ppfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
}

/*********************************************************************
*
*       _GetBitsPerPixel
*/
static int _GetBitsPerPixel(int LayerIndex) {
  int BitsPerPixel;

  GUI_USE_PARA(LayerIndex);
  BitsPerPixel = LCD_GetBitsPerPixelEx(GUI_Context.SelLayer);
  if (BitsPerPixel == 24) {
    BitsPerPixel = 32;
  }
  return BitsPerPixel;
}

/*********************************************************************
*
*       _GetLine
*/
static void _GetLine(GUI_DEVICE * pDevice, int x, int y, int NumPixels, LCD_PIXELINDEX * pBuffer, LCD_COLOR * pColor, tLCDDEV_Index2Color * pfIndex2Color) {
  LCD_PIXELINDEX * p;
  int i, x1, Expand, BitsPerPixel;
  void (* pReadRect)(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer);

  //
  // Get function pointer from next device
  //
  pDevice = pDevice->pNext;
  #if (GUI_SUPPORT_MEMDEV)
  if (pDevice->pDeviceAPI->DeviceClassIndex == DEVICE_CLASS_MEMDEV) {
    pReadRect = _ReadRectFromMemdev;
    Expand    = 0;
  } else {
  #else
  {
  #endif
    pReadRect = (void (*)(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_READRECT);
    Expand    = 1;
  }
  //
  // Get pixel data
  //
  x1 = x + NumPixels - 1;
  if (pReadRect) {
    //
    // Use function pointer
    //
    pReadRect(pDevice, x, y, x1, y, pBuffer);
    if (Expand) {
      //
      // Convert line data into right format
      //
      BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
      GUI__ExpandPixelIndices(pBuffer, NumPixels, BitsPerPixel);
    }
  } else {
    //
    // Read pixel by pixel
    //
    p = pBuffer;
    for (i = x; i <= x1; i++) {
      *(p++) = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, i, y);
    }
  }
  //
  // Convert indices into color
  //
  for (i = 0; i < NumPixels; i++) {
    *(pColor + i) = pfIndex2Color(*(pBuffer + i));
  }
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _GetPixelIndex
*/
static unsigned _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y)  {
  LCD_PIXELINDEX PixelIndex;

  pDevice = pDevice->pNext;
  PixelIndex = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
  return PixelIndex;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int ColorIndex) {
  LCD_PIXELINDEX IndexOld, IndexNew;
  LCD_COLOR      ColorOld, ColorNew, Color;

  pDevice  = pDevice->pNext;
  IndexOld = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
  ColorOld = pDevice->pColorConvAPI->pfIndex2Color(IndexOld);
  Color    = pDevice->pColorConvAPI->pfIndex2Color(ColorIndex);
  ColorNew = _MixColors(Color, ColorOld);
  IndexNew = pDevice->pColorConvAPI->pfColor2Index(ColorNew);
  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, IndexNew);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  int IndexMask;
  LCD_PIXELINDEX PixelIndex;

  PixelIndex = _GetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pNext->pColorConvAPI->pfGetIndexMask();
  _SetPixelIndex(pDevice, x, y, PixelIndex ^ IndexMask);
}

/*********************************************************************
*
*       _DrawBitLine1BPP, optimized
*/
static void  _DrawBitLine1BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Index0;
  LCD_PIXELINDEX Index1;
  LCD_COLOR      Color0;
  LCD_COLOR      Color1;
  int i, NumPixels, IndexMask, BitsPerPixel;
  tLCDDEV_Index2Color * pfIndex2Color;
  tLCDDEV_Color2Index * pfColor2Index;
  LCD_COLOR Color, ColorNew;

  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    NumPixels = xSize;
    _GetPointers(&pfIndex2Color, &pfColor2Index);
    _GetLine(pDevice, x, y, xSize, _pIndex, _pColor, pfIndex2Color);
    Color0 = pfIndex2Color(Index0);
    Color1 = pfIndex2Color(Index1);
    i = 0;
    do {
      Color      = (*p & (0x80 >> Diff)) ? Color1 : Color0;
      ColorNew   = _MixColors(Color, *(_pColor + i));
      *(_pIndex + i) = pfColor2Index(ColorNew);
			if (++Diff == 8) {
        Diff = 0;
				p++;
			}
      i++;
		} while (--xSize);
    BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
    GUI__CompactPixelIndices(_pIndex, NumPixels, BitsPerPixel);
    pDevice->pNext->pDeviceAPI->pfDrawBitmap(pDevice->pNext, x, y, NumPixels, 1, BitsPerPixel, 0, (U8 *)_pIndex, 0, 0);
    break;
  case LCD_DRAWMODE_TRANS:
    NumPixels = xSize;
    _GetPointers(&pfIndex2Color, &pfColor2Index);
    _GetLine(pDevice, x, y, xSize, _pIndex, _pColor, pfIndex2Color);
    Color1 = pfIndex2Color(Index1);
    i = 0;
    do {
  		if (*p & (0x80 >> Diff)) {
        ColorNew   = _MixColors(Color1, *(_pColor + i));
        *(_pIndex + i) = pfColor2Index(ColorNew);
      }
      i++;
			if (++Diff == 8) {
        Diff = 0;
				p++;
			}
		} while (--xSize);
    BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
    GUI__CompactPixelIndices(_pIndex, NumPixels, BitsPerPixel);
    pDevice->pNext->pDeviceAPI->pfDrawBitmap(pDevice->pNext, x, y, NumPixels, 1, BitsPerPixel, 0, (U8 *)_pIndex, 0, 0);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
  		if (*p & (0x80 >> Diff)) {
        int Pixel = _GetPixelIndex(pDevice, x, y);
        _SetPixelIndex(pDevice, x, y, Pixel ^ IndexMask);
      }
      x++;
			if (++Diff == 8) {
        Diff = 0;
				p++;
			}
		} while (--xSize);
    return;
	}
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void  _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x++, y, PixelIndex);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          _SetPixelIndex(pDevice, x++, y, Index);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            _SetPixelIndex(pDevice, x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            _SetPixelIndex(pDevice, x, y, Index);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
  }
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void  _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x++, y, PixelIndex);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          _SetPixelIndex(pDevice, x++, y, Index);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            _SetPixelIndex(pDevice, x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            _SetPixelIndex(pDevice, x, y, Index);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
  }
}

/*********************************************************************
*
*       _DrawBitLine8BPP
*/
static void  _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        for (; xSize > 0; xSize--, x++, p++) {
          Pixel = *p;
          _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
        }
      } else {
        for (; xSize > 0; xSize--, x++, p++) {
          _SetPixelIndex(pDevice, x, y, *p);
        }
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        for (; xSize > 0; xSize--, x++, p++) {
          Pixel = *p;
          if (Pixel) {
            _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
          }
        }
      } else {
        for (; xSize > 0; xSize--, x++, p++) {
          Pixel = *p;
          if (Pixel) {
            _SetPixelIndex(pDevice, x, y, Pixel);
          }
        }
      }
      break;
  }
}

/*********************************************************************
*
*       _DrawBitLine16BPP, optimized
*/
static void  _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const GUI_UNI_PTR * p, int xSize) {
  LCD_COLOR Color, ColorBk, ColorNew;
  U16 Index;
  LCD_PIXELINDEX * pIndex;
  LCD_COLOR * pColor;
  int NumPixels, BitsPerPixel;
  tLCDDEV_Index2Color * pfIndex2Color;
  tLCDDEV_Color2Index * pfColor2Index;

  _GetPointers(&pfIndex2Color, &pfColor2Index);
  _GetLine(pDevice, x, y, xSize, _pIndex, _pColor, pfIndex2Color);
  pColor    = _pColor;
  pIndex    = _pIndex;
  NumPixels = xSize;
  do {
    ColorBk   = *pColor++;
    Index     = *p++;
    Color     = pfIndex2Color(Index);
    ColorNew  = _MixColors(Color, ColorBk);
    *pIndex++ = pfColor2Index(ColorNew);
  } while (--NumPixels);
  BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
  GUI__CompactPixelIndices(_pIndex, xSize, BitsPerPixel);
  pDevice->pNext->pDeviceAPI->pfDrawBitmap(pDevice->pNext, x, y, xSize, 1, BitsPerPixel, 0, (U8 *)_pIndex, 0, 0);
}

/*********************************************************************
*
*       _DrawBitLine32BPP, optimized
*/
static void  _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xSize) {
  LCD_COLOR Color, ColorBk, ColorNew;
  U32 Index;
  LCD_PIXELINDEX * pIndex;
  LCD_COLOR * pColor;
  int NumPixels, BitsPerPixel;
  tLCDDEV_Index2Color * pfIndex2Color;
  tLCDDEV_Color2Index * pfColor2Index;

  _GetPointers(&pfIndex2Color, &pfColor2Index);
  _GetLine(pDevice, x, y, xSize, _pIndex, _pColor, pfIndex2Color);
  pColor    = _pColor;
  pIndex    = _pIndex;
  NumPixels = xSize;
  do {
    ColorBk   = *pColor++;
    Index     = *p++;
    Color     = pfIndex2Color(Index);
    ColorNew  = _MixColors(Color, ColorBk);
    *pIndex++ = pfColor2Index(ColorNew);
  } while (--NumPixels);
  BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
  GUI__CompactPixelIndices(_pIndex, xSize, BitsPerPixel);
  pDevice->pNext->pDeviceAPI->pfDrawBitmap(pDevice->pNext, x, y, xSize, 1, BitsPerPixel, 0, (U8 *)_pIndex, 0, 0);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans) {
  int i;
  for (i = 0; i < ySize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 2:
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 4:
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 8:
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      break;
    case 16:
      _DrawBitLine16BPP(pDevice, x0, i + y0, (const U16 *)pData, xSize);
      break;
    case 32:
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      break;
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  int i, NumPixels, BitsPerPixel;
  tLCDDEV_Index2Color * pfIndex2Color;
  tLCDDEV_Color2Index * pfColor2Index;
  LCD_COLOR Color, ColorNew;

  //
  // Get function pointers and read pixels of current device
  //
  NumPixels = x1 - x0 + 1;
  _GetPointers(&pfIndex2Color, &pfColor2Index);
  _GetLine(pDevice, x0, y, NumPixels, _pIndex, _pColor, pfIndex2Color);
  //
  // Mix up buffer with current foreground color
  //
  Color = LCD_Index2Color(LCD__GetColorIndex());
  for (i = 0; i < NumPixels; i++) {
    ColorNew   = _MixColors(Color, *(_pColor + i));
    *(_pIndex + i) = pfColor2Index(ColorNew);
  }
  //
  // Write data to current device
  //
  BitsPerPixel = _GetBitsPerPixel(GUI_Context.SelLayer);
  GUI__CompactPixelIndices(_pIndex, NumPixels, BitsPerPixel);
  pDevice->pNext->pDeviceAPI->pfDrawBitmap(pDevice->pNext, x0, y, NumPixels, 1, BitsPerPixel, 0, (U8 *)_pIndex, 0, 0);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (; y0 <= y1; y0++) {
      _XorPixel(pDevice, x, y0);
    }
  } else {
    for (; y0 <= y1; y0++) {
      _SetPixelIndex(pDevice, x, y0, LCD__GetColorIndex());
    }
  }
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  for (; y0 <= y1; y0++) {
    _DrawHLine(pDevice, x0, y0, x1);
  }
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(Index);
  return NULL;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: Here is nothing to do except routing...
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
}
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  *ppDevice = (*ppDevice)->pNext;
  return (*ppDevice)->pDeviceAPI->pfGetDevFunc(ppDevice, Index);
}
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  return pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
}
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfGetRect(pDevice, pRect);
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
static const GUI_DEVICE_API _Alpha_API = {
  //
  // Data
  //
  DEVICE_CLASS_ALPHA,
  //
  // Drawing functions
  //
  _DrawBitmap   ,
  _DrawHLine    ,
  _DrawVLine    ,
  _FillRect     ,
  _GetPixelIndex,
  _SetPixelIndex,
  _XorPixel     ,
  //
  // Set origin
  //
  _SetOrg       ,
  //
  // Request driver information
  //
  _GetDevFunc   ,
  //
  // Request generic information
  //
  _GetDevProp   ,
  _GetDevData,
  _GetRect      ,
};

/*********************************************************************
*
*       _InitAlpha
*/
static int _InitAlpha(void) {
  int xSize, i, j;
  if (_pIndex == NULL) {
    //
    // Get largest virtual layer size in X
    //
    xSize = LCD_GetVXSizeMax();
    //
    // Create buffer for reading one line of data
    //
    _pIndex = (LCD_PIXELINDEX *)GUI_ALLOC_GetFixedBlock(sizeof(LCD_PIXELINDEX) * xSize);
    if (_pIndex == NULL) {
      return 1; // Error
    }
    if (_pColor == NULL) {
      //
      // Create buffer for mixing up colors of one line
      //
      _pColor = (LCD_COLOR *)GUI_ALLOC_GetFixedBlock(sizeof(LCD_COLOR) * xSize);
      if (_pColor == NULL) {
        //
        // Clean up on error
        //
        GUI_ALLOC_FreeFixedBlock(_pIndex);
        return 1; // Error
      }
      //
      // Create devices for all layers on initialization to make sure that calling
      // GUI_SetColor() or GUI_SetBkColor() or similar does not require further memory
      // allocation!!!
      //
      for (i = 0; i < GUI_NUM_LAYERS; i++) {
        if (_apDevice[i] == NULL) {
          _apDevice[i] = GUI_DEVICE_Create(&_Alpha_API, NULL, 0, i);
          if (_apDevice[i] == NULL) {
            //
            // Clean up on error
            //
            GUI_ALLOC_FreeFixedBlock(_pIndex);
            GUI_ALLOC_FreeFixedBlock(_pColor);
            for (j = 0; j < GUI_NUM_LAYERS; j++) {
              if (_apDevice[j]) {
                GUI_DEVICE_Delete(_apDevice[j]);
              }
            }
            return 1; // Error
          }
        }
      }
    }
  }
  return 0;
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__AlphaSuppressMixing
*/
#if GUI_SUPPORT_MEMDEV
unsigned GUI__AlphaSuppressMixing(int OnOff) {
  unsigned r;

  r = _SuppressMixing;
  if (OnOff) {
    _SuppressMixing = 1;
  } else {
    _SuppressMixing = 0;
  }
  return r;
}
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SetAlpha
*/
unsigned GUI_SetAlpha(U8 Alpha) {
  GUI_DEVICE * pDevice;
  unsigned OldAlpha, LayerIndex;

  LayerIndex = GUI_Context.apDevice[GUI_Context.SelLayer]->LayerIndex;
  OldAlpha = _aAlpha[LayerIndex];
  if (Alpha == 0) {
    pDevice = GUI_DEVICE__GetpDevice(GUI_Context.SelLayer, DEVICE_CLASS_ALPHA);
    if (pDevice) {
      //
      // Remove device from chain
      //
      GUI_DEVICE_Unlink(pDevice);
      //
      // Set alpha value
      //
      _aAlpha[LayerIndex] = Alpha;
    }
  } else {
    if (_InitAlpha() == 0) {
      //
      // Check if alpha blending device already exists
      //
      pDevice = GUI_DEVICE__GetpDevice(GUI_Context.SelLayer, DEVICE_CLASS_ALPHA);
      if (pDevice == NULL) {
        //
        // Check if reusable device exists
        //
        pDevice = _apDevice[GUI_Context.SelLayer];
        if (pDevice) {
          //
          // Link device into chain
          //
          GUI_DEVICE_Link(pDevice);
        }
      }
      //
      // Set alpha value
      //
      _aAlpha[LayerIndex] = Alpha;
    }
  }
  return OldAlpha;
}

/*************************** End of file ****************************/
