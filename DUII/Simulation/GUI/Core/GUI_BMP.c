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
File        : GUI_BMP.c
Purpose     : Implementation of GUI_BMP... functions
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI_BMP_Private.h"

#define USE_NEXT_PIXEL 1

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
#if USE_NEXT_PIXEL
/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int x0, xPos, yPos, xSize;
  GUI_HMEM hMem;            /* Handle of buffer */
  U8 * pBuffer; /* Pointer to allocated buffer */
  U8 * pDst;    /* Pointer for storing next pixel */
  int BytesPerPixel;
  int BitsPerPixel;
  int BufferSizeInPixels;
  int NumPixelsInBuffer;
  int Bit;
} NEXT_PIXEL_CONTEXT;

typedef struct {
  int DataOff;
  int BytesPerLine;
  int ClipRight;
  int ClipLeft;
  int ClipTop;
  int ClipBottom;
} CLIP_PARAM_BMP;

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static NEXT_PIXEL_CONTEXT _Context;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _NextPixel_FlushBuffer
*/
static void _NextPixel_FlushBuffer(void) {
  const LCD_PIXELINDEX aTrans[] = {0, 1};
  const LCD_PIXELINDEX * pTrans;
  pTrans = (_Context.BitsPerPixel == 1) ? aTrans : NULL;
  LCD_DrawBitmap(_Context.xPos, _Context.yPos, _Context.xSize, 1, 1, 1, _Context.BitsPerPixel, 0, (const U8 *)_Context.pBuffer, pTrans);
  _Context.xPos += _Context.NumPixelsInBuffer;
  _Context.NumPixelsInBuffer = 0;
  _Context.pDst = _Context.pBuffer;
}

/*********************************************************************
*
*       _NextPixel_Start
*/
static void _NextPixel_Start(int x0, int y0, int x1, int y1) {
  U32 BufferSizeAvailable; /* Number of available bytes */
  int BufferSize;          /* Buffer size in bytes */
  int BytesPerPixel;       /* Size of LCD_PIXELINDEX */
  int BitsPerPixel;
  int BytesPerLineDest;
  GUI_USE_PARA(y0);
  _Context.xSize = x1 - x0 + 1;
  /*
   * Calculate buffer size
   */
  //BitsPerPixel          = LCD_GetBitsPerPixelEx(GUI_Context.SelLayer);
  BitsPerPixel          = GUI_GetBitsPerPixelEx(GUI_Context.SelLayer);
  _Context.BitsPerPixel = (BitsPerPixel < 24) ? BitsPerPixel : 32;
  BytesPerPixel         = (_Context.BitsPerPixel <= 8) ? 1 : (_Context.BitsPerPixel <= 16) ? 2 : 4;
  BytesPerLineDest      = BytesPerPixel * _Context.xSize;
  BufferSizeAvailable   = GUI_ALLOC_GetMaxSize();
  if (BufferSizeAvailable < (U32)BytesPerLineDest) {
    BufferSize = (BufferSizeAvailable / BytesPerPixel) * BytesPerPixel;
  } else {
    BufferSize = BytesPerLineDest;
  }
  _Context.BufferSizeInPixels = BufferSize / BytesPerPixel;
  _Context.NumPixelsInBuffer  = 0;
  _Context.xPos = _Context.x0 = x0;
  _Context.yPos = y1;
  _Context.BytesPerPixel = BytesPerPixel;
  _Context.Bit = 0;
  /*
   * Allocate buffer
   */
  _Context.hMem    = GUI_ALLOC_AllocNoInit(BytesPerLineDest);
  _Context.pBuffer = (U8 *)GUI_ALLOC_h2p(_Context.hMem);
  _Context.pDst    = _Context.pBuffer;
}

