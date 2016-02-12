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
File        : LCD_DrawBitmap_888.c
Purpose     : Drawing routine for drawing a 16bpp bitmap
---------------------------END-OF-HEADER------------------------------
*/

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
*/
static void _DrawBitmap(int x0,int y0,int xSize, int ySize, const U8 GUI_UNI_PTR * pPixel, int xMag, int yMag, int Swap) {
  const U32 * pSrc;
  U32 Color;
  LCD_PIXELINDEX Index;
  int BytesPerLine;
  tLCDDEV_Color2Index * pfColor2Index;
  /* 
   * Get function pointer for color conversion 
   */
  pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
  BytesPerLine = xSize * 4;
  if ((xMag | yMag) == 1) {
    int x1, y1, Diff;
    int x;//, y;
    LCD_API_NEXT_PIXEL * pNextPixel_API;
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
      pPixel += Diff * 4; 
      x0 += Diff; 
    }
    if (xSize <=0) {
		  return;
    }
    /*
     * Rectangle filling with index values
     */
    pNextPixel_API = LCD_GetNextPixelAPI();
    pNextPixel_API->pfStart(x0, y0, x1, y1); /* Initialize drawing routine of the driver */
    while (ySize--) {
      pSrc = (const U32 *)pPixel;
      for (x = 0; x < xSize; x++) {
        /*
         * Get RGB and calculate index value
         */
        Color = *pSrc++;
        if (Swap) {
          Color = (Color & 0x00ff00) | ((Color & 0xff0000) >> 16) | ((Color & 0x0000ff) << 16);
        }
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
      pSrc = (const U32 *)pPixel;
      /* Draw if within clip area */
      if ((yMax >= GUI_Context.ClipRect.y0) && (y <= GUI_Context.ClipRect.y1)) {
        int xi;
        for (x = x0, xi = 0; xi < xSize; xi++, x += xMag) {
          /*
           * Get RGB and calculate index value
           */
          Color = *pSrc++;
          if (Swap) {
            Color = (Color & 0x00ff00) | ((Color & 0xff0000) >> 16) | ((Color & 0x0000ff) << 16);
          }
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

/*********************************************************************
*
*       _DrawBitmap_888
*/
static void _DrawBitmap_888(int x0, int y0, int xSize, int ySize, const U8 GUI_UNI_PTR * pPixel, const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, int xMag, int yMag) {
  GUI_USE_PARA(pLogPal);
  _DrawBitmap(x0, y0, xSize, ySize, pPixel, xMag, yMag, 0);
}

/*********************************************************************
*
*       _DrawBitmap_M888
*/
static void _DrawBitmap_M888(int x0, int y0, int xSize, int ySize, const U8 GUI_UNI_PTR * pPixel, const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, int xMag, int yMag) {
  GUI_USE_PARA(pLogPal);
  _DrawBitmap(x0, y0, xSize, ySize, pPixel, xMag, yMag, 1);
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_BitmapMethods888
*/
const GUI_BITMAP_METHODS GUI_BitmapMethods888 = {
  _DrawBitmap_888,
  LCD_Index2Color_888
};

/*********************************************************************
*
*       GUI_BitmapMethodsM888
*/
const GUI_BITMAP_METHODS GUI_BitmapMethodsM888 = {
  _DrawBitmap_M888,
  LCD_Index2Color_M888
};

/*************************** End of file ****************************/
