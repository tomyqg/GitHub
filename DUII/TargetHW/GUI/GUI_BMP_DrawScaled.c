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
File        : GUI_BMP_DrawScaled.c
Purpose     : Implementation of GUI_BMP... functions
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_BMP_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define STATE_READY 0 /* Ready */
#define STATE_ABS8  1 /* Remaining absolute pixels (8bpp) */
#define STATE_RLE8  2 /* Remaining compressed pixels (RLE8) */
#define STATE_ABS4  3 /* Remaining absolute pixels (4bpp) */
#define STATE_RLE4  4 /* Remaining compressed pixels (RLE4) */
#define STATE_LBRK  5 /* Line break */
#define STATE_DELTA 6 /* Move cursor */

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int ColorIndex;
  U32 RemPixels;
  int State;
  int xSize;
  int xPos;
  int Compression;
  U8 Cmd;
  U8 Data;
  U8 NumNibble;
  U8 DummyRead;
} CONTEXT_DECOMP_BMP;

typedef void tDrawLineScaled(const U8 * pData, int x0, int y0, int xSize, int Num, int Denom, unsigned NumLines, int BytesPerPixel, int Bpp);

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetStep
*/
static int _GetStep(int * pYSize, int * pY, int Num, int Denom) {
  *pYSize = (I32)(*pYSize) * Num / Denom;
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
*       _GetColor
*/
static U32 _GetColor(const U8 ** ppData, unsigned *pPixelPos, int SkipCnt, int Bpp) {
  U32 Color;
  unsigned Index, Shift, Odd;
  U8 Mask;
  const U8 * pData;
  pData = *ppData;
  switch (Bpp) {
  case 1:
    Shift = (*pPixelPos) & 7;
    Mask  = 0x80 >> Shift;
    Index = (*pData & Mask) >> (7 - Shift);
    if ((Shift + SkipCnt + 1) > 7) {
      pData += (Shift + SkipCnt + 1) >> 3;
    }
    break;
  case 4:
    Odd   = (*pPixelPos) & 1;
    Shift = Odd << 2;
    Mask  = 0xF0 >> Shift;
    Index = (*pData & Mask) >> (4 - Shift);
    if ((Odd + SkipCnt + 1) > 1) {
      pData += (Odd + SkipCnt + 1) >> 1;
    }
    break;
  case 8:
    Index = *pData;
    pData += SkipCnt + 1;
    break;
  default:
    return 0;
  }
  Color       = LCD_Index2Color(LCD__aConvTable[Index]);
  *ppData     = pData;
  *pPixelPos += SkipCnt + 1;
  return Color;
}

/*********************************************************************
*
*       _DrawLineScaled_Pal
*
* Parameters
*   pData      - Pointer to buffer with color information
*   x0, y0     - Drawing position of leftmost pixel
*   xSize      - Number of origilal pixels in color buffer
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*   NumLines   - Number of repetitions the line needs to be drawn
*   Bpp        - Number of bits per pixel (1, 4 or 8)
*/
static void _DrawLineScaled_Pal(const U8 * pData, int x0, int y0, int xSize, int Num, int Denom, unsigned NumLines, int BytesPerPixel, int Bpp) {
  unsigned Compare, Current, PixelPos;
  int SkipCnt, NumReps;
  GUI_USE_PARA(BytesPerPixel);
  PixelPos = 0;
  /*
   * Iterate over the original pixels
   */
  Current = Denom / 2;
  Compare = Denom;
  do {
    GUI_COLOR Color;
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        return;
      }
    }
    /*
     * Get color from data pointer and skip the number of pixels that are irrelevant
     */
    Color = _GetColor(&pData, &PixelPos, SkipCnt, Bpp);
    LCD_SetColor(Color);
    /*
     * Find out how many times this pixel needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the pixel
     */
    if ((NumReps == 1) && (NumLines == 1)) {
      LCD_DrawPixel(x0, y0);
    } else {
      LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
    }
    x0 += NumReps;
  } while (xSize--);
}