/*********************************************************************
*
*       _NextPixel_SetPixel
*/
static void _NextPixel_SetPixel(int PixelIndex) {
  switch (_Context.BytesPerPixel) {
  case 1: {
      int Shift;
      U8 * pData;
      pData = (U8 *)_Context.pDst;
      switch (_Context.BitsPerPixel) {
      case 1:
        Shift = 7 - _Context.Bit;
        *pData &= ~(1          << Shift);
        *pData |=  (PixelIndex << Shift);
        _Context.Bit += 1;
        if (_Context.Bit == 8) {
          _Context.Bit   = 0;
          _Context.pDst += 1;
        }
        break;
      case 2:
        Shift = 6 - _Context.Bit;
        *pData &= ~(3          << Shift);
        *pData |=  (PixelIndex << Shift);
        _Context.Bit += 2;
        if (_Context.Bit == 8) {
          _Context.Bit   = 0;
          _Context.pDst += 1;
        }
        break;
      case 4:
        Shift = 4 - _Context.Bit;
        *pData &= ~(0xf        << Shift);
        *pData |=  (PixelIndex << Shift);
        _Context.Bit += 4;
        if (_Context.Bit == 8) {
          _Context.Bit   = 0;
          _Context.pDst += 1;
        }
        break;
      case 8:
        *pData = PixelIndex;
        _Context.pDst += 1;
        break;
      }
    }
    break;
  case 2: {
      U16 * pData;
      pData = (U16 *)_Context.pDst;
      *pData = PixelIndex;
      _Context.pDst += 2;
    }
    break;
  case 4: {
      U32 * pData;
      pData = (U32 *)_Context.pDst;
      *pData = PixelIndex;
      _Context.pDst += 4;
    }
    break;
  }
  if (++_Context.NumPixelsInBuffer == _Context.BufferSizeInPixels) {
    _NextPixel_FlushBuffer();
  }
}

/*********************************************************************
*
*       _NextPixel_NextLine
*/
static void _NextPixel_NextLine(void) {
  if (_Context.NumPixelsInBuffer) {
    _NextPixel_FlushBuffer();
  }
  _Context.xPos = _Context.x0;
  _Context.yPos--;
  _Context.Bit = 0;
}

/*********************************************************************
*
*       _NextPixel_End
*/
static void _NextPixel_End(void) {
  if (_Context.NumPixelsInBuffer) {
    _NextPixel_FlushBuffer();
  }
  GUI_ALLOC_Free(_Context.hMem);
}

/*********************************************************************
*
*       _NextPixel_API
*/
static LCD_API_NEXT_PIXEL _NextPixel_API = {
  _NextPixel_Start,
  _NextPixel_SetPixel,
  _NextPixel_NextLine,
  _NextPixel_End
};

#endif

/*********************************************************************
*
*       _GetStep
*/
static int _GetStep(int * pYSize, int * pY) {
  if (*pYSize > 0) {
    *pY = *pYSize - 1;
    return -1;
  } else if (*pYSize < 0) {
    *pYSize = -*pYSize;
    *pY = 0;
    return 1;
  } else {
    return 0;
  }
}

