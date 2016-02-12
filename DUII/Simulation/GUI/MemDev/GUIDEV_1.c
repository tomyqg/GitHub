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
File        : GUIDEV_1.c
Purpose     : Implementation of memory devices
              This file handles 1 bit memory devices.
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Macros
*
**********************************************************************
*/

#define PIXELINDEX                      U8
#define API_LIST      GUI_MEMDEV__APIList1
#define DEVICE_API     GUI_MEMDEV_DEVICE_1

/*********************************************************************
*
*       static consts
*
**********************************************************************
*/
/*********************************************************************
*
*       ID translation table
*
* This table serves as translation table for DDBs
*/
static const LCD_PIXELINDEX aID[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

/*********************************************************************
*
*       static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _XY2PTR_BITOFFSET
*/
static U8* _XY2PTR_BITOFFSET(int x, int y, int* pBitOffset) {
  GUI_ALLOC_DATATYPE_U Offset;
  GUI_MEMDEV* pDev;
  U8* pData;
  pDev  = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  pData = (U8*)(pDev + 1);
  #if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
    if ((x >= pDev->x0+pDev->XSize) | (x<pDev->x0) | (y >= pDev->y0+pDev->YSize) | (y<pDev->y0)) {
      GUI_DEBUG_ERROROUT2("_XY2PTR: parameters out of bounds", x, y);
    }
  #endif
  x -= pDev->x0;
  y -= pDev->y0;
  Offset = (GUI_ALLOC_DATATYPE_U)(y) * (GUI_ALLOC_DATATYPE_U)(pDev->BytesPerLine) + (x >> 3);
  if (pBitOffset) {
    *pBitOffset = 7 - (x & 7);
  }
  return pData + Offset;
}

/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const LCD_PIXELINDEX* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest)
{
  PIXELINDEX pixels;
  PIXELINDEX Index1;
  unsigned int PixelCnt;
  U8 AndMask;
  U8 * pData;
  int Bit;
  unsigned NumPixels;

  GUI_USE_PARA(pUsage);
  PixelCnt = 8 - (Diff & 7);
  pixels = (*p) << (Diff & 7);
  GUI_DEBUG_ERROROUT3_IF( x < pDev->x0, "GUIDEV.c: DrawBitLine1BPP, Act= %d, Border= %d, Clip= %d"
                    ,x,pDev->x0, GUI_Context.ClipRect.x0);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    // Write mode
    pData = _XY2PTR_BITOFFSET(x, y, &Bit);
    //
    // First byte
    //
    if (Bit < 7) {
      AndMask = 0xff << (Bit + 1);
      AndMask |= (AndMask ^ 0xff) >> xsize;
      *pData &= AndMask;
      NumPixels = Bit + 1;
      if (NumPixels > xsize) {
        NumPixels = xsize;
      }
      xsize -= NumPixels;
      do {
        *pData |= *(pTrans + ((U8)pixels >> 7)) << Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
      } while (--NumPixels);
      pData++;
    }
    //
    // Complete byte
    //
    while (xsize >= 8) {
      *pData = 0;
      for (Bit = 7; Bit >= 0; ) {
        *pData |= *(pTrans + ((U8)pixels >> 7)) << Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
      }
      pData++;
      xsize -= 8;
    }
    //
    // Last byte
    //
    if (xsize) {
      Bit = 7;
      AndMask = 0xff >> xsize;
      *pData &= AndMask;
      do {
        *pData |= *(pTrans + ((U8)pixels >> 7)) << Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    Index1 = *(pTrans + 1);
    pData = _XY2PTR_BITOFFSET(x, y, &Bit);
    //
    // First byte
    //
    if (Bit < 7) {
      NumPixels = Bit + 1;
      if (NumPixels > xsize) {
        NumPixels = xsize;
      }
      xsize -= NumPixels;
      do {
        if (pixels & 0x80) {
          if (Index1) {
            *pData |= 1 << Bit;
          } else {
            *pData &= ~(1 << Bit);
          }
          if (pUsage) {
            GUI_USAGE_AddPixel(pUsage, x, y);
          }
        }
        Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
        x++;
      } while (--NumPixels);
      pData++;
    }
    //
    // Complete byte
    //
    while (xsize >= 8) {
      for (Bit = 7; Bit >= 0; ) {
        if (pixels & 0x80) {
          if (Index1) {
            *pData |= 1 << Bit;
          } else {
            *pData &= ~(1 << Bit);
          }
          if (pUsage) {
            GUI_USAGE_AddPixel(pUsage, x, y);
          }
        }
        Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
        x++;
      }
      pData++;
      xsize -= 8;
    }
    //
    // Last byte
    //
    if (xsize) {
      Bit = 7;
      do {
        if (pixels & 0x80) {
          if (Index1) {
            *pData |= 1 << Bit;
          } else {
            *pData &= ~(1 << Bit);
          }
          if (pUsage) {
            GUI_USAGE_AddPixel(pUsage, x, y);
          }
        }
        Bit--;
        pixels <<= 1;
        if (--PixelCnt == 0) {
          PixelCnt = 8;
          pixels = *(++p);
        }
        x++;
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
  PixelLoopXor:
    if (PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      if ((pixels & 0x80)) {
        (*pDev->pDevice->pDeviceAPI->pfXorPixel)(pDevice, x, y);
      }
      x++;
      pDest++;
      pixels <<= 1;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 8;
      pixels = *(++p);
      goto PixelLoopXor;
    }
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void _DrawBitLine2BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, int xsize,
                             const LCD_PIXELINDEX* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest)
{
  U8 pixels;
  U8  PixelCnt;
  GUI_USE_PARA(pUsage);
  GUI_USE_PARA(pDest);
  PixelCnt = 4 - (Diff & 3);
  pixels = (*p) << ((Diff & 3) << 1);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    // Write mode
  PixelLoopWrite:
    if (PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + (pixels >> 6)));
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopWrite;
    }
    break;
  case LCD_DRAWMODE_TRANS:
  PixelLoopTrans:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if (pixels & 0xc0) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *(pTrans + (pixels >> 6)));
      }
      x++;
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopTrans;
    }
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void _DrawBitLine4BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, int xsize,
                             const LCD_PIXELINDEX* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest)
{
  U8 pixels;
  GUI_USE_PARA(pUsage);
  GUI_USE_PARA(pDest);
  pixels = (*p) << ((Diff & 1) << 2);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    //
    // Draw incomplete bytes to the left of center area
    //
    if (Diff) {
      (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + (pixels >> 4)));
      xsize--;
      pixels = *++p;
    }
    //
    // Draw center area (2 pixels in one byte)
    //
    if (xsize >= 2) {
      int i = xsize >> 1;
      xsize &= 1;
      do {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + (pixels >>  4)));
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + (pixels  & 15)));
        pixels = *++p;
      } while (--i);
    }
    //
    // Draw incomplete bytes to the right of center area
    //
    if (xsize) {
      (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + (pixels >> 4)));
    }
    break;
  case LCD_DRAWMODE_TRANS:
    //
    // Draw incomplete bytes to the left of center area
    //
    if (Diff) {
      if (pixels & 0xF0) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *(pTrans + (pixels >> 4)));
      }
      x++;
      xsize--;
      pixels = *++p;
    }
    //
    // Draw center area (2 pixels in one byte)
    //
    while (xsize >= 2) {
      //
      // Draw 1. (left) pixel
      //
      if (pixels & 0xF0) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *(pTrans + (pixels >> 4)));
      }
      //
      // Draw 2. (right) pixel
      //
      if (pixels &= 15) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x + 1, y, *(pTrans + pixels));
      }
      x += 2;
      xsize -= 2;
      pixels = *++p;
    }
    //
    // Draw incomplete bytes to the right of center area
    //
    if (xsize) {
      if (pixels >>= 4) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *(pTrans + pixels));
      }
    }
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine8BPP
*/
static void _DrawBitLine8BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * pSrc, int xsize,
                             const LCD_PIXELINDEX* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest) {
  GUI_USE_PARA(pUsage);
  GUI_USE_PARA(pDest);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    // Write mode
    do {
      (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *(pTrans + *pSrc));
      pSrc++;
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *(pTrans + *pSrc));
      }
      x++;
      pSrc++;
    } while (--xsize);
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine8BPP_DDB
*/
static void _DrawBitLine8BPP_DDB(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * pSrc, int xsize, GUI_MEMDEV* pDev, PIXELINDEX* pDest) {
  GUI_USE_PARA(pUsage);
  GUI_USE_PARA(pDest);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    // Write mode
    do {
      (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x++, y, *pSrc);
      pSrc++;
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        (*pDev->pDevice->pDeviceAPI->pfSetPixelIndex)(pDevice, x, y, *pSrc);
      }
      x++;
      pSrc++;
    } while (--xsize);
    break;
  }
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0, int xsize, int ysize,
                        int BitsPerPixel, int BytesPerLine,
                        const U8 GUI_UNI_PTR * pData, int Diff, const LCD_PIXELINDEX* pTrans)
{
  int i;
  GUI_MEMDEV* pDev   = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  GUI_USAGE*  pUsage = (pDev->hUsage) ? GUI_USAGE_H2P(pDev->hUsage) : 0;
  unsigned    BytesPerLineDest;
  PIXELINDEX* pDest;
  BytesPerLineDest = pDev->BytesPerLine;
  x0 += Diff;
  //
  // Mark all affected pixels dirty unless transparency is set
  //
  if (pUsage) {
    if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
      GUI_USAGE_AddRect(pUsage, x0, y0 , xsize, ysize);
    }
  }
  pDest = _XY2PTR_BITOFFSET(x0, y0, 0);
  //
  // Handle 8 bpp bitmaps seperately as we have different routine bitmaps with or without palette
  //
  if (BitsPerPixel == 8) {
    for (i = 0; i < ysize; i++) {
      if (pTrans) {
        _DrawBitLine8BPP(pDevice, pUsage, x0, i + y0, pData, xsize, pTrans, pDev, pDest);
      } else {
        _DrawBitLine8BPP_DDB(pDevice, pUsage, x0, i + y0, pData, xsize, pDev, pDest);
      }
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  }
  //
  // Use aID for bitmaps without palette
  //
  if (!pTrans) {
    pTrans = aID;
  }
  for (i = 0; i < ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
      break;
    case 2:
      _DrawBitLine2BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
      break;
    case 4:
      _DrawBitLine4BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
      break;
    }
    pData += BytesPerLine;
    pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
  }
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  GUI_MEMDEV * pDev;
  U8 * pData;
  int Bit, Len, NumBytes, RemPixels, Color, FillByte;
  LCD_PIXELINDEX ColorIndex;

  GUI_USE_PARA(pDevice);
  pDev = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  Len = x1 - x0 + 1;
  //
  // Mark rectangle as modified
  //
  if (pDev->hUsage) {
    GUI_USAGE_AddRect(GUI_USAGE_H2P(pDev->hUsage), x0, y0, Len, y1 - y0 + 1);
  }
  //
  // Do the drawing
  //
  for (; y0 <= y1; y0++) {
    pData = _XY2PTR_BITOFFSET(x0, y0, &Bit);
    RemPixels = Len;
    if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
      if (Bit < 7) {
        while ((Bit >= 0) && RemPixels--) {
          *pData ^= 1 << (Bit--);
        }
        pData++;
      }
      if (RemPixels > 0) {
        NumBytes = RemPixels >> 3;
        if (NumBytes > 0) {
          RemPixels -= NumBytes << 3;
          do {
            *pData ^= 0xFF;
            pData++;
          } while (--NumBytes);
        }
        Bit = 7;
        while (RemPixels--) {
          *pData ^= 1 << (Bit--);
        }
      }
    } else {  // Fill
      ColorIndex = LCD__GetColorIndex();
      Color    = (ColorIndex & 1);
      FillByte = (-Color) & 0xFF;
      if (Bit < 7) {
        while ((Bit >= 0) && RemPixels--) {
          *pData &= ~(1 << Bit);
          *pData |= Color << (Bit--);
        }
        pData++;
      }
      if (RemPixels > 0) {
        NumBytes = RemPixels >> 3;
        if (NumBytes > 0) {
          GUI_MEMSET(pData, FillByte, NumBytes);
          pData += NumBytes;
          RemPixels -= NumBytes << 3;
        }
        Bit = 7;
        while (RemPixels--) {
          *pData &= ~(1 << Bit);
          *pData |= Color << (Bit--);
        }
      }
    }
  }
}

