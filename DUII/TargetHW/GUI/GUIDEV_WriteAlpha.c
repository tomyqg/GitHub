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
File        : GUIDEV_WriteAlpha.C
Purpose     : Implementation of memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>

#include "GUI_Private.h"

/* Memory device capabilities are compiled only if support for them is enabled.*/ 
#if GUI_SUPPORT_MEMDEV

/*********************************************************************
*
*       Static routines
*
**********************************************************************
*/
/*********************************************************************
*
*       _ReadLine
*/
static void _ReadLine(int x0, int y, int x1, LCD_PIXELINDEX * pBuffer) {
  LCD_RECT r;
  GUI_DEVICE * pDevice;
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];

  pDevice->pDeviceAPI->pfGetRect(pDevice, &r);
  if (x0 > r.x1) {
    return;
  }
  if (x1 < r.x0) {
    return;
  }
  if (y > r.y1) {
    return;
  }
  if (y < r.y0) {
    return;
  }
  if (x0 < r.x0) {
    pBuffer += r.x0 - x0;
    x0 = r.x0;
  }
  if (x1 > r.x1) {
    x1 = r.x1;
  }
  GUI_ReadRectEx(x0, y, x1, y, pBuffer, pDevice); //GUI_ReadRect replaced with GUI_ReadRectEx because of problem with transparency in Dashboard sample
}