/*********************************************************************
*
*       _DrawBitmap_RLE
*/
static int _DrawBitmap_RLE(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize, int NumColors, int Compression) {
  int x, y, EOI, Dx, Dy;
  const U8 * pData;
  int ColorIndex;
  EOI = 0;
  ColorIndex = LCD__GetColorIndex();
  /* Read palette */
  if (GUI_BMP__ReadPalette(pContext, NumColors)) {
    return 1;
  }
  /* Decompress pixel data */
  x = x0;
  y = y0 + YSize - 1;
  do {
    U8 Cmd;
    U8 Data;
    if (GUI_BMP__ReadData(pContext, 2, &pData, 0)) {
      return 1;
    }
    Cmd  = *pData++;
    Data = *pData;
    if (Cmd == 0) {
      switch (Data) {
      case 0:   /* End of line */
        if (x < x0 + XSize - 1) {
          LCD__SetColorIndex(LCD__aConvTable[0]);
          LCD_DrawHLine(x, y, x0 + XSize - 1 );
        }
        x = x0;
        y--;
        if (y < y0) {
          return 1; /* Error: Data after end of image */
        }
        break;
      case 1:   /* End of image */
        if (x < x0 + XSize - 1) {
          LCD__SetColorIndex(LCD__aConvTable[0]);
          LCD_DrawHLine(x, y, x0 + XSize - 1 );
        }
        if (y > y0) {
          LCD__SetColorIndex(LCD__aConvTable[0]);
          LCD_FillRect(x0, y0, x0 + XSize - 1, y - 1);
        }
        EOI = 1;
        break;
      case 2:   /* Delta */
        if (GUI_BMP__ReadData(pContext, 2, &pData, 0)) {
          return 1;
        }
        Dx = *pData++;
        Dy = *pData;
        LCD__SetColorIndex(LCD__aConvTable[0]);
        LCD_FillRect(x, y - Dy, x + Dx, y);
        x += Dx;
        y -= Dy;
        break;
      default:  /* Absolute mode */
        switch (Compression) {
        case BI_RLE4:
          if (GUI_BMP__ReadData(pContext, (Data + 1) / 2, &pData, 0)) {
            return 1;
          }
          LCD_DrawBitmap(x, y, Data, 1, 1, 1, 4, (Data + 1) / 2, pData, LCD__aConvTable);
          x += Data;
          /* Make sure we are on an even 16bit-adr */
          if (((Data + 1) >> 1) & 1) {
            if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
              return 1;
            }
          }
          break;
        case BI_RLE8:
          if (GUI_BMP__ReadData(pContext, Data, &pData, 0)) {
            return 1;
          }
          LCD_DrawBitmap(x, y, Data, 1, 1, 1, 8, Data, pData, LCD__aConvTable);
          x += Data;
          /* Make sure we are on an even 16bit-adr */
          if (Data & 1) {
            if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
              return 1;
            }
          }
          break;
        }
        break;
      }
    } else {
      switch (Compression) {
      case BI_RLE4:
        if ((Data & 0xf) == (Data >> 4)) {
          LCD__SetColorIndex(LCD__aConvTable[Data & 0xf]);
          LCD_DrawHLine(x, y, x + Cmd - 1);
          x += Cmd;
        } else {
          int i;
          for (i = 0; i < Cmd; i++) {
            if ((i & 1) ==0) {
              LCD__SetColorIndex(LCD__aConvTable[Data >> 4]);
            } else {
              LCD__SetColorIndex(LCD__aConvTable[Data & 15]);
            }
            LCD_DrawPixel(x++, y);
          }
        }
        break;
      case BI_RLE8:
        LCD__SetColorIndex(LCD__aConvTable[Data]);
        LCD_DrawHLine(x, y, x + Cmd - 1);
        x += Cmd;
        break;
      }
    }
  } while (!EOI);
  LCD__SetColorIndex(ColorIndex);
  return 0;
}