/*********************************************************************
*
*       _DrawLineScaled_RGB
*
* Parameters
*   pData      - Pointer to buffer with color information
*   x0, y0     - Drawing position of leftmost pixel
*   xSize      - Number of origilal pixels in color buffer
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*   NumLines   - Number of repetitions the line needs to be drawn
*/
static void _DrawLineScaled_RGB(const U8 * pData, int x0, int y0, int xSize, int Num, int Denom, unsigned NumLines, int BytesPerPixel, int Bpp) {
  unsigned Compare, Current;
  int SkipCnt, NumReps;
  GUI_USE_PARA(Bpp);
  /*
   * Iterate over the original pixels
   */
  Current = Denom / 2;
  Compare = Denom;
  /* Adjust data pointer to first pixel */
  do {
    U8 r, g, b;
    GUI_COLOR Color;
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        return;
      }
    }
    /*
     * Get color from data pointer
     */
    b = *(pData);
    g = *(pData + 1);
    r = *(pData + 2);
    Color = ((U32)b << 16) | (g << 8) | r;
    /*
     * Skip the number of pixels that are irrelevant
     */
    pData += (SkipCnt + 1) * BytesPerPixel;
    /*
     * Find out how many times this pixel needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the pixel
     */
    LCD_SetColor(Color);
    if ((NumReps == 1) && (NumLines == 1)) {
      LCD_DrawPixel(x0, y0);
    } else {
      LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
    }
    x0 += NumReps;
  } while (xSize--);
}

/*********************************************************************
*
*       _DrawLineScaled_16bpp
*
* Parameters
*   pData      - Pointer to buffer with color information
*   x0, y0     - Drawing position of leftmost pixel
*   xSize      - Number of origilal pixels in color buffer
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*   NumLines   - Number of repetitions the line needs to be drawn
*/
static void _DrawLineScaled_16bpp(const U8 * pData, int x0, int y0, int xSize, int Num, int Denom, unsigned NumLines, int BytesPerPixel, int Bpp) {
  unsigned Compare, Current;
  int SkipCnt, NumReps;
  GUI_USE_PARA(Bpp);
  /*
   * Iterate over the original pixels
   */
  Current = Denom / 2;
  Compare = Denom;
  /* Adjust data pointer to first pixel */
  do {
    U16 Index;
    GUI_COLOR Color;
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        return;
      }
    }
    /*
     * Get color from data pointer
     */
    Index = GUI__Read16(&pData);
    Color = LCD_Index2Color_M555(Index);
    /*
     * Skip the number of pixels that are irrelevant
     */
    pData += (SkipCnt + 0) * BytesPerPixel;
    /*
     * Find out how many times this pixel needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the pixel
     */
    LCD_SetColor(Color);
    if ((NumReps == 1) && (NumLines == 1)) {
      LCD_DrawPixel(x0, y0);
    } else {
      LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
    }
    x0 += NumReps;
  } while (xSize--);
}

/*********************************************************************
*
*       _DrawBitmapScaled
*
* Parameters
*   pContext   - Pointer to GUI_BMP_CONTEXT structure
*   x0, y0     - Drawing position of leftmost pixel
*   XSize      - Number of original width in color buffer
*   YSize      - Number of original height in color buffer
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*/
static int _DrawBitmapScaled(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize, int Num, int Denom, int BytesPerPixel, int Bpp, tDrawLineScaled * pfDrawLineScaled) {
  int y, BytesPerLine, Step, SkipCnt, NumReps;
  const U8 * pData;
  unsigned Compare, Current;
  Current = Denom / 2;
  Compare = Denom;
  Step = _GetStep(&YSize, &y, Num, Denom);
  if (!Step) {
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
  case 16:
  case 24:
  case 32:
    BytesPerLine = (((BytesPerPixel << 3) * XSize + 31) >> 5) << 2;
    break;
  }
  /*
   * Iterate over the original pixels
   */
  do {
    SkipCnt = 0;
    /*
     * Find out how many source lines can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
    }
    /*
     * Skip the number of lines that are irrelevant
     */
    while (SkipCnt-- >= 0) {
      /* Read one line of pixel data */
      if (GUI_BMP__ReadData(pContext, BytesPerLine, &pData, 0)) {
        return 1; /* Error */
      }
    }
    /*
     * Find out how many times this line needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the line
     */
    pfDrawLineScaled(pData, x0, (Step < 0) ? y0 + y - NumReps + 1 : y0 + y, XSize, Num, Denom, NumReps, BytesPerPixel, Bpp);
    y += Step * NumReps;
  } while ((y < YSize) && (y >= 0));
  return 0;
}

