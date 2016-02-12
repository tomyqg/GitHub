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
File        : GUIDEV_Perspective.c
Purpose     : Implementation of perspectively output
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include <math.h>

#include "GUI_Private.h"

#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       _XY2PTR
*/
static U32 * _XY2PTR(GUI_MEMDEV * pDev, int x, int y) {
  U8 * pData = (U8 *)(pDev + 1);
  #if GUI_DEBUG_LEVEL >= GUI_DEBUG_LEVEL_CHECK_ALL
    if ((x >= pDev->x0 + pDev->XSize) || 
        (x < pDev->x0) || 
        (y >= pDev->y0 + pDev->YSize) || 
        (y < pDev->y0)) {
      GUI_DEBUG_ERROROUT2("_XY2PTR: parameters out of bounds", x, y);
    }
  #endif
  pData += (GUI_ALLOC_DATATYPE_U)(y - pDev->y0) * (GUI_ALLOC_DATATYPE_U)pDev->BytesPerLine;
  return ((U32 *)pData) + x - pDev->x0;
}

/*********************************************************************
*
*       _GetPixel
*/
static U32 _GetPixel(GUI_MEMDEV * pDev, int x, int y) {
  U32 * pData;
  U32 Index;
  pData = _XY2PTR(pDev, x, y);
  Index = *pData;
  return Index;
}

/*********************************************************************
*
*       _GetX
*
* Purpose:
*   The function returns the x-value which is required to get the data
*   from the memory device.
*/
static int _GetX(int h0, int h1, int b, int x0) {
  int sx, sy, x, MirrorX;
  float m0, m1, c0, c1;
  if (h0 == h1) {
    return x0;
  }
  if (h0 < h1) {
    int temp;
    MirrorX = 1;
    temp = h0;
    h0 = h1;
    h1 = temp;
  } else {
    MirrorX = 0;
    x0 = b - x0;
  }
  sx = (b * h0) / (h0 - h1);
  c0 = sqrt((float)((h0 - h1) * (h0 - h1) + b * b));
  c1 = sqrt((float)((sx - b) * (sx - b) + h1 * h1));
  sy = (int)(b * c1 / c0);
  m0 = (float)(sy + h0 + x0) / sx;
  m1 = (float)h0 / sx;
  if (m0 == m1) {
    return x0;
  }
  x = (int)((float)x0 / (m0 - m1));
  if (MirrorX == 0) {
    x = b - x;
  }
  if (x >= b) {
    x = b - 1;
  }
  return x;
}

/*********************************************************************
*
*       _DrawColumnScaled
*/
static void _DrawColumnScaled(GUI_MEMDEV * pDev, int x, int x0, int y0, int yOff, int Num, int Denom) {
  tLCDDEV_Color2Index * pfColor2Index;
  int SkipCnt, NumReps, y, i;
  unsigned Compare, Current;
  pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
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
    y += SkipCnt;
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
    if (y < pDev->YSize) {
      LCD_COLOR Color;
      U8 Alpha;
      U32 Index;
      Color = _GetPixel(pDev, x, y);
      Index = pfColor2Index(Color);
      Alpha = Color >> 24;
      if (Alpha < 255) {
        if (Alpha) {
          for (i = NumReps; i--;) {
            GUI__SetPixelAlpha(x0, y0 + yOff + i, 255 - Alpha, Color);
          }
        } else {
          for (i = NumReps; i--;) {
            LCD_SetPixelIndex(x0, y0 + yOff + i, Index);
          }
        }
      }
    }
    y0 += NumReps;
  } while (++y < pDev->YSize);
}

/*********************************************************************
*
*       _DrawMemdevPerspective
*/
static void _DrawMemdevPerspective(GUI_MEMDEV * pDev, int x0, int y0, int h0, int h1, int dx, int dy) {
  int SkipCnt, NumReps, x, xSize, xStart;
  int Num, yOff0, yOff1, xx, i;
  unsigned Compare, Current;
  xSize = pDev->XSize;
  xStart = x0;
  Current = xSize / 2;
  Compare = xSize;
  x = 0;
  /*
   * Iterate over the original pixels
   */
  do {
    SkipCnt = 0;
    /*
     * Find out how many source columns can be skipped
     */
    while (Current += dx, Current < Compare) {
      SkipCnt++;
    }
    /*
     * Skip the number of lines that are irrelevant
     */
    x += SkipCnt + 1;
    /*
     * Find out how many times this line needs to be repeated
     */
    NumReps = 1;
    while (Compare += xSize, Current >= Compare) {
      NumReps++;
    }
    /*
     * Draw the column(s)
     */
    for (i = 0; i < NumReps; i++) {
      yOff0 = (dy * (x0 - xStart) + (dx >> 1)) / dx;
      yOff1 = ((h0 - h1 - dy) * (x0 - xStart) + (dx >> 1)) / dx;
      Num = h0 - yOff0 - yOff1;
      xx = _GetX(h0, h1, pDev->XSize, x - 1);
      _DrawColumnScaled(pDev, xx, x0 + i, y0, yOff0, Num, pDev->YSize);
    }
    x0 += NumReps;
  } while (x < xSize);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_DrawPerspective
*/
void GUI_MEMDEV_DrawPerspectiveX(GUI_MEMDEV_Handle hMem, int x, int y, int h0, int h1, int dx, int dy) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  GUI_MEMDEV * pDev;
  if (dx != 0) {
    GUI_LOCK();
    pDev = GUI_MEMDEV_H2P(hMem);
    #if (GUI_WINSUPPORT)
      WM_ADDORG(x, y);
      r.x0 = x;
      r.y0 = y + (dy < 0 ? dy : 0);
      r.x1 = r.x0 + dx - 1;
      r.y1 = r.y0 + ((h0 > h1) ? h0 : h1) - 1;
      WM_ITERATE_START(&r) {
    #endif
    _DrawMemdevPerspective(pDev, x, y, h0, h1, dx, dy);
    #if (GUI_WINSUPPORT)
      } WM_ITERATE_END();
    #endif
    GUI_UNLOCK();
  }
}

#else

void GUIDEV_Perspective(void);
void GUIDEV_Perspective(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** End of file ****************************/