/*********************************************************************
*
*       _GetClipParam
*/
static void _GetClipParam(int x0, int y0, int XSize, int YSize, int BitsPerPixel, int BytesPerLine, CLIP_PARAM_BMP * pParam) {
  int Diff;
  /*
  * Left
  */
  Diff = GUI_Context.ClipRect.x0 - x0;
  if (Diff > 0) {
    pParam->ClipLeft = Diff;
  }
  /*
  * Top
  */
  Diff = GUI_Context.ClipRect.y0 - y0;
  if (Diff > 0) {
    pParam->ClipTop = Diff;
  }
  /*
  * Right
  */
  Diff = x0 + XSize - 1 - GUI_Context.ClipRect.x1;
  if (Diff > 0) {
    pParam->ClipRight = Diff;
  }
  /*
  * Bottom
  */
  Diff = y0 + YSize - 1 - GUI_Context.ClipRect.y1;
  if (Diff > 0) {
    pParam->ClipBottom = Diff;
  }
  /*
  * Data offset
  */
  switch (BitsPerPixel) {
  case 1:
    pParam->DataOff      = pParam->ClipLeft >> 3;
    pParam->ClipLeft     = pParam->DataOff << 3;
    pParam->BytesPerLine = BytesPerLine - (pParam->ClipRight >> 3) - pParam->DataOff;
    break;
  case 4:
    pParam->DataOff      = pParam->ClipLeft >> 1;
    pParam->ClipLeft     = pParam->DataOff << 1;
    pParam->BytesPerLine = BytesPerLine - (pParam->ClipRight >> 1) - pParam->DataOff;
    break;
  case 8:
    pParam->DataOff      = pParam->ClipLeft;
    pParam->BytesPerLine = BytesPerLine - pParam->ClipRight        - pParam->DataOff;
    break;
  case 16:
    pParam->DataOff      = pParam->ClipLeft << 1;
    pParam->BytesPerLine = BytesPerLine - (pParam->ClipRight << 1) - pParam->DataOff;
    break;
  case 24:
    pParam->DataOff      = pParam->ClipLeft * 3;
    pParam->BytesPerLine = BytesPerLine - (pParam->ClipRight  * 3) - pParam->DataOff;
    break;
  case 32:
    pParam->DataOff      = pParam->ClipLeft << 2;
    pParam->BytesPerLine = BytesPerLine - (pParam->ClipRight << 2) - pParam->DataOff;
    break;
  }
}

/*********************************************************************
*
*       _DrawBitmap_Pal
*/
static int _DrawBitmap_Pal(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize, int Bpp, int NumColors) {
  int y, Step, SkipBytes;
  int BytesPerLine = 0;
  const U8 * pData;
  CLIP_PARAM_BMP Param = {0};

  if (GUI_BMP__ReadPalette(pContext, NumColors)) {
    return 1;
  }
  switch (Bpp) {
  case 1:
    BytesPerLine = ((XSize + 31) >> 5) << 2;
    break;
  case 4:
    BytesPerLine = (((XSize << 2) + 31) >> 5) << 2;
    break;
  case 8:
    BytesPerLine = ((XSize +  3) >> 2) << 2;
    break;
  }
  /*
  * Get clipping data
  */
  _GetClipParam(x0, y0, XSize, YSize, Bpp, BytesPerLine, &Param);
  pContext->Off += Param.DataOff;
  /*
  * Calculate step (+1 or -1)
  */
  Step = _GetStep(&YSize, &y);
  if (!Step) {
    return 1;
  }
  /*
  * Clip Y and move data pointer to the beginning
  */
  if (Step < 0) {
    if (Param.ClipBottom > 0) {
      pContext->Off += BytesPerLine * Param.ClipBottom;
      y -= Param.ClipBottom;
    }
  } else {
    if (Param.ClipTop > 0) {
      pContext->Off += BytesPerLine * Param.ClipTop;
      y += Param.ClipTop;
    }
  }
  YSize -= Param.ClipTop + Param.ClipBottom;
  /*
  * Clip X
  */
  x0       += Param.ClipLeft;
  XSize    -= Param.ClipRight + Param.ClipLeft;
  SkipBytes = BytesPerLine - Param.BytesPerLine;
  do {
    /*
    * Draw line of pixel data
    */
    if (GUI_BMP__ReadData(pContext, Param.BytesPerLine, &pData, 0)) {
      return 1; /* Error */
    }
    LCD_DrawBitmap(x0, y0 + y, XSize, 1, 1, 1, Bpp, 1, pData, LCD__aConvTable);
    /*
    * Skip data
    */
    pContext->Off += SkipBytes;
    y += Step;
  } while (--YSize);
  return 0;
}