/*********************************************************************
*
*       _GetColorIndex_RLE
*/
static int _GetColorIndex_RLE(GUI_BMP_CONTEXT * pContext, int SkipCnt, CONTEXT_DECOMP_BMP * pContextBmp) {
  const U8 * pData;
  do {
    if (pContextBmp->State == STATE_READY) {
      int DoLoop;
      DoLoop = 1;
      do {
        if (GUI_BMP__ReadData(pContext, 2, &pData, 0)) {
          return 1;
        }
        pContextBmp->Cmd  = *pData++;
        pContextBmp->Data = *pData;
        if (pContextBmp->Cmd == 0) {
          switch (pContextBmp->Data) {
          case 0:   /* End of line */
            if (pContextBmp->xPos < pContextBmp->xSize) {
              /* Premature break */
              DoLoop = 0;
              pContextBmp->State = STATE_LBRK;
            } else {
              pContextBmp->xPos = 0;
            }
            break;
          case 1:   /* End of image */
            DoLoop = 0;
            if (pContextBmp->xPos < pContextBmp->xSize) {
              pContextBmp->State = STATE_LBRK;
            }
            break;
          case 2:   /* Delta */
            if (GUI_BMP__ReadData(pContext, 2, &pData, 0)) {
              return 1;
            }
            pContextBmp->RemPixels = (*(pData + 1) * pContextBmp->xSize) + *pData;
            pContextBmp->State = STATE_DELTA;
            if (pContextBmp->xPos == pContextBmp->xSize) {
              pContextBmp->xPos = 0;
            }
            pContextBmp->xPos += *pData;
            DoLoop = 0;
            break;
          default:  /* Absolute mode */
            switch (pContextBmp->Compression) {
            case BI_RLE4:
              pContextBmp->State     = STATE_ABS4;
              pContextBmp->RemPixels = pContextBmp->Data;
              pContextBmp->DummyRead = (((pContextBmp->Data + 1) >> 1) & 1);
              pContextBmp->NumNibble = 0;
              DoLoop = 0;
              break;
            case BI_RLE8:
              pContextBmp->State     = STATE_ABS8;
              pContextBmp->RemPixels = pContextBmp->Data;
              DoLoop = 0;
              break;
            }
            break;
          }
        } else {
          switch (pContextBmp->Compression) {
          case BI_RLE4:
            pContextBmp->State      = STATE_RLE4;
            pContextBmp->RemPixels  = pContextBmp->Cmd;
            pContextBmp->NumNibble  = 0;
            DoLoop = 0;
            break;
          case BI_RLE8:
            pContextBmp->ColorIndex = LCD__aConvTable[pContextBmp->Data];
            pContextBmp->State      = STATE_RLE8;
            pContextBmp->RemPixels  = pContextBmp->Cmd;
            DoLoop = 0;
            break;
          }
        }
      } while (DoLoop);
    }
    switch (pContextBmp->State) {
    /* Remaining absolute pixels (8bpp) */
    case STATE_ABS8:
      if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
        return 1;
      }
      pContextBmp->ColorIndex = LCD__aConvTable[*pData];
      if (--pContextBmp->RemPixels == 0) {
        pContextBmp->State = STATE_READY;
        if (pContextBmp->Data & 1) {
          if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
            return 1;
          }
        }
      }
      pContextBmp->xPos++;
      break;
    /* Remaining compressed pixels (RLE8) */
    case STATE_RLE8:
      if (--pContextBmp->RemPixels == 0) {
        pContextBmp->State = STATE_READY;
      }
      pContextBmp->xPos++;
      break;

    /* Remaining absolute pixels (4bpp) */
    case STATE_ABS4:
      if (pContextBmp->NumNibble == 0) {
        if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
          return 1;
        }
        pContextBmp->Data       = *pData;
        pContextBmp->ColorIndex = LCD__aConvTable[pContextBmp->Data >> 4];
      } else {
        pContextBmp->ColorIndex = LCD__aConvTable[pContextBmp->Data & 15];
      }
      pContextBmp->NumNibble ^= 1;
      pContextBmp->xPos++;
      if (--pContextBmp->RemPixels == 0) {
        pContextBmp->State = STATE_READY;
        /* Make sure we are on an even 16bit-adr */
        if (pContextBmp->DummyRead) {
          if (GUI_BMP__ReadData(pContext, 1, &pData, 0)) {
            return 1;
          }
        }
      }
      break;
    /* Remaining compressed pixels (RLE4) */
    case STATE_RLE4:
      if (pContextBmp->NumNibble == 0) {
        pContextBmp->ColorIndex = LCD__aConvTable[pContextBmp->Data  >> 4];
      } else {
        pContextBmp->ColorIndex = LCD__aConvTable[pContextBmp->Data  & 15];
      }
      pContextBmp->NumNibble ^= 1;
      pContextBmp->xPos++;
      if (--pContextBmp->RemPixels == 0) {
        pContextBmp->State = STATE_READY;
      }
      break;

    /* Premature line break */
    case STATE_LBRK:
      pContextBmp->ColorIndex = LCD__aConvTable[0];
      if (++pContextBmp->xPos == pContextBmp->xSize) {
        pContextBmp->State = STATE_READY;
      }
    /* Delta mode */
    case STATE_DELTA:
      pContextBmp->ColorIndex = LCD__aConvTable[0];
      if (--pContextBmp->RemPixels == 0) {
        pContextBmp->State = STATE_READY;
      }
      break;
    }
  } while (SkipCnt--);
  return pContextBmp->ColorIndex;
}