/*********************************************************************
*
*       _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y, int x1) {
  _FillRect(pDevice, x0, y, x1, y);
}

/*********************************************************************
*
*       _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x , int y0, int y1) {
  GUI_MEMDEV * pDev;
  GUI_USAGE_h  hUsage; 
  GUI_USAGE  * pUsage;
  U8         * pData;
  int Bit, Mask, Pixel;
  LCD_PIXELINDEX ColorIndex;

  GUI_USE_PARA(pDevice);
  pDev   = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  hUsage = pDev->hUsage; 
  pUsage = hUsage ? GUI_USAGE_H2P(hUsage) : NULL;
  pData  = _XY2PTR_BITOFFSET(x, y0, &Bit);
  Mask   = (1 << Bit);
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    do {
      *pData ^= Mask;
      if (pUsage) {
        GUI_USAGE_AddPixel(pUsage, x, y0);
      }
      pData += pDev->BytesPerLine;
    } while (++y0 <= y1);
  } else {
    ColorIndex = LCD__GetColorIndex();
    Pixel = (ColorIndex & 1) << Bit;
    do {
      *pData &= ~Mask;
      *pData |= Pixel;
      if (pUsage) {
        GUI_USAGE_AddPixel(pUsage, x, y0);
      }
      pData += pDev->BytesPerLine;
    } while (++y0 <= y1);
  }
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int Index) {
  GUI_MEMDEV * pDev;
  U8 * pData;
  int Bit;

  GUI_USE_PARA(pDevice);
  pDev    = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  pData   = _XY2PTR_BITOFFSET(x, y, &Bit);
  *pData &= ~(1 << Bit);
  *pData |= (Index & 1) << Bit;
  if (pDev->hUsage) {
    GUI_USAGE_AddPixel(GUI_USAGE_H2P(pDev->hUsage), x, y);
  }
}

/*********************************************************************
*
*       _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  GUI_MEMDEV * pDev;
  U8 * pData;
  int Bit;

  GUI_USE_PARA(pDevice);
  pDev    = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  pData   = _XY2PTR_BITOFFSET(x, y, &Bit);
  *pData ^= (1 << Bit);
  if (pDev->hUsage) {
    GUI_USAGE_AddPixel(GUI_USAGE_H2P(pDev->hUsage), x, y);
  }
}

/*********************************************************************
*
*       _GetPixelIndex
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) {
  U8 * pData;
  int Bit;

  GUI_USE_PARA(pDevice);
  pData = _XY2PTR_BITOFFSET(x, y, &Bit);
  return (*pData >> Bit) & 1;
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  GUI_MEMDEV * pDev;
  
  pDev = GUI_MEMDEV_H2P(pDevice->u.hContext);
  pRect->x0 = pDev->x0;
  pRect->y0 = pDev->y0;
  pRect->x1 = pDev->x0 + pDev->XSize-1;
  pRect->y1 = pDev->y0 + pDev->YSize-1;
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  switch (Index) {
  case LCD_DEVDATA_MEMDEV:
    return (void *)&DEVICE_API;
  }
  return NULL;
}

/*********************************************************************
*
*       _GetDevFunc
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case MEMDEV_DEVFUNC_WRITETOACTIVE:
    return (void (*)(void))GUI_MEMDEV__WriteToActiveOpaque;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: From here there is nothing to do except routing...
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
}
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  return pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
const GUI_DEVICE_API DEVICE_API = {
  //
  // Data
  //
  DEVICE_CLASS_MEMDEV,
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
  // Initialization
  //
  //
  // Request information
  //
  _GetDevFunc   ,
  _GetDevProp   ,
  _GetDevData,
  _GetRect      ,
};



#else

void GUIDEV1_C(void);
void GUIDEV1_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
