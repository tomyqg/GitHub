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
File        : GUI__DrawBitmap16bpp.c
Purpose     : Implementation of GUI__DrawBitmap16bpp
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__DrawBitmap16bpp
*/
void GUI__DrawBitmap16bpp(int x0, int y0, int xSize, int ySize, const U8 GUI_UNI_PTR * pPixel, const LCD_LOGPALETTE GUI_UNI_PTR * pLogPal, int xMag, int yMag, tLCDDEV_Index2Color * pfIndex2Color) {
  const U16 * pData;
  int x, y, xi, yi, yMax, PrevIndex, Mag, BytesPerLine, UseDrawBitmap = 0;
  LCD_PIXELINDEX Index;
  const U16 GUI_UNI_PTR * p;
  GUI_COLOR OldColor; 

  GUI_USE_PARA(pLogPal);
  OldColor = GUI_GetColor();
  PrevIndex = LCD_GetColorIndex();
  pData = (const U16 *)pPixel;
  Mag = (xMag | yMag);
  BytesPerLine = xSize * 2;
  /* Check if bitmaps color format fits to the current color conversion */
  if (Mag == 1) {
    tLCDDEV_Index2Color * pfIndex2ColorCompare;
    pfIndex2ColorCompare = GUI_GetpfIndex2ColorEx(GUI_Context.SelLayer); /* Get pointer to color conversion routine */
    if (pfIndex2ColorCompare == pfIndex2Color) {                         /* Check if color conversion routine fits  */
      UseDrawBitmap = 1;
    }
  }
  if (UseDrawBitmap) {
    LCD_DrawBitmap(x0, y0, xSize, ySize, 1, 1, 16, BytesPerLine, pPixel, 0);
  } else {
    if (Mag == 1) {
      /* Perform clipping */
      U16 Value;
      const U16 * pSrc;
      U32 Color;
      tLCDDEV_Color2Index * pfColor2Index;
      LCD_API_NEXT_PIXEL * pNextPixel_API;
      int x1, y1, Diff;
      y1 = y0 + ySize - 1;
      x1 = x0 + xSize - 1;
      /* Clip Y */
      if (y0 < GUI_Context.ClipRect.y0) {
        Diff = GUI_Context.ClipRect.y0 -y0;
        y0   = GUI_Context.ClipRect.y0;
        #if GUI_SUPPORT_LARGE_BITMAPS                       /* Required only for 16 bit CPUs if some bitmaps are >64kByte */
          pData += (U32)Diff * (U32)BytesPerLine / 2;
        #else
          pData += (unsigned)Diff * (unsigned)BytesPerLine / 2;
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
      /* Clip right side */
      Diff = x1 - GUI_Context.ClipRect.x1;
      if (Diff > 0) {
        xSize -= Diff;
      }
      /* Clip left side */
      if (x0 < GUI_Context.ClipRect.x0) {
        Diff = GUI_Context.ClipRect.x0-x0;
			  xSize -= Diff;
        pData += Diff; 
        x0 += Diff; 
      }
      if (xSize <=0) {
		    return;
      }
      /*
       * Rectangle filling with index values
       */
      pNextPixel_API = LCD_GetNextPixelAPI();
      pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
      pNextPixel_API->pfStart(x0, y0, x1, y1); /* Initialize drawing routine of the driver */
      while (ySize--) {
        pSrc = (const U16 *)pData;
        for (x = 0; x < xSize; x++) {
          Value = *pSrc++;
          Color = pfIndex2Color(Value);
          Index = pfColor2Index(Color);
          pNextPixel_API->pfSetPixel(Index); /* Send index value to the driver */
        }
        if (ySize) {
          pNextPixel_API->pfNextLine(); /* Increment line */
          pData += BytesPerLine >> 1;
        }
      }
      pNextPixel_API->pfEnd(); /* Done */
    } else {
      /* Magnified output using LCD_FillRect() */
      for (y = y0, yi = 0; yi < ySize; yi++, y += yMag, pPixel += BytesPerLine) {
        yMax = y + yMag - 1;
        /* Draw if within clip area */
        if ((yMax >= GUI_Context.ClipRect.y0) && (y <= GUI_Context.ClipRect.y1)) {
          p = pData;
          for (x = x0, xi = 0; xi < xSize; xi++, x += xMag) {
            Index = *p++;
            if (Index != (LCD_PIXELINDEX)PrevIndex) {
              LCD_SetColor(pfIndex2Color(Index));
              PrevIndex = Index;
            }
            LCD_FillRect(x, y, x + xMag - 1, yMax);
          }
        }
        pData += BytesPerLine / 2;
      }
    }
    GUI_SetColor(OldColor);
  }
}

/*************************** End of file ****************************/