/*********************************************************************
*
*       _SkipLine_RLE
*
* Return value:
*   Number of skipped lines
*/
static int _SkipLine_RLE(GUI_BMP_CONTEXT * pContext, CONTEXT_DECOMP_BMP * pContextBmp) {
  int i;
  for (i = 0; i < pContextBmp->xSize; i++) {
    _GetColorIndex_RLE(pContext, 0, pContextBmp);
  }
  return 1;
}

/*********************************************************************
*
*       _DrawLineScaled_RLE
*
* Parameters
*   pData      - Pointer to buffer with color information
*   x0, y0     - Drawing position of leftmost pixel
*   xSize      - Number of origilal pixels in color buffer
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*   NumLines   - Number of repetitions the line needs to be drawn
*   Compression- Compression method (BI_RLE4 or BI_RLE8)
*
* Return value:
*   0  = ok
*   >0 = Number of lines skipped
*   <0 = Error
*/
static int _DrawLineScaled_RLE(GUI_BMP_CONTEXT * pContext, int x0, int y0, int xSize, int Num, int Denom, unsigned NumLines, CONTEXT_DECOMP_BMP * pContextBmp) {
  unsigned Compare, Current;
  int SkipCnt, NumReps;
  /*
   * Iterate over the original pixels
   */
  Current = Denom / 2;
  Compare = Denom;
  do {
    int ColorIndex;
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        return 0;
      }
    }
    /*
     * Get color from data pointer and skip the number of pixels that are irrelevant
     */
    ColorIndex = _GetColorIndex_RLE(pContext, SkipCnt, pContextBmp);
    LCD__SetColorIndex(ColorIndex);
    /*
     * Find out how many times this pixel needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the pixel
     */
    if ((NumReps == 1) && (NumLines == 1)) {
      LCD_DrawPixel(x0, y0);
    } else {
      LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
    }
    x0 += NumReps;
  } while (--xSize);
  return 0;
}

