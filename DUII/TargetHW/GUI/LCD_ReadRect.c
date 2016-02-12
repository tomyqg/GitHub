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
File        : LCD_ReadRect.c
Purpose     : Implementation of LCD_ReadRect
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

#define CLIP_X() \
  if (x0 < GUI_Context.ClipRect.x0) { x0 = GUI_Context.ClipRect.x0; } \
  if (x1 > GUI_Context.ClipRect.x1) { x1 = GUI_Context.ClipRect.x1; }

#define CLIP_Y() \
  if (y0 < GUI_Context.ClipRect.y0) { y0 = GUI_Context.ClipRect.y0; } \
  if (y1 > GUI_Context.ClipRect.y1) { y1 = GUI_Context.ClipRect.y1; }

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_ReadRectEx
*/
static void LCD_ReadRectEx(int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer, GUI_DEVICE * pDevice) {
  void       (*(* pfGetDevFunc)(GUI_DEVICE ** ppDevice, int Index))(void);
  unsigned   (* pfGetPixelIndex)(GUI_DEVICE *, int, int);
  void       (* pfReadRect)     (GUI_DEVICE *, int, int, int, int, LCD_PIXELINDEX *);
  
  pfGetDevFunc    = pDevice->pDeviceAPI->pfGetDevFunc;
  pfGetPixelIndex = pDevice->pDeviceAPI->pfGetPixelIndex;
  if (pfGetDevFunc) {
    pfReadRect = (void (*)(GUI_DEVICE *, int, int, int, int, LCD_PIXELINDEX *))pfGetDevFunc(&pDevice, LCD_DEVFUNC_READRECT);
  } else {
    pfReadRect = NULL;
  }
  if (!pfReadRect) {
    U8 * pData;
    int BytesPerPixel;
    int BitsPerPixel;
    BitsPerPixel = LCD_GetBitsPerPixel();
    if (BitsPerPixel <= 8) {
      BytesPerPixel = 1;
    } else if (BitsPerPixel <= 16) {
      BytesPerPixel = 2;
    } else {
      BytesPerPixel = 4;
    }
    pData = (U8 *)pBuffer;
    while (y0 <= y1) {
      int x;
      for (x = x0; x <= x1; x++) {
        switch (BytesPerPixel) {
        case 1:
          *((U8 *)pData) = pfGetPixelIndex(pDevice, x, y0);
          break;
        case 2:
          *((U16 *)pData) = pfGetPixelIndex(pDevice, x, y0);
          break;
        case 4:
          *((U32 *)pData) = pfGetPixelIndex(pDevice, x, y0);
          break;
        }
        pData += BytesPerPixel;
      }
      y0++;
    }
  } else {
    pfReadRect(pDevice, x0, y0, x1, y1, pBuffer);
  }
}

/*********************************************************************
*
*       GUI_ReadRectEx
*/
void GUI_ReadRectEx(int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer, GUI_DEVICE * pDevice) {
  CLIP_X();
  if (x1 < x0) {
    return;
  }
  CLIP_Y();
  if (y1 < y0) {
    return;
  }
  while ((pDevice->pDeviceAPI->DeviceClassIndex != DEVICE_CLASS_DRIVER) &&
         (pDevice->pDeviceAPI->DeviceClassIndex != DEVICE_CLASS_MEMDEV) &&
         pDevice->pNext) {
    pDevice = pDevice->pNext;
  }
  LCD_ReadRectEx(x0, y0, x1, y1, pBuffer, pDevice);
}

/*********************************************************************
*
*       LCD_ReadRect
*/
void LCD_ReadRect(int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer, GUI_DEVICE * pDevice) {
  while ((pDevice->pDeviceAPI->DeviceClassIndex != DEVICE_CLASS_DRIVER) &&
         pDevice->pNext) {
    pDevice = pDevice->pNext;
  }
  LCD_ReadRectEx(x0, y0, x1, y1, pBuffer, pDevice);
}

/*********************************************************************
*
*       GUI_ReadRect
*/
void GUI_ReadRect(int x0, int y0, int x1, int y1, LCD_PIXELINDEX * pBuffer, GUI_DEVICE * pDevice) {
  GUI_ReadRectEx(x0, y0, x1, y1, pBuffer, pDevice);
  GUI__ExpandPixelIndices(pBuffer, (x1 - x0 + 1) * (y1 - y0 + 1), LCD_GetBitsPerPixel());
}

/*************************** End of file ****************************/
