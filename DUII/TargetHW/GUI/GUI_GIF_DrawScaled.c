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
File        : GUI_GIF_DrawScaled.c
Purpose     : Implementation of GUI_GIF... functions
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI_GIF_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _ClearUnusedPixelsScaled
*/
static void _ClearUnusedPixelsScaled(int x0, int y0, IMAGE_DESCRIPTOR * pDescriptor, GUI_GIF_IMAGE_INFO * pInfo, int Num, int Denom) {
  LCD_SetColorIndex(LCD_GetBkColorIndex());
  if (pDescriptor->YPos > pInfo->yPos) {
    LCD_FillRect(x0 + pInfo->xPos * (U32)Num / Denom,
                 y0 + pInfo->yPos * (U32)Num / Denom,
                 x0 + (pInfo->xPos + pInfo->xSize) * (U32)Num / Denom - 1,
                 y0 + pDescriptor->YPos * (U32)Num / Denom - 1);
  }
  if (pDescriptor->XPos > pInfo->xPos) {
    LCD_FillRect(x0 + pInfo->xPos * (U32)Num / Denom,
                 y0 + pInfo->yPos * (U32)Num / Denom,
                 x0 + pDescriptor->XPos * (U32)Num / Denom - 1,
                 y0 + (pInfo->yPos + pInfo->ySize) * (U32)Num / Denom - 1);
  }
  if ((pDescriptor->YPos + pDescriptor->YSize) < (pInfo->yPos + pInfo->ySize)) {
    LCD_FillRect(x0 + pInfo->xPos * (U32)Num / Denom,
                 y0 + (pDescriptor->YPos + pDescriptor->YSize) * (U32)Num / Denom - 1,
                 x0 + (pInfo->xPos + pInfo->xSize) * (U32)Num / Denom - 1,
                 y0 + (pInfo->yPos + pInfo->ySize) * (U32)Num / Denom - 1);
  }
  if ((pDescriptor->XPos + pDescriptor->XSize) < (pInfo->xPos + pInfo->xSize)) {
    LCD_FillRect(x0 + (pDescriptor->XPos + pDescriptor->XSize) * (U32)Num / Denom - 1,
                 y0 + pInfo->yPos * (U32)Num / Denom,
                 x0 + (pInfo->xPos + pInfo->xSize) * (U32)Num / Denom - 1,
                 y0 + (pInfo->yPos + pInfo->ySize) * (U32)Num / Denom - 1);
  }
}

/*********************************************************************
*
*       _SkipPixels
*/
static void _SkipPixels(GUI_GIF_CONTEXT * pContext, int SkipCnt) {
  while (SkipCnt--) {
    if (pContext->sp > pContext->aDecompBuffer) {
      --pContext->sp;
    } else {
      GUI_GIF__GetNextByte(pContext);
    }
  }
}

/*********************************************************************
*
*       _DrawLineScaled
*/
static int _DrawLineScaled(GUI_GIF_CONTEXT * pContext, int x0, int y0, int xSize, int Transparency, int Disposal, int NumColors, const LCD_PIXELINDEX * pTrans, int Num, int Denom, unsigned NumLines) {
  int BkColorIndex;
  unsigned Compare, Current;
  int SkipCnt, NumReps, Index;
  Current = Denom / 2;
  Compare = Denom;
  BkColorIndex = LCD_GetBkColorIndex();
  Index = 0;
  /*
   * Iterate over the original pixels
   */
  do {
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        _SkipPixels(pContext, SkipCnt - 1);
        return 0;
      }
    }
    /*
     * Get color
     */
    if (xSize) {
      if (pContext->sp > pContext->aDecompBuffer) {
        Index = *--(pContext->sp);
      } else {
        Index = GUI_GIF__GetNextByte(pContext);
      }
      if (Index == -2) {
        return 0; /* End code */
      }
      if ((Index < 0) || (Index >= NumColors)) {
        return 1; /* If Index out of legal range stop decompressing, error */
      }
    }
    /*
     * Skip the number of pixels that are irrelevant
     */
    _SkipPixels(pContext, SkipCnt);
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
    if (Index >= 0) {
      if ((NumReps == 1) && (NumLines == 1)) {
        if (Index != Transparency) {
          LCD_SetPixelIndex(x0, y0, *(pTrans + Index));
        } else if (Disposal == 2) {
          LCD_SetPixelIndex(x0, y0, BkColorIndex);
        }
      } else {
        if (Index != Transparency) {
          LCD_SetColorIndex(*(pTrans + Index));
          LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
        } else if (Disposal == 2) {
          LCD_SetColorIndex(BkColorIndex);
          LCD_FillRect(x0, y0, x0 + NumReps - 1, y0 + NumLines - 1);
        }
      }
    }
    x0 += NumReps;
  } while (xSize-- > 0);
  return 0;
}