/*********************************************************************
*
*       _DrawBitmap_16bpp
*/
static int _DrawBitmap_16bpp(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize) {
  #if USE_NEXT_PIXEL

  LCD_PIXELINDEX Index;
  LCD_API_NEXT_PIXEL * pNextPixel_API;
  tLCDDEV_Color2Index * pfColor2Index;
  unsigned BytesPerLine;
  int x, y, Step, SkipBytes;
  const U8 * pData;
  CLIP_PARAM_BMP Param = {0};

  BytesPerLine = ((16 * XSize + 31) >> 5) << 2;
  /*
  * Get clipping data
  */
  _GetClipParam(x0, y0, XSize, YSize, 16, BytesPerLine, &Param);
  pContext->Off += Param.DataOff;
  /*
  * Calculate step (+1 or -1)
  */
  Step = _GetStep(&YSize, &y);
  if (!Step) {
    return 1;
  }
  /*
  * Clip Y and move data pointer to the beginning
  */
  if (Step < 0) {
    if (Param.ClipBottom > 0) {
      pContext->Off += BytesPerLine * Param.ClipBottom;
      y -= Param.ClipBottom;
    }
  } else {
    if (Param.ClipTop > 0) {
      pContext->Off += BytesPerLine * Param.ClipTop;
      y += Param.ClipTop;
    }
  }
  YSize -= Param.ClipTop + Param.ClipBottom;
  /*
  * Clip X
  */
  x0       += Param.ClipLeft;
  XSize    -= Param.ClipRight + Param.ClipLeft;
  SkipBytes = BytesPerLine - Param.BytesPerLine;
  /*
  * Initialize drawing
  */
  pfColor2Index = LCD_GetpfColor2IndexEx(GUI_Context.SelLayer);
  pNextPixel_API = &_NextPixel_API;
  pNextPixel_API->pfStart(x0, y0 + Param.ClipTop, x0 + XSize - 1, y0 + Param.ClipTop + YSize - 1); /* Initialize drawing routine of the driver */
  do {
    /*
    * Read one line of pixel data
    */
    if (GUI_BMP__ReadData(pContext, Param.BytesPerLine, &pData, 0)) {
      return 1; /* Error */
    }
    for (x = 0; x < XSize; x++) {
      U16 Value;
      GUI_COLOR Color;
      Value = GUI__Read16(&pData);
      Color = LCD_Index2Color_M555(Value);
      Index = pfColor2Index(Color);
      pNextPixel_API->pfSetPixel(Index); /* Send index value to the driver */
    }
    if (y) {
      pNextPixel_API->pfNextLine(); /* Increment line */
    }
    /*
    * Skip data
    */
    pContext->Off += SkipBytes;
    y += Step;
  } while (--YSize);
  pNextPixel_API->pfEnd(); /* Done */
  return 0;

  #else

  unsigned BytesPerLine;
  int x, y, Step;
  const U8 * pData;
  BytesPerLine = ((16 * XSize + 31) >> 5) << 2;
  Step = _GetStep(&YSize, &y);
  if (!Step) {
    return 1;
  }
  for (; (y < YSize) && (y >= 0); y += Step) {
    /* Read one line of pixel data */
    if (GUI_BMP__ReadData(pContext, BytesPerLine, &pData, 0)) {
      return 1; /* Error */
    }
    for (x = 0; x < XSize; x++) {
      U16 Index;
      GUI_COLOR Color;
      Index = GUI__Read16(&pData);
      Color = LCD_Index2Color_M555(Index);
      LCD_SetPixelIndex(x0 + x, y0 + y, LCD_Color2Index(Color));
    }
  }
  return 0;

  #endif
}

