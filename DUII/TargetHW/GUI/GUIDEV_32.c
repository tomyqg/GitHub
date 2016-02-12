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
File        : GUIDEV_32.c
Purpose     : Implementation of memory devices
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

#define PIXELINDEX                      U32
#define BITSPERPIXEL                     32
#define API_LIST      GUI_MEMDEV__APIList32
#define DEVICE_API     GUI_MEMDEV_DEVICE_32

/*********************************************************************
*
*       _DrawBitLine32BPP_DDB
*/
static int _GetDistToNextAlpha(const U32 * pSrc, int xsize) {
  int Dist = 0;
  U8 Alpha;
  do {
    Alpha = (*(pSrc++)) >> 24;
    if (Alpha == 0) {
      Dist++;
    }
  } while ((Alpha == 0) && (--xsize));
  return Dist;
}

/*********************************************************************
*
*       _DrawBitLine32BPP_DDB
*/
static void _DrawBitLine32BPP_DDB(GUI_DEVICE * pDevice, GUI_USAGE * pUsage, int x, int y, const U32 * pSrc, int xSize, U32 * pDest) {
  int Dist;
  U8 Alpha;
  U32 Color;

  GUI_USE_PARA(pDevice);
  if (pUsage) {
    GUI_USAGE_AddHLine(pUsage, x, y, xSize);
  }
  do {
    Dist = _GetDistToNextAlpha(pSrc, xSize);
    if (Dist) {
      GUI_MEMCPY(pDest, pSrc, Dist * 4);
      pDest += Dist;
      pSrc  += Dist;
      xSize -= Dist;
    } else {
      Color = *pSrc;
      Alpha = Color >> 24;
      if (Alpha < 255) {
        *pDest = GUI__MixColors(Color, *pDest, 255 - Alpha);
      }
      pDest++;
      pSrc++;
      xSize--;
    }
  } while (xSize);
}

/*********************************************************************
*
*       _DrawBitLine16BPP
*/
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, GUI_USAGE * pUsage, int x, int y, const U16 * pSrc, int xsize, PIXELINDEX * pDest) {
  if (pUsage) {
    GUI_USAGE_AddHLine(pUsage, x, y, xsize);
  }
  do {
    U32 Color;
    U16 Index;
    
    Index = *pSrc;
    Color = pDevice->pColorConvAPI->pfIndex2Color(Index);
    *pDest = Color;
    if (pUsage) {
      GUI_USAGE_AddPixel(pUsage, x, y);
    }
    x++;
    pDest++;
    pSrc++;
  } while (--xsize);
}

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
*       _XY2PTR
*/
static PIXELINDEX* _XY2PTR(int x, int y) {
  GUI_MEMDEV* pDev = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  U8 *pData = (U8*)(pDev + 1);
  #if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
    if ((x >= pDev->x0+pDev->XSize) | (x<pDev->x0) | (y >= pDev->y0+pDev->YSize) | (y<pDev->y0)) {
      GUI_DEBUG_ERROROUT2("_XY2PTR: parameters out of bounds",x,y);
    }
  #endif
  pData += (GUI_ALLOC_DATATYPE_U)(y - pDev->y0) * (GUI_ALLOC_DATATYPE_U)pDev->BytesPerLine;
  return ((PIXELINDEX*)pData) + x - pDev->x0;
}

