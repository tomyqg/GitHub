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
File        : GUIDEV__WriteToActiveAlpha.c
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
*       Static data
*
**********************************************************************
*/
static U32 * pCurrent; // Buffer for background
static U32 * pConvert; // Buffer for result

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DoAlphaBlending
*/
static void _DoAlphaBlending(int x, int y, int yOff, GUI_MEMDEV_Handle hMem, int xSize, int ConversionRequired, tLCDDEV_Index2Color * pfIndex2Color) {
  U32 * pData;
  U32 Color;
  U8 Alpha;
  int BitsPerPixelCurrent, x0, Off, xOff, xSizeOrg;
  int Read = 0;
  int Index = 0;
  static int VXSizeMax;
  GUI_DEVICE * pDevice;

  GUI_USE_PARA(ConversionRequired);
  pData = (U32 *)GUI_MEMDEV_GetDataPtr(hMem) + xSize * yOff;
  //
  // Allocate destination buffer
  //
  if (pConvert == NULL) {
    VXSizeMax = LCD_GetVXSizeMax();
    pConvert  = (U32 *)GUI_ALLOC_GetFixedBlock(VXSizeMax << 2);
  }
  if (xSize > VXSizeMax) {
    xSize = VXSizeMax;
  }
  xSizeOrg = xSize;
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];
  BitsPerPixelCurrent = LCD_GetBitsPerPixel();
  xOff = 0;
  do {
    Color = *pData++;
    xOff++;
    Alpha = Color >> 24;
    if (Alpha) {
      //
      // Allocate background buffer
      //
      if (pCurrent == NULL) {
        pCurrent = (U32 *)GUI_ALLOC_GetFixedBlock(VXSizeMax << 2);
        pData = (U32 *)GUI_MEMDEV_GetDataPtr(hMem) + (xSizeOrg * yOff) + xOff;
      }
      //
      // Read background data of whole line if not already done
      //
      if (Read == 0) {
        x0 = x + Index;
        if (GUI_Context.ClipRect.x0 > x0) { 
          Off = GUI_Context.ClipRect.x0 - x0;
          x0  = GUI_Context.ClipRect.x0;
        } else {
          Off = 0;
        }
        GUI_ReadRectEx(x0, y, x + xSizeOrg - 1, y, pCurrent + Index + Off, pDevice);
        Read = 1;
        //if (ConversionRequired) {
        if (BitsPerPixelCurrent < 24) {
          GUI__ExpandPixelIndicesEx(pCurrent + Index, xSize, BitsPerPixelCurrent, pfIndex2Color);
        }
      }
      if (Alpha == 255) {
        //
        // Use background
        //
        *(pConvert + Index) = *(pCurrent + Index);
      } else {
        //
        // Mix colors
        //
        *(pConvert + Index) = GUI__MixColors(Color, *(pCurrent + Index), 255 - Alpha);
      }
    } else {
      *(pConvert + Index) = Color;
    }
    Index++;
  } while (--xSize);
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV__WriteToActiveAlpha
*/
void GUI_MEMDEV__WriteToActiveAlpha(GUI_MEMDEV_Handle hMem,int x, int y) {
  GUI_DEVICE          * pDevice;
  GUI_MEMDEV          * pDev;
  int                   ySizeDevice, xSizeDevice, i, ConversionRequired;
  unsigned              BitsPerPixel_DST;
  unsigned              BytesPerPixel_DST;
  tLCDDEV_Index2Color * pfIndex2Color_DEV;
  tLCDDEV_Index2Color * pfIndex2Color_DST;
  tLCDDEV_Color2Index * pfColor2Index_DST;

  //
  // Get memory device properties
  //
  pDev = (GUI_MEMDEV *)GUI_LOCK_H(hMem); {
    xSizeDevice       = pDev->XSize;
    ySizeDevice       = pDev->YSize;
    pfIndex2Color_DEV = pDev->pDevice->pColorConvAPI->pfIndex2Color;
  } GUI_UNLOCK_H(pDev);
  //
  // Calculate bytes per pixel of current layer
  //
  pDevice = GUI_Context.apDevice[GUI_Context.SelLayer];
  BitsPerPixel_DST = pDevice->pDeviceAPI->pfGetDevProp(pDevice, LCD_DEVCAP_BITSPERPIXEL);
  if (BitsPerPixel_DST <= 8) {
    BytesPerPixel_DST = 1;
  } else if (BitsPerPixel_DST <= 16) {
    BytesPerPixel_DST = 2;
  } else {
    BytesPerPixel_DST = 4;
  }
  //
  // Get API pointer of destination device
  //
  pfIndex2Color_DST = GUI_GetpfIndex2ColorEx(GUI_Context.SelLayer);
  pfColor2Index_DST = GUI_GetpfColor2IndexEx(GUI_Context.SelLayer);
  ConversionRequired = (pfIndex2Color_DEV == pfIndex2Color_DST) ? 0 : 1;
  //
  // Iterate over all lines
  //
  i = 0;
  do {
    //
    // Do alpha blending (if required)
    //
    _DoAlphaBlending(x, y, i++, hMem, xSizeDevice, ConversionRequired, pfIndex2Color_DST);
    //
    // Compact result (if required)
    //
    if (ConversionRequired) {
      GUI__CompactPixelIndicesEx(pConvert, xSizeDevice, BitsPerPixel_DST, pfColor2Index_DST);
    }
    //
    // Draw result with current device
    //
    LCD_DrawBitmap(x, y++, xSizeDevice, 1, 1, 1, BytesPerPixel_DST << 3, 0, (U8 *)pConvert, NULL);
  } while (--ySizeDevice);
}

#else

void GUIDEV__WriteToActiveAlpha(void);
void GUIDEV__WriteToActiveAlpha(void) {}

#endif /* GUI_SUPPORT_MEMDEV */

/*************************** end of file ****************************/