/*********************************************************************
*
*       _SkipLine
*/
static int _SkipLine(GUI_GIF_CONTEXT * pContext, int Width) {
  while (Width--) {
    if (pContext->sp > pContext->aDecompBuffer) {
      --pContext->sp;
    } else {
      GUI_GIF__GetNextByte(pContext);
    }
  }
  return 0;
}

/*********************************************************************
*
*       _DrawNonInterlacedScaled
*/
static int _DrawNonInterlacedScaled(GUI_GIF_CONTEXT * pContext, int x0, int y0, int Width, int Height, int Transparency, int Disposal, int NumColors, const LCD_PIXELINDEX * pTrans, int Num, int Denom) {
  int SkipCnt, NumReps, y;
  unsigned Compare, Current;
  Current = Denom / 2;
  Compare = Denom;
  y = 0;
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
    y += SkipCnt + 1;
    while (--SkipCnt >= 0) {
      /* Skip one line of pixel data */
      if (_SkipLine(pContext, Width)) {
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
    _DrawLineScaled(pContext, x0, y0, Width, Transparency, Disposal, NumColors, pTrans, Num, Denom, NumReps);
    y0 += NumReps;
  } while (y < Height);
  return 0;
}

/*********************************************************************
*
*       _DrawInterlacedLineScaled
*/
static int _DrawInterlacedLineScaled(GUI_GIF_CONTEXT * pContext, int x0, int y0, int Width, int Height, int Transparency, int Disposal, int NumColors, const LCD_PIXELINDEX * pTrans, int Num, int Denom, int yPos) {
  int SkipCnt, NumReps, y, yCompare;
  unsigned Compare, Current;
  Current = Denom / 2;
  Compare = Denom;
  y = yCompare = 0;
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
    y += SkipCnt + 1;
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
    if (yCompare == yPos) {
      if (_DrawLineScaled(pContext, x0, y0, Width, Transparency, Disposal, NumColors, pTrans, Num, Denom, NumReps)) {
        return 1; /* Error */
      }
      return 0; /* Line has been drawn, return */
    }
    if (yCompare > yPos) {
      break;
    }
    y0 += NumReps;
    yCompare += SkipCnt + 1;
  } while (y < Height);
  /* If the line is not drawn, the data pointer needs to be incremented */
  _SkipLine(pContext, Width);
  return 0;
}

/*********************************************************************
*
*       _DrawInterlacedScaled
*/
static int _DrawInterlacedScaled(GUI_GIF_CONTEXT * pContext, int x0, int y0, int Width, int Height, int Transparency, int Disposal, int NumColors, const LCD_PIXELINDEX * pTrans, int Num, int Denom) {
  int YCnt, YPos, Pass;
  for (YCnt = 0, YPos = 0, Pass = 0; YCnt < Height; YCnt++) {
    if (_DrawInterlacedLineScaled(pContext, x0, y0, Width, Height, Transparency, Disposal, NumColors, pTrans, Num, Denom, YPos)) {
      return 1; /* Error */
    }
    YPos += GUI_GIF__aInterlaceOffset[Pass];
    if (YPos >= Height) {
      ++Pass;
      YPos = GUI_GIF__aInterlaceYPos[Pass];
    }
  }
  return 0;
}

