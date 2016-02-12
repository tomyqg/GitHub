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
File        : GUIDEV__WriteToActiveOpaque.c
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/


#include <string.h>

#include "GUI_Private.h"

#if GUI_WINSUPPORT
  #include "WM.h"
#endif

//
// Memory device capabilities are compiled only if support for them is enabled.
//
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV__WriteToActiveOpaque
*/
void GUI_MEMDEV__WriteToActiveOpaque(GUI_MEMDEV_Handle hMem,int x, int y) {
  const LCD_PIXELINDEX  aTrans[2] = {0, 1};
  const LCD_PIXELINDEX* pTrans;
  GUI_MEMDEV          * pDev;
  GUI_USAGE_h           hUsage; 
  GUI_USAGE           * pUsage;
  int                   XSize, YSize, y0, yi, xOff, xSizeDevice, NumPixels;
  unsigned              BytesPerLine_Device;
  unsigned              BitsPerPixel_Layer, BitsPerPixel_Device;
  unsigned              BytesPerPixel_Layer, BytesPerPixel_Device;
  U8                  * pData;
  U32                 * pSrc;
  U8                  * pBuffer;
  U32                   Color;
  GUI_HMEM              hBuffer;
  tLCDDEV_Index2Color * pfIndex2Color_DEV;
  tLCDDEV_Index2Color * pfIndex2Color_DST;
  tLCDDEV_Color2Index * pfColor2Index;

  pDev                = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
  hUsage              = pDev->hUsage; 
  YSize               = pDev->YSize;
  BytesPerLine_Device = pDev->BytesPerLine;
  BitsPerPixel_Device = pDev->BitsPerPixel;
  pData               = (U8 *)(pDev + 1);
  if (BitsPerPixel_Device <= 8) {
    BytesPerPixel_Device = 1;
  } else if (BitsPerPixel_Device <= 16) {
    BytesPerPixel_Device = 2;
  } else {
    BytesPerPixel_Device = 4;
  }
  if (BitsPerPixel_Device == 1) {
    pTrans = aTrans;
  } else {
    pTrans = NULL;
  }
  if (hUsage) {
    pUsage = GUI_USAGE_H2P(hUsage);
    for (yi = 0; yi < YSize; yi++) {
      xOff = 0;
      XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
      if (XSize == pDev->XSize) {
        //
        // If the entire line is affected, calculate the number of entire lines
        //
        y0 = yi;
        while ((GUI_USAGE_GetNextDirty(pUsage, &xOff, yi + 1)) == XSize) {
          yi++;
        }
        LCD_DrawBitmap(x, y + y0, pDev->XSize, yi - y0 + 1, 1, 1, BitsPerPixel_Device, BytesPerLine_Device, pData, pTrans);
        pData += (yi - y0 + 1) * BytesPerLine_Device;
      } else {
        //
        // Draw the partial line which needs to be drawn
        //
        if (BitsPerPixel_Device == 1) {
          //
          // In case of 1bpp bitmaps draw lines of a length of multiples of 8
          //
          for (; XSize; ) {
            XSize = (XSize + (xOff & 7) + 7) & ~0x7;
            xOff  = xOff & ~0x7;
            LCD_DrawBitmap(x + xOff, y + yi, XSize, 1, 1, 1, BitsPerPixel_Device, BytesPerLine_Device, pData + (xOff >> 3), pTrans);
            xOff += XSize;
            XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
          }
        } else {
          for (; XSize; ) {
            LCD_DrawBitmap(x + xOff, y + yi, XSize, 1, 1, 1, BitsPerPixel_Device, BytesPerLine_Device, pData + xOff * BytesPerPixel_Device, pTrans);
            xOff += XSize;
            XSize = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
          }
        }
        pData += BytesPerLine_Device;
      }
    }
  } else {
    //BitsPerPixel_Layer = LCD__GetBPPDevice(GUI_Context.apDevice[GUI_Context.SelLayer]->pColorConvAPI->pfGetIndexMask);
    BitsPerPixel_Layer = LCD_GetBitsPerPixel();
    if (BitsPerPixel_Layer <= 8) {
      BytesPerPixel_Layer = 1;
    } else if (BitsPerPixel_Layer <= 16) {
      BytesPerPixel_Layer = 2;
    } else {
      BytesPerPixel_Layer = 4;
    }
    pfIndex2Color_DEV = pDev->pDevice->pColorConvAPI->pfIndex2Color;
    pfIndex2Color_DST = GUI_GetpfIndex2ColorEx(GUI_Context.SelLayer);
    if (pfIndex2Color_DEV == pfIndex2Color_DST) {
      //
      // Same color conversion, so LCD_DrawBitmap() can be used without converting data
      //
      //LCD_DrawBitmap(x, y, pDev->XSize, YSize, 1, 1, /*BitsPerPixel_Device*/BytesPerPixel_Device << 3, BytesPerLine_Device, pData, NULL);
      LCD_DrawBitmap(x, y, pDev->XSize, YSize, 1, 1, BitsPerPixel_Device, BytesPerLine_Device, pData, pTrans);
    } else if (BytesPerPixel_Device == 4) {
      //
      // Conversion required if color conversion routines are different
      //
      xSizeDevice = pDev->XSize;
      GUI_UNLOCK_H(pDev);
      hBuffer     = GUI_ALLOC_AllocNoInit(BytesPerPixel_Layer * xSizeDevice);
      if (hBuffer) {
        pBuffer       = (U8 *)GUI_LOCK_H(hBuffer);
        pDev          = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
        pSrc          = (U32 *)(pDev + 1);
        pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
        do {
          NumPixels = pDev->XSize;
          switch (BytesPerPixel_Layer) {
          case 1: {
              U8 * pDest;
              U8 Index;
              pDest = (U8 *)pBuffer;
              do {
                Color = *pSrc++;
                Index = pfColor2Index(Color);
                *pDest++ = Index;
              } while (--NumPixels);
            }
            break;
          case 2: {
              U16 * pDest;
              U16 Index;
              pDest = (U16 *)pBuffer;
              do {
                Color = *pSrc++;
                Index = pfColor2Index(Color);
                *pDest++ = Index;
              } while (--NumPixels);
            }
            break;
          case 4: {
              U32 * pDest;
              U32 Index;
              pDest = (U32 *)pBuffer;
              do {
                Color = *pSrc++;
                Index = pfColor2Index(Color);
                *pDest++ = Index;
              } while (--NumPixels);
            }
            break;
          }
          //
          // Draw line by line using LCD_DrawBitmap()
          //
          LCD_DrawBitmap(x, y++, pDev->XSize, 1, 1, 1, BytesPerPixel_Layer << 3, 0, pBuffer, pTrans);
        } while (--YSize);
        GUI_UNLOCK_H(pBuffer);
        GUI_ALLOC_Free(hBuffer);
      }
    } else {
      GUI_DEBUG_WARN("GUI_MEMDEV__WriteToActiveAt: This color depth currently not supported");
    }
  }
  GUI_UNLOCK_H(pDev);
}

#else

void GUIDEV__WriteToActiveOpaque(void);
void GUIDEV__WriteToActiveOpaque(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
