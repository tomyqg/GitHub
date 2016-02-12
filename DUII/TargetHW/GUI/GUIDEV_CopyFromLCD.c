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
File        : GUIDEV_CopyFromLCD.c
Purpose     : Implementation of memory devices
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled. */ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       defines
*
**********************************************************************
*/

#define LCD_LIMIT(Var, Op, Limit) if (Var Op Limit) Var = Limit

/*********************************************************************
*
*       public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_CopyFromLCD
*/
void GUI_MEMDEV_CopyFromLCD(GUI_MEMDEV_Handle hMem) {
  GUI_MEMDEV * pDev;
  LCD_RECT r;
  int y;
  int XMax;
  unsigned BitsPerPixelDev;
  unsigned BitsPerPixelLCD;
  unsigned BytesPerPixelDev;
  unsigned BytesPerPixelLCD;
  GUI_USAGE * pUsage;
  GUI_MEMDEV_Handle hMemOld;
  LCD_PIXELINDEX * pData;
  tLCDDEV_Index2Color *   pfIndex2Color_DEV;
  tLCDDEV_Index2Color *   pfIndex2Color_LCD;

  GUI_LOCK();
  /* Make sure memory handle is valid */
  if (!hMem) {
    hMem = GUI_Context.hDevData;
  }
  if (hMem) {
    pDev    = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
    pUsage  = 0;
    hMemOld = GUI_Context.hDevData;
    GUI_MEMDEV_Select(hMem);
    if (pDev->hUsage) {
      pUsage = GUI_USAGE_H2P(pDev->hUsage);
    }
    /*
    * Get number of bytes per pixel (Device)
    */
    BitsPerPixelDev = GUI_MEMDEV_GetBitsPerPixel(hMem);
    if (BitsPerPixelDev <= 8) {
      BytesPerPixelDev = 1;
    } else if (BitsPerPixelDev <= 16) {
      BytesPerPixelDev = 2;
    } else {
      BytesPerPixelDev = 4;
    }
    /*
    * Get number of bytes per pixel (LCD)
    */
    BitsPerPixelLCD = LCD_GetBitsPerPixel();
    if (BitsPerPixelLCD <= 8) {
      BytesPerPixelLCD = 1;
    } else if (BitsPerPixelLCD <= 16) {
      BytesPerPixelLCD = 2;
    } else {
      BytesPerPixelLCD = 4;
    }
    GUI_MEMDEV_Select(hMemOld);
    /*
    * Do operation only if color depth of device > display
    */
    if (BytesPerPixelLCD <= BytesPerPixelDev) {
      /* Get bounding rectangle */
      r.y0 = pDev->y0;
      r.x0 = pDev->x0;
      r.x1 = pDev->x0 + pDev->XSize - 1;
      r.y1 = pDev->y0 + pDev->YSize - 1;
      /* Make sure bounds are within LCD area so we can call driver directly */
      LCD_LIMIT(r.x0, <, 0);
      LCD_LIMIT(r.y0, <, 0);
      LCD_LIMIT(r.x1, >, LCD_GetVXSize() - 1);
      LCD_LIMIT(r.y1, >, LCD_GetVYSize() - 1);
      XMax = r.x1;
      for (y = r.y0; y <= r.y1; y++) {
        pData = (LCD_PIXELINDEX *)GUI_MEMDEV__XY2PTREx(pDev, 0, y - r.y0);
        LCD_ReadRect(r.x0, y, XMax, y, (LCD_PIXELINDEX *)pData, GUI_Context.apDevice[GUI_Context.SelLayer]);
        if (pUsage) {
          GUI_USAGE_AddHLine(pUsage, r.x0, y, r.x1 - r.x0 + 1);
        }
      }
      pfIndex2Color_DEV = pDev->pDevice->pColorConvAPI->pfIndex2Color;
      pfIndex2Color_LCD = LCD_GetpfIndex2ColorEx(GUI_Context.SelLayer);
      /*
      * Conversion required if color conversion routines are different
      */
      if (pfIndex2Color_DEV != pfIndex2Color_LCD) {
        U32 Color;
        unsigned NumPixels;
        /*
        * Currently only 32 bpp devices are supported
        */
        if (BytesPerPixelDev == 4) {
          for (y = r.y0; y <= r.y1; y++) {
            U32 * pDst;
            NumPixels = r.x1 - r.x0 + 1;
            pDst = (U32 *)GUI_MEMDEV__XY2PTREx(pDev, 0, y - r.y0) + NumPixels;
            switch (BytesPerPixelLCD) {
            case 2: {
                U16 * pSrc;
                U16 PixelIndex;
                pSrc = (U16 *)GUI_MEMDEV__XY2PTREx(pDev, 0, y - r.y0) + NumPixels;
                do {
                  PixelIndex = *(--pSrc);
                  Color = pfIndex2Color_LCD(PixelIndex);
                  *(--pDst) = Color;
                } while (--NumPixels);
              }
              break;
            case 4: {
                U32 * pSrc;
                U32 PixelIndex;
                pSrc = (U32 *)GUI_MEMDEV__XY2PTREx(pDev, 0, y - r.y0) + NumPixels;
                do {
                  PixelIndex = *(--pSrc);
                  Color = pfIndex2Color_LCD(PixelIndex);
                  *(--pDst) = Color;
                } while (--NumPixels);
              }
              break;
            }
          }
        }
      }
    }
    GUI_UNLOCK_H(pDev);
  }
  GUI_UNLOCK();
}

#else

void GUI_MEMDEV_CopyFromLCD_C(void);
void GUI_MEMDEV_CopyFromLCD_C(void) {} /* avoid empty object files */

#endif /* GUI_MEMDEV_SUPPORT */

/*************************** end of file ****************************/