/*********************************************************************
*
*       _DrawBitmapScaled_RLE
*/
static int _DrawBitmapScaled_RLE(GUI_BMP_CONTEXT * pContext, int x0, int y0, int XSize, int YSize, int Compression, int Num, int Denom) {
  CONTEXT_DECOMP_BMP ContextBmp = {0};
  int y, Step, SkipCnt, NumReps;
  unsigned Compare, Current;
  ContextBmp.xSize = XSize;
  ContextBmp.Compression = Compression;
  Current = Denom / 2;
  Compare = Denom;
  Step = _GetStep(&YSize, &y, Num, Denom);
  if (!Step) {
    return 1;
  }
  /*
   * Iterate over the original pixels
   */
  do {
    SkipCnt = 0;
    /*
     * Find out how many source lines can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
    }
    /*
     * Skip the number of lines that are irrelevant
     */
    while (SkipCnt-- > 0) {
      /* Read one line of pixel data */
      if (_SkipLine_RLE(pContext, &ContextBmp) < 0) {
        return 1; /* Error */
      }
    }
    /*
     * Find out how many times this line needs to be repeated
     */
    NumReps = 1;
    while (Compare += Denom, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the line
     */
    _DrawLineScaled_RLE(pContext, x0, (Step < 0) ? y0 + y - NumReps + 1 : y0 + y, XSize, Num, Denom, NumReps, &ContextBmp);
    y += Step * NumReps;
  } while ((y < YSize) && (y >= 0));
  return 0;
}

/*********************************************************************
*
*       _Draw
*/
static int _Draw(int x0, int y0, GUI_BMP_CONTEXT * pContext, int Num, int Denom) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  int Ret = 0;
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
    r.x1 = (r.x0 = x0) + Width  * (U32)Num / Denom - 1;
    r.y1 = (r.y0 = y0) + Height * (U32)Num / Denom - 1;
    WM_ITERATE_START(&r) {
  #endif
  /* Show bitmap */
  switch (Compression) {
  case BI_RGB:
    switch (BitCount) {
    case 1:
    case 4:
    case 8:
      GUI_BMP__ReadPalette(pContext, NumColors);
      Ret = _DrawBitmapScaled(pContext, x0, y0, Width, Height, Num, Denom, 1, BitCount, _DrawLineScaled_Pal);
      break;
    case 16:
      Ret = _DrawBitmapScaled(pContext, x0, y0, Width, Height, Num, Denom, 2, BitCount, _DrawLineScaled_16bpp);
      break;
    case 24:
      Ret = _DrawBitmapScaled(pContext, x0, y0, Width, Height, Num, Denom, 3, BitCount, _DrawLineScaled_RGB);
      break;
    case 32:
      Ret = _DrawBitmapScaled(pContext, x0, y0, Width, Height, Num, Denom, 4, BitCount, _DrawLineScaled_RGB);
      break;
    }
    break;
  case BI_RLE4:
  case BI_RLE8:
    GUI_BMP__ReadPalette(pContext, NumColors);
    Ret = _DrawBitmapScaled_RLE(pContext, x0, y0, Width, Height, Compression, Num, Denom);
    break;
  }
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
  return Ret;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_BMP_DrawScaledEx
*/
int  GUI_BMP_DrawScaledEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0, int Num, int Denom) {
  GUI_BMP_CONTEXT Context;
  int r;
  int ColorIndex;
  ColorIndex = LCD__GetColorIndex();
  Context.pParam = p;
  Context.pfGetData = pfGetData;
  r = _Draw(x0, y0, &Context, Num, Denom);
  LCD__SetColorIndex(ColorIndex);
  return r;
}

/*********************************************************************
*
*       GUI_BMP_DrawScaled
*/
int  GUI_BMP_DrawScaled(const void * pFileData, int x0, int y0, int Num, int Denom) {
  GUI_BMP_PARAM Param;
  Param.pFileData = (const U8 *)pFileData;
  return GUI_BMP_DrawScaledEx(GUI_BMP__GetData, &Param, x0, y0, Num, Denom);
}

/*************************** End of file ****************************/
