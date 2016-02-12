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
File        : GUI_SetOrientationC0.c
Purpose     : Runtime display orientation without cache
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_SetOrientation.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Sort
*
* Purpose:
*   Sorts the values pointed by the given pointers. Please note that
*   the same static function is also in GUI_SetOrientationCX.h
*   to enable better compiler optimization.
*/
static void _Sort(int * p0, int * p1) {
  int temp;

  if (*p0 > *p1) {
    temp = *p0;
    *p0 = *p1;
    *p1 = temp;
  }
}

/*********************************************************************
*
*       Static code: Bitmap drawing routines
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
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const GUI_UNI_PTR * p, int xsize) {
  LCD_PIXELINDEX Pixel;
  void (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  for (; xsize > 0; xsize--, x++, p++) {
    Pixel = *p;
    pfSetPixelIndex(pDevice, x, y, Pixel);
  }
}

/*********************************************************************
*
*       Draw Bitmap 32 BPP
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xsize) {
  LCD_PIXELINDEX Pixel;
  void (* pfSetPixelIndex)(GUI_DEVICE * pDevice, int x, int y, int ColorIndex);

  pfSetPixelIndex = pDevice->pDeviceAPI->pfSetPixelIndex;
  for (; xsize > 0; xsize--, x++, p++) {
    Pixel = *p;
    pfSetPixelIndex(pDevice, x, y, Pixel);
  }
}

/*********************************************************************
*
*       Static code: API functions for drawing operations, no cache
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBitmap_C0
*/
static void _DrawBitmap_C0(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans) {
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
      _DrawBitLine16BPP(pDevice, x0, i + y0, (U16 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  case 32:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine32BPP(pDevice, x0, i + y0, (U32 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/*********************************************************************
*
*       _GetPixelIndex_C0
*/
static unsigned int _GetPixelIndex_C0(GUI_DEVICE * pDevice, int x, int y) {
  int x_phys, y_phys;
  unsigned int PixelIndex;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x, y, &x_phys, &y_phys);
  pDevice = pDevice->pNext;
  PixelIndex = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x_phys, y_phys);
  return PixelIndex;
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex_C0(GUI_DEVICE * pDevice, int x, int y, int PixelIndex) {
  int x_phys, y_phys;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x, y, &x_phys, &y_phys);
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x_phys, y_phys, PixelIndex);
}

/*********************************************************************
*
*       _XorPixel_C0
*/
static void _XorPixel_C0(GUI_DEVICE * pDevice, int x, int y) {
  int x_phys, y_phys;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x, y, &x_phys, &y_phys);
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfXorPixel(pDevice, x_phys, y_phys);
}

/*********************************************************************
*
*       _DrawHLine_C0
*/
static void _DrawHLine_C0(GUI_DEVICE * pDevice, int x0, int y, int x1) {
  int x0_phys, y0_phys, x1_phys, y1_phys;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x0, y, &x0_phys, &y0_phys);
  pContext->pfLog2Phys(pContext, x1, y, &x1_phys, &y1_phys);
  _Sort(&x0_phys, &x1_phys);
  _Sort(&y0_phys, &y1_phys);
  pDevice = pDevice->pNext;
  if (pContext->Orientation & GUI_SWAP_XY) {
    pDevice->pDeviceAPI->pfDrawVLine(pDevice, x0_phys, y0_phys, y1_phys);
  } else {
    pDevice->pDeviceAPI->pfDrawHLine(pDevice, x0_phys, y0_phys, x1_phys);
  }
}

/*********************************************************************
*
*       _DrawVLine_C0
*/
static void _DrawVLine_C0(GUI_DEVICE * pDevice, int x, int y0, int y1) {
  int x0_phys, y0_phys, x1_phys, y1_phys;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x, y0, &x0_phys, &y0_phys);
  pContext->pfLog2Phys(pContext, x, y1, &x1_phys, &y1_phys);
  _Sort(&x0_phys, &x1_phys);
  _Sort(&y0_phys, &y1_phys);
  pDevice = pDevice->pNext;
  if (pContext->Orientation & GUI_SWAP_XY) {
    pDevice->pDeviceAPI->pfDrawHLine(pDevice, x0_phys, y0_phys, x1_phys);
  } else {
    pDevice->pDeviceAPI->pfDrawVLine(pDevice, x0_phys, y0_phys, y1_phys);
  }
}

/*********************************************************************
*
*       _FillRect_C0
*/
static void _FillRect_C0(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  int x0_phys, y0_phys, x1_phys, y1_phys;
  int temp;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pfLog2Phys(pContext, x0, y0, &x0_phys, &y0_phys);
  pContext->pfLog2Phys(pContext, x1, y1, &x1_phys, &y1_phys);
  if (x0_phys > x1_phys) {
    temp = x0_phys;
    x0_phys = x1_phys;
    x1_phys = temp;
  }
  if (y0_phys > y1_phys) {
    temp = y0_phys;
    y0_phys = y1_phys;
    y1_phys = temp;
  }
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfFillRect(pDevice, x0_phys, y0_phys, x1_phys, y1_phys);
}

/*********************************************************************
*
*       Static data: Drawing API(s)
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_OrientationAPI_C0
*/
const GUI_ORIENTATION_API GUI_OrientationAPI_C0 = {
  _DrawBitmap_C0,
  _DrawHLine_C0,
  _DrawVLine_C0,
  _FillRect_C0,
  _GetPixelIndex_C0,
  _SetPixelIndex_C0,
  _XorPixel_C0,
  0
};

/*************************** End of file ****************************/