/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const LCD_PIXELINDEX* pTrans, GUI_MEMDEV* pDev, PIXELINDEX* pDest)
{
  PIXELINDEX Index1;
  PIXELINDEX IndexMask;
  unsigned pixels;
  unsigned PixelCnt;

  GUI_USE_PARA(pDevice);
  PixelCnt = 8 - Diff;
  pixels = LCD_aMirror[*p] >> Diff;

  GUI_DEBUG_ERROROUT3_IF( x < pDev->x0, "GUIDEV.c: DrawBitLine1BPP, Act= %d, Border= %d, Clip= %d", x,pDev->x0, GUI_Context.ClipRect.x0);

  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      /* Write as many pixels as we are allowed to and have loaded in this inner loop */
      do {
        *pDest++ = *(pTrans + (pixels & 1));
        pixels >>= 1;
      } while (--PixelCnt);
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*++p];
    } while (1);
  case LCD_DRAWMODE_TRANS:
    Index1 = *(pTrans + 1);
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      if (pUsage) {
        do {
          if (pixels == 0) {      /* Early out optimization; not required */
            pDest += PixelCnt;
            x     += PixelCnt;
            break;
          }
          if ((pixels & 1)) {
            GUI_USAGE_AddPixel(pUsage, x, y);
            *pDest = Index1;
          }
          x++;
          pDest++;
          if (--PixelCnt == 0) {
            break;
          }
          pixels >>= 1;
        } while (1);
      } else {
        do {
          if (pixels == 0) {      /* Early out optimization; not required */
            pDest += PixelCnt;
            break;
          }
          if ((pixels & 1)) {
            *pDest = Index1;
          }
          pDest++;
          if (--PixelCnt == 0) {
            break;
          }
          pixels >>= 1;
        } while (1);
      }
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*(++p)];
    } while (1);
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask = pDev->pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      /* Prepare loop */
      if (PixelCnt > xsize) {
        PixelCnt = xsize;
      }
      xsize -= PixelCnt;
      /* Write as many pixels as we are allowed to and have loaded in this inner loop */
      do {
        if ((pixels & 1)) {
          *pDest ^= IndexMask;
        }
        pDest++;
        pixels >>= 1;
      } while (--PixelCnt);
      /* Check if an other Source byte needs to be loaded */
      if (xsize == 0) {
        return;
      }
      PixelCnt = 8;
      pixels = LCD_aMirror[*(++p)];
    } while (1);
  }
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void _DrawBitLine2BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, int xsize,
                             const LCD_PIXELINDEX* pTrans, PIXELINDEX* pDest)
{
  U8 pixels;
  U8  PixelCnt;

  GUI_USE_PARA(pDevice);
  PixelCnt = 4 - Diff;
  pixels = (*p) << (Diff << 1);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
  PixelLoopWrite:
    if (PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      *pDest++ = *(pTrans + (pixels >> 6));
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
        *pDest = *(pTrans + (pixels >> 6));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      pDest++;
      x++;
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopTrans;
    }
    break;
  case LCD_DRAWMODE_XOR:;
  PixelLoopXor:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if ((pixels & 0xc0))
        *pDest ^= 255;
      pDest++;
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 4;
      pixels = *(++p);
      goto PixelLoopXor;
    }
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void _DrawBitLine4BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * p, int Diff, int xsize,
                             const LCD_PIXELINDEX* pTrans, PIXELINDEX* pDest)
{
  U8 pixels;
  U8 PixelCnt;

  GUI_USE_PARA(pDevice);
  PixelCnt = 2 - Diff;
  pixels = (*p) << (Diff << 2);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
/*
          * Write mode *
*/
  case 0:
    /* Draw incomplete bytes to the left of center area */
    if (Diff) {
      *pDest = *(pTrans + (pixels >> 4));
      pDest++;
      xsize--;
      pixels = *++p;
    }
    /* Draw center area (2 pixels in one byte) */
    if (xsize >= 2) {
      int i = xsize >> 1;
      xsize &= 1;
      do {
        *pDest     = *(pTrans + (pixels >> 4));   /* Draw 1. (left) pixel */
        *(pDest+1) = *(pTrans + (pixels & 15));   /* Draw 2. (right) pixel */
        pDest += 2;
        pixels = *++p;
      } while (--i);
    }
    /* Draw incomplete bytes to the right of center area */
    if (xsize) {
      *pDest = * (pTrans + (pixels >> 4));
    }
    break;
/*
          * Transparent draw mode *
*/
  case LCD_DRAWMODE_TRANS:
    /* Draw incomplete bytes to the left of center area */
    if (Diff) {
      if (pixels & 0xF0) {
        *pDest = *(pTrans + (pixels >> 4));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      pDest++;
      x++;
      xsize--;
      pixels = *++p;
    }
    /* Draw center area (2 pixels in one byte) */
    while (xsize >= 2) {
      /* Draw 1. (left) pixel */
      if (pixels & 0xF0) {
        *pDest = *(pTrans + (pixels >> 4));
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      /* Draw 2. (right) pixel */
      if (pixels &= 15) {
        *(pDest + 1) = *(pTrans + pixels);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x + 1, y);
        }
      }
      pDest += 2;
      x += 2;
      xsize -= 2;
      pixels = *++p;
    }
    /* Draw incomplete bytes to the right of center area */
    if (xsize) {
      if (pixels >>= 4) {
        *pDest = *(pTrans + pixels);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
    }
    break;
  case LCD_DRAWMODE_XOR:;
  PixelLoopXor:
    if (PixelCnt > xsize)
      PixelCnt = xsize;
    xsize -= PixelCnt;
    do {
      if ((pixels & 0xc0)) {
        *pDest ^= 255;
      }
      pDest++;
      pixels <<= 4;
    } while (--PixelCnt);
    if (xsize) {
      PixelCnt = 2;
      pixels = *(++p);
      goto PixelLoopXor;
    }
    break;
  }
}

