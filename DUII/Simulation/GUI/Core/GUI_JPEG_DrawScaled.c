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
File        : GUI_JPEG_DrawScaled.c
Purpose     : Implementation of GUI_JPEG... functions
---------------------------END-OF-HEADER------------------------------
*/

#include <stdlib.h>

#include "GUI_Private.h"
#include "GUI_JPEG_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawScanlineScaled
*
* Parameters
*   pContext   - Pointer to decompression context
*   x0, y0     - Drawing position of leftmost pixel
*   xSize      - Number of origilal pixels in color buffer
*   pfGetColor - Function pointer for getting color information
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*   NumLines   - Number of repetitions the line needs to be drawn
*/
static void _DrawScanlineScaled(GUI_JPEG_DCONTEXT * pContext, int x0, int y0, int xSize, GUI_COLOR (* pfGetColor)(const U8 ** ppData, unsigned SkipCnt), int Num, int Denom, unsigned NumLines) {
  const U8 * p;
  unsigned Compare, Current;
  int SkipCnt, NumReps;
  Current = Denom / 2;
  Compare = Denom;
  if (pContext->BufferIndex == 0) {
    p = (U8 *)GUI_LOCK_H(pContext->hScanLine0);
  } else {
    p = (U8 *)GUI_LOCK_H(pContext->hScanLine1);
  }
  /*
   * Iterate over the original pixels
   */
  do {
    GUI_COLOR Color;
    SkipCnt = 0;
    /*
     * Find out how many source pixels can be skipped
     */
    while (Current += Num, Current < Compare) {
      SkipCnt++;
      if (xSize-- == 0) {
        goto _DrawScanlineScaled_Skip;
      }
    }
    /*
     * Skip the number of pixels that are irrelevant
     */
    Color = pfGetColor(&p, SkipCnt + 1);
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
_DrawScanlineScaled_Skip:
  GUI_UNLOCK_H(p);
}

/*********************************************************************
*
*       _DrawScanlinesScaledAtOnce
*
* Parameters
*   pContext   - Pointer to decompression context
*   x0, y0     - Drawing position of leftmost pixel
*   pfGetColor - Function pointer for getting color information
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*/
static int _DrawScanlinesScaledAtOnce(GUI_JPEG_DCONTEXT * pContext, int x0, int y0, GUI_COLOR (* pfGetColor)(const U8 ** ppData, unsigned SkipCnt), int Num, int Denom) {
  unsigned Compare, Current;
  int SkipCnt, NumReps;
  int (* pfFunc)(GUI_JPEG_DCONTEXT * pContext);

  Current = Denom / 2;
  Compare = Denom;
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
    do {
      if (SkipCnt == 0) {
        if (GUI_JPEG__DecodeLine(pContext)) {
          return 1;
        }
      } else {
        if (pContext->IsProgressive) {
          if (GUI_JPEG__DecodeLine(pContext)) {
            return 1;
          }
        } else {
          if (SkipCnt == 1) {
            pfFunc = GUI_JPEG__DecodeLine;
          } else {
            pfFunc = GUI_JPEG__SkipLine;
          }
          if (pfFunc(pContext)) {
            return 1;
          }
        }
      }
      if (pContext->TotalLinesLeft == 0) {
        break;
      }
    } while (--SkipCnt >= 0);
    if (pContext->TotalLinesLeft == 0) {
      break;
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
    _DrawScanlineScaled(pContext, x0, y0, pContext->xSize, pfGetColor, Num, Denom, NumReps);
    y0 += NumReps;
  } while (pContext->TotalLinesLeft);
  return 0;
}

/*********************************************************************
*
*       _DrawScanlinesScaledBanding
*
* Parameters
*   pContext   - Pointer to decompression context
*   x0, y0     - Drawing position of leftmost pixel
*   pfGetColor - Function pointer for getting color information
*   Num        - Numerator for scaling
*   Denom      - Denominator for scaling
*/
static int _DrawScanlinesScaledBanding(GUI_HMEM hContext, int x0, int y0, GUI_COLOR (* pfGetColor)(const U8 ** ppData, unsigned SkipCnt), int Num, int Denom) {
  GUI_JPEG_DCONTEXT * pContext;
  unsigned Band;
  int r = 0;
  int yPos;

  /*
   * Allocate coefficient buffer
   */
  if (GUI_JPEG__AllocBandingCoeffBuffer(hContext) != 0) {
    return 1;
  }
  pContext = JPEG_LOCK_H(hContext);
  for (Band = 0; Band < pContext->NumBands; Band++) {
    unsigned Compare, Current;
    int SkipCnt, NumReps;
    int BlockY;
    Current = Denom / 2;
    Compare = Denom;
    /*
     * Start reading and process markers until SOF marker
     */
    if (Band) {
      GUI_JPEG__ReadUntilSOF(hContext);
    }
    /*
     * Decode one band of data
     */
    GUI_JPEG__DecodeProgressiveBanding(pContext);
    /*
     * Iterate over the original pixels
     */
    yPos = y0;
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
        if (GUI_JPEG__DecodeLine(pContext)) {
          return 1;
        }
        if (pContext->TotalLinesLeft == 0) {
          break;
        }
      }
      if (pContext->TotalLinesLeft == 0) {
        break;
      }
      /*
       * Find out how many times this line needs to be repeated
       */
      NumReps = 1;
      while (Compare += Denom, Current >= Compare) {
        NumReps++;
      }
      BlockY = Current >> 3;
      if ((BlockY >= pContext->FirstBlockOfBand) && (BlockY <= (pContext->FirstBlockOfBand + pContext->NumBlocksPerBand - 1))) {
        /*
         * Draw the line
         */
        _DrawScanlineScaled(pContext, x0, y0, pContext->xSize, pfGetColor, Num, Denom, NumReps);
      }
      y0 += NumReps;
    } while (pContext->TotalLinesLeft);
    y0 = yPos;
    /*
     * Set parameters for next band
     */
    GUI_JPEG__SetNextBand(pContext);
  }
  GUI_UNLOCK_H(pContext);
  return r;
}