/*********************************************************************
*
*       _WriteAlphaToActiveAt
*/
static void _WriteAlphaToActiveAt(GUI_MEMDEV_Handle hMem, int Intens, int x, int y) {
  GUI_MEMDEV * pDev;
  GUI_USAGE_h hUsage; 
  GUI_USAGE * pUsage;
  GUI_HMEM hBuffer, hBufferBk;
  U8 * pBuffer;
  U8 * pBufferBk;
  U8 * pData;
  tLCDDEV_Index2Color * pfIndex2Color;
  tLCDDEV_Color2Index * pfColor2Index;
  LCD_COLOR Color, BkColor;
  int Index, xSize, ySize, NumPixels, NumPixelsBitmap, yi, xOff, xPos, yPos, n, xOffFirst;
  int BitsPerPixelSRC, BitsPerPixelDST;
  int BytesPerPixelSRC, BytesPerPixelDST;
  
  if (hMem) {
    pDev   = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
    hUsage = pDev->hUsage; 
    xSize  = pDev->XSize;
    ySize  = pDev->YSize;
    GUI_UNLOCK_H(pDev);
    if (hUsage) {
      //
      // Allocate buffer for result
      //
      hBuffer   = GUI_ALLOC_AllocNoInit(xSize * sizeof(LCD_PIXELINDEX));
      if (hBuffer) {
        //
        // Allocate buffer for background
        //
        hBufferBk = GUI_ALLOC_AllocNoInit(xSize * sizeof(LCD_PIXELINDEX));
        if (hBufferBk) {
          //
          // Get data pointers
          //
          pUsage        = (GUI_USAGE  *)GUI_LOCK_H(hUsage);
          pDev          = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
          pBuffer       = (U8 *)GUI_LOCK_H(hBuffer);
          pBufferBk     = (U8 *)GUI_LOCK_H(hBufferBk);
          //
          // Get color depth(s)
          //
          if (GUI_Context.hDevData) {
            BitsPerPixelDST = GUI_MEMDEV_GetBitsPerPixel(GUI_Context.hDevData);
          } else {
            BitsPerPixelDST = LCD_GetBitsPerPixelEx(GUI_Context.SelLayer);
          }
          if (BitsPerPixelDST <= 8) {
            BytesPerPixelDST = 1;
          } else if (BitsPerPixelDST <= 16) {
            BytesPerPixelDST = 2;
          } else {
            BytesPerPixelDST = 4;
          }
          BitsPerPixelSRC = pDev->BitsPerPixel;
          if (BitsPerPixelSRC <= 8) {
            BytesPerPixelSRC = 1;
          } else if (BitsPerPixelSRC <= 16) {
            BytesPerPixelSRC = 2;
          } else {
            BytesPerPixelSRC = 4;
          }
          //
          // Get function pointers
          //
          pfIndex2Color = GUI_GetpfIndex2ColorEx(GUI_Context.SelLayer);
          pfColor2Index = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
          //
          // Iterate over all lines
          //
          for (yi = 0; yi < ySize; yi++) {
            xOff      = 0;
            NumPixels = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
            yPos      = yi + y;
            //
            // Draw the partial line which needs to be drawn
            //
            while (NumPixels) {
              xPos            = xOff + x;
              pData           = (U8 *)GUI_MEMDEV__XY2PTREx(pDev, xOff, yi);
              NumPixelsBitmap = NumPixels;
              //
              // Get background data
              //
              _ReadLine(xPos, yPos, xPos + NumPixels - 1, (LCD_PIXELINDEX *)pBufferBk/*, GUI_Context.pDeviceAPI*/);
              //
              // Do the calculation
              //
              n = 0;
              xOffFirst = xOff;
              switch (BytesPerPixelDST) {
              case 1: {
                  U8 * pDst;
                  U8 * pBk;
                  pDst = (U8 *)pBuffer;
                  pBk  = (U8 *)pBufferBk;
                  switch (BytesPerPixelSRC) {
                  case 1: {
                      U8 * pSrc;
                      pSrc = (U8 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 2: {
                      U16 * pSrc;
                      pSrc = (U16 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 4: {
                      U32 * pSrc;
                      pSrc = (U32 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  }
                }
                break;
              case 2: {
                  U16 * pDst;
                  U16 * pBk;
                  pDst = (U16 *)pBuffer;
                  pBk  = (U16 *)pBufferBk;
                  switch (BytesPerPixelSRC) {
                  case 1: {
                      U8 * pSrc;
                      pSrc = (U8 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 2: {
                      U16 * pSrc;
                      pSrc = (U16 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 4: {
                      U32 * pSrc;
                      pSrc = (U32 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  }
                }
                break;
              case 4: {
                  U32 * pDst;
                  U32 * pBk;
                  pDst = (U32 *)pBuffer;
                  pBk  = (U32 *)pBufferBk;
                  switch (BytesPerPixelSRC) {
                  case 1: {
                      U8 * pSrc;
                      pSrc = (U8 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 2: {
                      U16 * pSrc;
                      pSrc = (U16 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  case 4: {
                      U32 * pSrc;
                      pSrc = (U32 *)pData;
                      do {
                        Index   = *(pSrc + n);
                        Color   = pfIndex2Color(Index);
                        BkColor = pfIndex2Color(*(pBk + n));
                        Color   = LCD_MixColors256(Color, BkColor, Intens);
                        Index   = pfColor2Index(Color);
                        *(pDst + xOff) = Index;
                        xOff++;
                        n++;
                      } while (--NumPixels);
                    }
                    break;
                  }
                }
                break;
              }
              //
              // Draw result
              //
              LCD_DrawBitmap(xPos, yPos, NumPixelsBitmap, 1, 1, 1, BitsPerPixelDST, NumPixelsBitmap * sizeof(LCD_PIXELINDEX), (U8 *)(pBuffer + xOffFirst * BytesPerPixelDST), NULL);
              NumPixels = GUI_USAGE_GetNextDirty(pUsage, &xOff, yi);
            }
          }
          //
          // Unlock pointers...
          //
          GUI_UNLOCK_H(pUsage);
          GUI_UNLOCK_H(pDev);
          GUI_UNLOCK_H(pBuffer);
          GUI_UNLOCK_H(pBufferBk);
          //
          // ...and release memory
          //
          GUI_ALLOC_Free(hBufferBk);
        }
        GUI_ALLOC_Free(hBuffer);
      }
    }
  }
}

/*********************************************************************
*
*       Exported routines
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_WriteAlphaAt
*/
void GUI_MEMDEV_WriteAlphaAt(GUI_MEMDEV_Handle hMem, int Alpha, int x, int y) {
  if (hMem) {
    GUI_MEMDEV* pDevData;
    #if (GUI_WINSUPPORT)
      GUI_RECT r;
    #endif
    GUI_LOCK();
    pDevData = (GUI_MEMDEV*) GUI_ALLOC_h2p(hMem);  // Convert to pointer
    if (x == GUI_POS_AUTO) {
      x = pDevData->x0;
      y = pDevData->y0;
    }
    #if (GUI_WINSUPPORT)
      r.x1 = (r.x0 = x) + pDevData->XSize-1;
      r.y1 = (r.y0 = y) + pDevData->YSize-1;;
      WM_ITERATE_START(&r) {
      _WriteAlphaToActiveAt(hMem, Alpha, x,y);
      } WM_ITERATE_END();
    #else
      _WriteAlphaToActiveAt(hMem, Alpha, x,y);
    #endif
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_MEMDEV_WriteAlpha
*/
void GUI_MEMDEV_WriteAlpha(GUI_MEMDEV_Handle hMem, int Alpha) {
  GUI_MEMDEV_WriteAlphaAt(hMem, Alpha, GUI_POS_AUTO, GUI_POS_AUTO);
}

#else

void GUIDEV_WriteAlpha_C(void);
void GUIDEV_WriteAlpha_C(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