/*********************************************************************
*
*       _DrawBitLine8BPP
*/
static void _DrawBitLine8BPP(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * pSrc, int xsize,
                             const LCD_PIXELINDEX* pTrans, PIXELINDEX* pDest) {
  GUI_USE_PARA(pDevice);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      *pDest = *(pTrans + *pSrc);
      pDest++;
      pSrc++;
    } while (--xsize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        *pDest = *(pTrans + *pSrc);
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      x++;
      pDest++;
      pSrc++;
    } while (--xsize);
    break;
  }
}

#if GUI_UNI_PTR_USED
/*********************************************************************
*
*       _Memcopy
*/
static void _Memcopy(PIXELINDEX * pDest, const U8 GUI_UNI_PTR * pSrc, int NumBytes) {
  while(NumBytes--) {
    *pDest++ = *pSrc++;
  }
}
#endif

/*********************************************************************
*
*       _DrawBitLine8BPP_DDB
*/
static void _DrawBitLine8BPP_DDB(GUI_DEVICE * pDevice, GUI_USAGE* pUsage, int x, int y, const U8 GUI_UNI_PTR * pSrc, int xsize, PIXELINDEX* pDest) {
  GUI_USE_PARA(pDevice);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    #if GUI_UNI_PTR_USED
      _Memcopy(pDest, pSrc, xsize);
    #else
      GUI_MEMCPY(pDest, pSrc, xsize);
    #endif
    break;
  case LCD_DRAWMODE_TRANS:
    do {
      if (*pSrc) {
        *pDest = *pSrc;
        if (pUsage) {
          GUI_USAGE_AddPixel(pUsage, x, y);
        }
      }
      x++;
      pDest++;
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
  /* Mark all affected pixels dirty unless transparency is set */
  if (pUsage) {
    if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
      GUI_USAGE_AddRect(pUsage, x0, y0 , xsize, ysize);
    }
  }
  pDest = _XY2PTR(x0, y0);
#if BITSPERPIXEL == 16
  /* handle 16 bpp bitmaps in high color modes, but only without palette */
  if (BitsPerPixel == 16) {
    for (i = 0; i < ysize; i++) {
      _DrawBitLine16BPP_DDB(pDevice, pUsage, x0, i + y0, (const U16*)pData, xsize, pDest);
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  } else if (BitsPerPixel >= 24) {
    for (i = 0; i < ysize; i++) {
      _DrawBitLine32BPP(pDevice, pUsage, x0, i + y0, (const U32 *)pData, xsize, pDest);
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  }
#endif
#if BITSPERPIXEL == 32
  /* handle 32 bpp bitmaps in high color modes, but only without palette */
  if (BitsPerPixel >= 24) {
    for (i = 0; i < ysize; i++) {
      _DrawBitLine32BPP_DDB(pDevice, pUsage, x0, i + y0, (const U32*)pData, xsize, pDest);
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  } else if (BitsPerPixel == 16) {
    for (i = 0; i < ysize; i++) {
      _DrawBitLine16BPP(pDevice, pUsage, x0, i + y0, (const U16 *)pData, xsize, pDest);
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  }
#endif
  /* Handle 8 bpp bitmaps seperately as we have different routine bitmaps with or without palette */
  if (BitsPerPixel == 8) {
    for (i = 0; i < ysize; i++) {
      if (pTrans) {
        _DrawBitLine8BPP(pDevice, pUsage, x0, i + y0, pData, xsize, pTrans, pDest);
      } else {
        _DrawBitLine8BPP_DDB(pDevice, pUsage, x0, i + y0, pData, xsize, pDest);
      }
      pData += BytesPerLine;
      pDest = (PIXELINDEX*)((U8*)pDest + BytesPerLineDest); 
    }
    return;
  }
  /* Use aID for bitmaps without palette */
  if (!pTrans) {
    pTrans = aID;
  }
  for (i = 0; i < ysize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDev, pDest);
      break;
    case 2:
      _DrawBitLine2BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDest);
      break;
    case 4:
      _DrawBitLine4BPP(pDevice, pUsage, x0, i + y0, pData, Diff, xsize, pTrans, pDest);
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
  unsigned BytesPerLine, Cnt;
  int Len, RemPixels;
  GUI_MEMDEV * pDev;
  PIXELINDEX * pData;
  PIXELINDEX * pDest;
  PIXELINDEX IndexMask;
  LCD_PIXELINDEX ColorIndex;

  GUI_USE_PARA(pDevice);
  pDev         = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  pData        = _XY2PTR(x0, y0);
  BytesPerLine = pDev->BytesPerLine;
  Len          = x1 - x0 + 1;
  ColorIndex   = LCD__GetColorIndex();
  /* Mark rectangle as modified */
  if (pDev->hUsage) {
    GUI_USAGE_AddRect(GUI_USAGE_H2P(pDev->hUsage), x0, y0, Len, y1 - y0 + 1);
  }
  /* Do the drawing */
  for (; y0 <= y1; y0++) {
    if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
      RemPixels = Len;
      pDest  = pData;
      IndexMask = pDev->pDevice->pColorConvAPI->pfGetIndexMask();
      do {
        *pDest = *pDest ^ IndexMask;
        pDest++;
      } while (--RemPixels);
    } else {  /* Fill */
      #if BITSPERPIXEL == 8
        GUI_MEMSET(pData, ColorIndex, Len);
      #elif BITSPERPIXEL == 16
        GUI__memset16(pData, ColorIndex, Len);
      #elif (BITSPERPIXEL == 24) || (BITSPERPIXEL == 32)
        if (Len > 0) {
          Cnt = Len;
          pDest = pData;
          do {
            *pDest++ = ColorIndex;
          } while (--Cnt);
        }
      #else
        #error Unsupported
      #endif
    }
    pData = (PIXELINDEX*)((U8*)pData + BytesPerLine); 
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
  GUI_USAGE_h hUsage; 
  GUI_USAGE  * pUsage;
  PIXELINDEX * pData;
  unsigned BytesPerLine;
  LCD_PIXELINDEX ColorIndex;

  GUI_USE_PARA(pDevice);
  pDev   = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  hUsage = pDev->hUsage; 
  pUsage = hUsage ? GUI_USAGE_H2P(hUsage) : NULL;
  pData  = _XY2PTR(x, y0);
  BytesPerLine = pDev->BytesPerLine;
  ColorIndex = LCD__GetColorIndex();
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    PIXELINDEX IndexMask;
    IndexMask = pDev->pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      if (hUsage) {
        GUI_USAGE_AddPixel(pUsage, x, y0);
      }
      *pData = *pData ^ IndexMask;
      pData = (PIXELINDEX*)((U8*)pData + pDev->BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
    } while (++y0 <= y1);
  } else {
    if (hUsage) {
      do {
        GUI_USAGE_AddPixel(pUsage, x, y0);
        *pData = ColorIndex;
        pData = (PIXELINDEX*)((U8*)pData + BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
      } while (++y0 <= y1);
    } else {
      unsigned NumPixels;
      NumPixels = y1 - y0 + 1;
      do {
        *pData = ColorIndex;
        pData = (PIXELINDEX*)((U8*)pData + BytesPerLine); /* Same as "pData += pDev->BytesPerLine >> 1;", Just more efficient */
      } while (--NumPixels);
    }
  }
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int Index) {
  GUI_MEMDEV * pDev;
  GUI_USAGE_h hUsage; 
  PIXELINDEX * pData;

  GUI_USE_PARA(pDevice);
  pDev   = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  hUsage = pDev->hUsage; 
  pData  = _XY2PTR(x, y);
  *pData = Index;
  if (hUsage) {
    GUI_USAGE_AddPixel(GUI_USAGE_H2P(hUsage), x, y);
  }
}

/*********************************************************************
*
*       _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  PIXELINDEX IndexMask;
  GUI_MEMDEV * pDev;
  GUI_USAGE_h hUsage; 
  PIXELINDEX * pData;

  GUI_USE_PARA(pDevice);
  pDev      = GUI_MEMDEV_H2P(GUI_Context.hDevData);
  hUsage    = pDev->hUsage; 
  pData     = _XY2PTR(x, y);
  IndexMask = pDev->pDevice->pColorConvAPI->pfGetIndexMask();
  *pData    = *pData ^ IndexMask;
  if (hUsage) {
    GUI_USAGE_AddPixel(GUI_USAGE_H2P(hUsage), x, y);
  }
}

/*********************************************************************
*
*       _GetPixelIndex
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) {
  PIXELINDEX * pData;
  
  GUI_USE_PARA(pDevice);
  pData = _XY2PTR(x, y);
  return *pData;
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  GUI_MEMDEV * pDev = GUI_MEMDEV_H2P(pDevice->u.hContext);
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
    return (void (*)(void))GUI_MEMDEV__WriteToActiveAlpha;
    //return (void (*)(void))GUI_MEMDEV__WriteToActiveOpaque;
  }
  return NULL;
}

/*********************************************************************
*
*       _GetDevProp
*/
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  switch(Index) {
  case LCD_DEVCAP_BITSPERPIXEL:
    return 32;
  }
  return 0;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: From here there is nothing to do except routing...
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
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

void GUIDEV_32_C(void);
void GUIDEV_32_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