/*********************************************************************
*
*       _DrawBitmap_RGB
*/
static int _DrawBitmap_RGB(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize, unsigned BytesPerLine, unsigned BytesPerPixel) {
  #if USE_NEXT_PIXEL

  LCD_PIXELINDEX Index;
  LCD_API_NEXT_PIXEL * pNextPixel_API;
  tLCDDEV_Color2Index * pfColor2Index;
  int x, y, Step, SkipBytes;
  const U8 * pData;
  CLIP_PARAM_BMP Param = {0};

  /*
  * Get clipping data
  */
  _GetClipParam(x0, y0, XSize, YSize, BytesPerPixel * 8, BytesPerLine, &Param);
  pContext->Off += Param.DataOff;
  /*
  * Calculate step (+1 or -1)
  */
  Step = _GetStep(&YSize, &y);
  if (!Step) {
    return 1;
  }
  /*
  * Clip Y and move data pointer to the beginning
  */
  if (Step < 0) {
    if (Param.ClipBottom > 0) {
      pContext->Off += BytesPerLine * Param.ClipBottom;
      y -= Param.ClipBottom;
    }
  } else {
    if (Param.ClipTop > 0) {
      pContext->Off += BytesPerLine * Param.ClipTop;
      y += Param.ClipTop;
    }
  }
  YSize -= Param.ClipTop + Param.ClipBottom;
  /*
  * Clip X
  */
  x0       += Param.ClipLeft;
  XSize    -= Param.ClipRight + Param.ClipLeft;
  SkipBytes = BytesPerLine - Param.BytesPerLine;
  /*
  * Initialize drawing
  */
  pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
  pNextPixel_API = &_NextPixel_API;
  pNextPixel_API->pfStart(x0, y0 + Param.ClipTop, x0 + XSize - 1, y0 + Param.ClipTop + YSize - 1); /* Initialize drawing routine of the driver */
  do {
    /*
    * Read one line of pixel data
    */
    if (GUI_BMP__ReadData(pContext, Param.BytesPerLine, &pData, 0)) {
      return 1; /* Error */
    }
    for (x = 0; x < XSize; x++) {
      const U8 * pColor = pData + x * BytesPerPixel;
      U8 r, g, b;
      U32 Color;
      b = *(pColor);
      g = *(pColor + 1);
      r = *(pColor + 2);
      Color = ((U32)b << 16) | ((unsigned)g << 8) | r;
      Index = pfColor2Index(Color);
      pNextPixel_API->pfSetPixel(Index); /* Send index value to the driver */
    }
    if (y) {
      pNextPixel_API->pfNextLine(); /* Increment line */
    }
    /*
    * Skip data
    */
    pContext->Off += SkipBytes;
    y += Step;
  } while (--YSize);
  pNextPixel_API->pfEnd(); /* Done */
  return 0;

  #else

  int x, y, Step;
  const U8 * pData;
  Step = _GetStep(&YSize, &y);
  if (!Step) {
    return 1;
  }
  for (; (y < YSize) && (y >= 0); y += Step) {
    /* Read one line of pixel data */
    if (GUI_BMP__ReadData(pContext, BytesPerLine, &pData, 0)) {
      return 1; /* Error */
    }
    for (x = 0; x < XSize; x++) {
      const U8 * pColor = pData + x * BytesPerPixel;
      U8 r, g, b;
      b = *(pColor);
      g = *(pColor + 1);
      r = *(pColor + 2);
      LCD_SetPixelIndex(x0 + x, y0 + y, LCD_Color2Index(((U32)b << 16) | ((unsigned)g << 8) | r));
    }
  }
  return 0;

  #endif
}

/*********************************************************************
*
*       _DrawBitmap_24bpp
*/
static int _DrawBitmap_24bpp(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize) {
  unsigned BytesPerLine;
  BytesPerLine = ((24 * XSize + 31) >> 5) << 2;
  return _DrawBitmap_RGB(pContext, x0, y0, XSize, YSize, BytesPerLine, 3);
}

/*********************************************************************
*
*       _DrawBitmap_32bpp
*/
static int _DrawBitmap_32bpp(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize) {
  unsigned BytesPerLine;
  BytesPerLine = XSize << 2;
  return _DrawBitmap_RGB(pContext, x0, y0, XSize, YSize, BytesPerLine, 4);
}

