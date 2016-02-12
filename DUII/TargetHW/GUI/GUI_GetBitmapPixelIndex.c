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
File        : GUI_GetBitmapPixelIndex.c
Purpose     : Implementation of GUI_GetBitmapPixelIndex
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetBitmapPixelIndexEx
*/
int GUI_GetBitmapPixelIndexEx(int BitsPerPixel, int BytesPerLine, const U8 GUI_UNI_PTR * pData, unsigned x, unsigned y) {
  unsigned Off, Value;
  const U32 GUI_UNI_PTR * p32;
  const U16 GUI_UNI_PTR * p16;
  switch (BitsPerPixel) {
  case 1:
    Off   = (x >> 3) + (y * BytesPerLine);
    Value = *(pData + Off);
    Value = Value >> (7 - (x & 0x7)) & 0x1;
    break;
  case 2:
    Off   = (x >> 2) + (y * BytesPerLine);
    Value = *(pData + Off);
    Value = Value >> (6 - ((x << 1) & 0x6)) & 0x3;
    break;
  case 4:
    Off   = (x >> 1) + (y * BytesPerLine);
    Value = *(pData + Off);
    Value = (x & 1) ? (Value & 0xF) : (Value >> 4);
    break;
  case 8:
    Off   = x + y * BytesPerLine;
    Value = *(pData + Off);
    break;
  case 16:
    p16   = (const U16 GUI_UNI_PTR *)pData;
    Off   = x + y * (BytesPerLine >> 1);
    Value = *(p16 + Off);
    break;
  case 32:
    p32   = (const U32 GUI_UNI_PTR *)pData;
    Off   = x + y * (BytesPerLine >> 2);
    Value = *(p32 + Off);
    break;
  default:
    Value = 0;
  }
  return Value;
}

/*********************************************************************
*
*       GUI_GetBitmapPixelIndex
*/
int GUI_GetBitmapPixelIndex(const GUI_BITMAP GUI_UNI_PTR * pBMP, unsigned x, unsigned y) {
  if (!pBMP) {
    return 0;
  }
  return GUI_GetBitmapPixelIndexEx(pBMP->BitsPerPixel, pBMP->BytesPerLine, pBMP->pData, x, y);
}

/*************************** End of file ****************************/
