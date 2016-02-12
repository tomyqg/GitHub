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
File        : GUI_DrawBitmap_24.c
Purpose     : Drawing routine for drawing true color bitmaps
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBitmap
*
* Purpose:
*   The function draws true color 24bpp bitmaps.
*/
static void _DrawBitmap(int x0,int y0,int xSize, int ySize, const U8 GUI_UNI_PTR * pPixel, const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, int xMag, int yMag) {
  U8 r, g, b;
  const U8 GUI_UNI_PTR * pSrc;
  U32 Color;
  LCD_PIXELINDEX Index;
  int BytesPerLine;
  tLCDDEV_Color2Index * pfColor2Index;
  LCD_API_NEXT_PIXEL * pNextPixel_API;

  GUI_USE_PARA(pLogPal);
  /* 
   * Get function pointer for color conversion 
   */
  pfColor2Index = LCD_GetpfColor2IndexEx(GUI_Context.SelLayer);
  BytesPerLine = xSize * 3;
  if ((xMag | yMag) == 1) {
    int x, x1, y1, Diff;
    y1 = y0 + ySize - 1;
    x1 = x0 + xSize - 1;
    /*
     * Clip Y
     */
    if (y0 < GUI_Context.ClipRect.y0) {
      Diff = GUI_Context.ClipRect.y0 - y0;
      y0   = GUI_Context.ClipRect.y0;
      #if GUI_SUPPORT_LARGE_BITMAPS                       /* Required only for 16 bit CPUs if some bitmaps are >64kByte */
        pPixel += (U32)Diff * (U32)BytesPerLine;
      #else
        pPixel += (unsigned)Diff * (unsigned)BytesPerLine;
      #endif
      ySize -= Diff;
    }
    Diff = y1 - GUI_Context.ClipRect.y1;
    if (Diff > 0) {
      ySize -= Diff;
    }
    if (ySize <=0) {
		  return;
    }
    /* 
     * Clip right side 
     */
    Diff = x1 - GUI_Context.ClipRect.x1;
    if (Diff > 0) {
      xSize -= Diff;
    }
    /*
     * Clip left side 
     */
    if (x0 < GUI_Context.ClipRect.x0) {
      Diff = GUI_Context.ClipRect.x0 - x0;
			xSize -= Diff;
      pPixel += Diff * 3; 
      x0 += Diff; 
    }
    if (xSize <=0) {
		  return;
    }
    /*
     * Driver is not able to manage the complete bitmap
     */
    pNextPixel_API = LCD_GetNextPixelAPI();
    /*
     * Rectangle filling with index values
     */
    pNextPixel_API->pfStart(x0, y0, x1, y1); /* Initialize drawing routine of the driver */
    while (ySize--) {
      pSrc = pPixel;
      for (x = 0; x < xSize; x++) {
        /*
         * Get RGB and calculate index value
         */
        r = *(pSrc + 0);
        g = *(pSrc + 1);
        b = *(pSrc + 2);
        pSrc += 3;
        Color = ((U32)b << 16) | ((U16)g << 8) | r;
        Index = pfColor2Index(Color);
        pNextPixel_API->pfSetPixel(Index); /* Send index value to the driver */
      }
      if (ySize) {
        pNextPixel_API->pfNextLine(); /* Increment line */
        pPixel += BytesPerLine;
      }
    }
    pNextPixel_API->pfEnd(); /* Done */
  } else {
    /*
     * Magnified output
     */
    U32 PrevColor;
    int x, y, yi;
    PrevColor = GUI_INVALID_COLOR;
    for (y = y0, yi = 0; yi < ySize; yi++, y += yMag, pPixel += BytesPerLine) {
      int yMax;
      yMax = y + yMag - 1;
      pSrc = pPixel;
      /* Draw if within clip area */
      if ((yMax >= GUI_Context.ClipRect.y0) && (y <= GUI_Context.ClipRect.y1)) {
        int xi;
        for (x = x0, xi = 0; xi < xSize; xi++, x += xMag) {
          /*
           * Get RGB and calculate index value
           */
          r = *(pSrc + 0);
          g = *(pSrc + 1);
          b = *(pSrc + 2);
          pSrc += 3;
          Color = ((U32)b << 16) | ((U16)g << 8) | r;
          if (Color != PrevColor) {
            LCD_SetColorIndex(pfColor2Index(Color));
            PrevColor = Color;
          }
          LCD_FillRect(x, y, x + xMag - 1, yMax);
        }
      }
    }
  }
}

const GUI_BITMAP_METHODS GUI_BitmapMethods24 = {
  _DrawBitmap,
  LCD_Index2Color_888
};

/*************************** End of file ****************************/