/*********************************************************************
*
*       _Draw
*/
static int _Draw(int x0, int y0, GUI_BMP_CONTEXT * pContext) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  int Ret = 0;
  int Cnt = 0;
  I32 Width, Height;
  U16 BitCount;
  int NumColors;
  int Compression;
  /* Initialize */
  if (GUI_BMP__Init(pContext, &Width, &Height, &BitCount, &NumColors, &Compression)) {
    return 1;
  }
  /* Start output */
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    r.x1 = (r.x0 = x0) + Width - 1;
    r.y1 = (r.y0 = y0) + Height - 1;
    WM_ITERATE_START(&r) {
  #endif
  if (Cnt++) {
    Ret = GUI_BMP__Init(pContext, &Width, &Height, &BitCount, &NumColors, &Compression);
  }
  if (Ret == 0) {
    /* Show bitmap */
    switch (Compression) {
    case BI_RGB:
      switch (BitCount) {
      case 1:
      case 4:
      case 8:
        Ret = _DrawBitmap_Pal(pContext, x0, y0, Width, Height, BitCount, NumColors);
        break;
      case 16:
        Ret = _DrawBitmap_16bpp(pContext, x0, y0, Width, Height);
        break;
      case 24:
        Ret = _DrawBitmap_24bpp(pContext, x0, y0, Width, Height);
        break;
      case 32:
        Ret = _DrawBitmap_32bpp(pContext, x0, y0, Width, Height);
        break;
      default:
        Ret = 1;
      }
      break;
    case BI_RLE4:
    case BI_RLE8:
      Ret = _DrawBitmap_RLE(pContext, x0, y0, Width, Height, NumColors, Compression);
      break;
    }
  }
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
  return Ret;
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_BMP__ReadPalette
*/
int GUI_BMP__ReadPalette(GUI_BMP_CONTEXT * pContext, int NumColors) {
  int i;
  const U8 * pData;
  for (i = 0; i < NumColors; i++) {
    U8 r, g, b;
    /* Read data of one color */
    if (GUI_BMP__ReadData(pContext, 4, &pData, 0)) {
      return 1; /* Error */
    }
    b = *(pData);
    g = *(pData + 1);
    r = *(pData + 2);
    LCD__aConvTable[i] = LCD_Color2Index(((U32)b << 16) | ((unsigned)g << 8) | r);
  }
  return 0;
}

/*********************************************************************
*
*       GUI_BMP__ReadData
*/
int GUI_BMP__ReadData(GUI_BMP_CONTEXT * pContext, int NumBytes, const U8 ** ppData, unsigned StartOfFile) {
  if (StartOfFile) {
    pContext->Off = 0;
  }
  if (pContext->pfGetData(pContext->pParam, ppData, NumBytes, pContext->Off) != NumBytes) {
    return 1;
  }
  pContext->Off += NumBytes;
  return 0;
}

/*********************************************************************
*
*       GUI_BMP__GetData
*
* Parameters:
*   p           - Pointer to application defined data.
*   NumBytesReq - Number of bytes requested.
*   ppData      - Pointer to data pointer. This pointer should be set to
*                 a valid location.
*
* Return value:
*   Number of data bytes available.
*/
int GUI_BMP__GetData(void * p, const U8 ** ppData, unsigned NumBytesReq, U32 Off) {
  GUI_BMP_PARAM * pParam;
  pParam = (GUI_BMP_PARAM *)p;
  *ppData = pParam->pFileData + Off;
  return NumBytesReq;
}

/*********************************************************************
*
*       GUI_BMP__Init
*/
int GUI_BMP__Init(GUI_BMP_CONTEXT * pContext, I32 * pWidth, I32 * pHeight, U16 * pBitCount, int * pNumColors, int * pCompression) {
  U16 Type;
  U32 ClrUsed;
  const U8 * pSrc;
  /* Read BITMAPFILEHEADER and BITMAPINFOHEADER */
  if (GUI_BMP__ReadData(pContext, 54, &pSrc, 1)) {
    return 1; /* Error */
  }
  /* Get values from BITMAPFILEHEADER */
  Type          = GUI__Read16(&pSrc);  /* Get type from BITMAPFILEHEADER */
  pSrc += 12;                          /* Skip rest of BITMAPFILEHEADER */
  /* Get values from BITMAPINFOHEADER */
  pSrc += 4;
  *pWidth       = GUI__Read32(&pSrc);
  *pHeight      = GUI__Read32(&pSrc);
  pSrc += 2;
  *pBitCount    = GUI__Read16(&pSrc);
  *pCompression = GUI__Read32(&pSrc);
  pSrc += 12;
  ClrUsed       = GUI__Read32(&pSrc);
  /* Calculate number of colors */
  switch (*pBitCount) {
    case 0:   return 1; /* biBitCount = 0 (JPEG format) not supported. Please convert image ! */
    case 1:   *pNumColors = 2;   break;
    case 4:   *pNumColors = 16;  break;
    case 8:   *pNumColors = 256; break;
    case 16:
    case 24:
    case 32:  *pNumColors = 0;   break;
    default:
      return 1; /* biBitCount should be 1, 4, 8, 16, 24 or 32 */
  }
  if (*pNumColors && ClrUsed) {
    *pNumColors = ClrUsed;
  }
  /* Check validity of bmp */
  if ((*pNumColors > LCD_MAX_LOG_COLORS) ||
      (Type != 0x4d42) || /* 'BM' */
      ((*pCompression != BI_RGB) && (*pCompression != BI_RLE8) && (*pCompression != BI_RLE4))) {
    return 1;
  }
  return 0;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_BMP_GetXSizeEx
*/
int GUI_BMP_GetXSizeEx(GUI_GET_DATA_FUNC * pfGetData, void * p) {
  GUI_BMP_CONTEXT Context;
  const U8 * pSrc;
  Context.pfGetData = pfGetData;
  Context.pParam    = p;
  if (GUI_BMP__ReadData(&Context, 26, &pSrc, 1)) {
    return 1; /* Error */
  }
  pSrc += 18;
  return GUI__Read32(&pSrc);
}
/*********************************************************************
*
*       GUI_BMP_GetXSize
*/
int GUI_BMP_GetXSize(const void * pBMP) {
  GUI_BMP_PARAM Param = {0};
  Param.pFileData = (const U8 *)pBMP;
  return GUI_BMP_GetXSizeEx(GUI_BMP__GetData, &Param);
}

/*********************************************************************
*
*       GUI_BMP_GetYSizeEx
*/
int GUI_BMP_GetYSizeEx(GUI_GET_DATA_FUNC * pfGetData, void * p) {
  GUI_BMP_CONTEXT Context;
  const U8 * pSrc;
  Context.pfGetData = pfGetData;
  Context.pParam    = p;
  if (GUI_BMP__ReadData(&Context, 26, &pSrc, 1)) {
    return 1; /* Error */
  }
  pSrc += 22;
  return labs((int)GUI__Read32(&pSrc));
}

/*********************************************************************
*
*       GUI_BMP_GetYSize
*/
int GUI_BMP_GetYSize(const void * pBMP) {
  GUI_BMP_PARAM Param = {0};
  Param.pFileData = (const U8 *)pBMP;
  return GUI_BMP_GetYSizeEx(GUI_BMP__GetData, &Param);
}

/*********************************************************************
*
*       GUI_BMP_DrawEx
*/
int  GUI_BMP_DrawEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0) {
  GUI_BMP_CONTEXT Context;
  Context.pfGetData = pfGetData;
  Context.pParam    = p;
  return _Draw(x0, y0, &Context);
}

/*********************************************************************
*
*       GUI_BMP_Draw
*/
int GUI_BMP_Draw(const void * pBMP, int x0, int y0) {
  GUI_BMP_PARAM Param = {0};
  Param.pFileData = (const U8 *)pBMP;
  return GUI_BMP_DrawEx(GUI_BMP__GetData, &Param, x0, y0);
}

/*************************** End of file ****************************/