/*********************************************************************
*
*       _DrawFromDataBlockScaled
*/
static int _DrawFromDataBlockScaled(GUI_GIF_CONTEXT * pContext, IMAGE_DESCRIPTOR * pDescriptor, int x0, int y0, int Transparency, int Disposal, int Num, int Denom) {
  int Codesize, Interlace;
  int Width, Height, NumColors;
  LCD_LOGPALETTE LogPalette;
  const LCD_PIXELINDEX * pTrans;
  const U8 * pData;
  x0 += pDescriptor->XPos * (U32)Num / Denom;
  y0 += pDescriptor->YPos * (U32)Num / Denom;
  Width     = pDescriptor->XSize;
  Height    = pDescriptor->YSize;
  NumColors = pDescriptor->NumColors;
  /* Get color translation table  */
  LogPalette.NumEntries  = NumColors;
  LogPalette.HasTrans    = 0;
  LogPalette.pPalEntries = pContext->aColorTable;
  if ((pTrans = LCD_GetpPalConvTable((const LCD_LOGPALETTE *)&LogPalette)) == NULL) {
    return 1; /* Error */
  }
  if (GUI_GIF__ReadData(pContext, 1, &pData, 0)) {
    return 1; /* Error */
  }
  if ((!Width) || (!Height)) {
    return 1; /* Error */
  }
  Codesize  = *pData;                    /* Read the LZW codesize */
  GUI_GIF__InitLZW(pContext, Codesize);            /* Initialize the LZW stack with the LZW codesize */
  Interlace = pDescriptor->Flags & 0x40; /* Evaluate if image is interlaced */
  if (!Interlace) {
    return _DrawNonInterlacedScaled(pContext, x0, y0, Width, Height, Transparency, Disposal, NumColors, pTrans, Num, Denom);
  }
  return _DrawInterlacedScaled(pContext, x0, y0, Width, Height, Transparency, Disposal, NumColors, pTrans, Num, Denom);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GIF_DrawSubScaledEx
*/
int  GUI_GIF_DrawSubScaledEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0, int Index, int Num, int Denom) {
  int Result, OldColorIndex;
  GUI_HMEM          hContext;
  GUI_GIF_CONTEXT * pContext;
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
    int Width, Height;
  #endif

  GUI_LOCK();
  hContext = GUI_ALLOC_AllocZero(sizeof(GUI_GIF_CONTEXT));
  if (hContext) {
    pContext = (GUI_GIF_CONTEXT *)GUI_ALLOC_h2p(hContext);
    pContext->pfGetData = pfGetData;
    pContext->pParam    = p;
    #if (GUI_WINSUPPORT)
    {
      const U8 * pData;
      GUI_GIF__ReadData(pContext, 10, &pData, 1);
      pData += 6;
      Width  = GUI__Read16(&pData);
      Height = GUI__Read16(&pData);
    }
    #endif
    OldColorIndex = LCD_GetColorIndex();
    #if (GUI_WINSUPPORT)
      WM_ADDORG(x0,y0);
      r.x1 = (r.x0 = x0) + Width  * (U32)Num / Denom - 1;
      r.y1 = (r.y0 = y0) + Height * (U32)Num / Denom - 1;
      WM_ITERATE_START(&r) {
    #endif
    Result = GUI_GIF__DrawFromFilePointer(pContext, x0, y0, Index, Num, Denom, _DrawFromDataBlockScaled, _ClearUnusedPixelsScaled);
    #if (GUI_WINSUPPORT)
      } WM_ITERATE_END();
    #endif
    LCD_SetColorIndex(OldColorIndex);
    GUI_ALLOC_Free(hContext);
  }
  GUI_UNLOCK();
  return Result;
}

/*********************************************************************
*
*       GUI_GIF_DrawSubScaled
*/
int  GUI_GIF_DrawSubScaled(const void * pGIF, U32 NumBytes, int x0, int y0, int Index, int Num, int Denom) {
  GUI_GIF_PARAM Param;
  Param.pFileData = (const U8 *)pGIF;
  Param.FileSize  = NumBytes;
  return GUI_GIF_DrawSubScaledEx(GUI_GIF__GetData, &Param, x0, y0, Index, Num, Denom);
}

/*************************** End of file ****************************/