/*********************************************************************
*
*       _Draw
*/
static int _Draw(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0, int Num, int Denom) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_HMEM            hContext;
  GUI_JPEG_DCONTEXT * pContext;
  GUI_JPEG_INFO Info;
  GUI_COLOR (* pfGetColor)(const U8 ** ppData, unsigned SkipCnt);
  int r = 1;
  int OldIndex;

  hContext = GUI_ALLOC_AllocZero((GUI_ALLOC_DATATYPE)sizeof(GUI_JPEG_DCONTEXT));
  if (hContext) {
    pContext = JPEG_LOCK_H(hContext);
    pContext->pParam = p;
    pContext->pfGetData = pfGetData;
    /* Remember old foreground color */
    OldIndex = LCD_GetColorIndex();
    GUI_UNLOCK_H(pContext);
    /* Get size of image */
    if (GUI_JPEG__GetInfoEx(hContext, &Info)) {
      return 1;
    }
    /* Iterate over all windows */
    #if (GUI_WINSUPPORT)
      WM_ADDORG(x0,y0);
      Rect.x1 = (Rect.x0 = x0) + (U32)Info.XSize * (U32)Num / Denom - 1;
      Rect.y1 = (Rect.y0 = y0) + (U32)Info.YSize * (U32)Num / Denom - 1;
      WM_ITERATE_START(&Rect) {
    #endif
      /* Initialize drawing and allocate memory */
      r = GUI_JPEG__InitDraw(hContext);
      pContext = JPEG_LOCK_H(hContext);
      if (pContext->ScanType == GRAYSCALE) {
        pfGetColor = GUI_JPEG__GetColorGray;
      } else {
        pfGetColor = GUI_JPEG__GetColorRGB;
      }
      /* Draw the lines */
      if (r == 0) {
        if (pContext->BandingRequired == 0) {
          r = _DrawScanlinesScaledAtOnce(pContext, x0, y0, pfGetColor, Num, Denom);
        } else {
          GUI_UNLOCK_H(pContext);
          r = _DrawScanlinesScaledBanding(hContext, x0, y0, pfGetColor, Num, Denom);
          pContext = JPEG_LOCK_H(hContext);
        }
      }
      /* Free allocated memory */
      GUI_JPEG__Free(pContext);
      GUI_UNLOCK_H(pContext);
    #if (GUI_WINSUPPORT)
      } WM_ITERATE_END();
    #endif
    /* Restore foreground color */
    LCD_SetColor(LCD_Index2Color(OldIndex));
    GUI_ALLOC_Free(hContext);
  }
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_JPEG_DrawScaledEx
*/
int  GUI_JPEG_DrawScaledEx(GUI_GET_DATA_FUNC * pfGetData, void * p, int x0, int y0, int Num, int Denom) {
  int r;
  GUI_LOCK();
  r = _Draw(pfGetData, p, x0, y0, Num, Denom);
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       GUI_JPEG_DrawScaled
*/
int  GUI_JPEG_DrawScaled(const void * pFileData, int DataSize, int x0, int y0, int Num, int Denom) {
  int r;
  GUI_JPEG_PARAM Param;
  GUI_LOCK();
  Param.FileSize  = DataSize;
  Param.pFileData = (const U8 *)pFileData;
  r = GUI_JPEG_DrawScaledEx(GUI_JPEG__GetData, &Param, x0, y0, Num, Denom);
  GUI_UNLOCK();
  return r;
}

/*************************** End of file ****************************/
