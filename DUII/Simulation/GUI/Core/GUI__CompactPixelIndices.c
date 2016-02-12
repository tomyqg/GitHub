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
File        : GUI__CompactPixelIndices.c
Purpose     : ...
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
*       GUI__CompactPixelIndices
*/
void GUI__CompactPixelIndices(LCD_PIXELINDEX * pBuffer, int NumPixels, int BitsPerPixel) {
  int BytesPerPixel;
  if (BitsPerPixel <= 8) {
    BytesPerPixel = 1;
  } else if (BitsPerPixel <= 16) {
    BytesPerPixel = 2;
  } else {
    BytesPerPixel = 4;
  }
  if (BytesPerPixel < GUI_BYTESPERPIXEL) {
    #if   (GUI_BYTESPERPIXEL == 4)
      switch (BytesPerPixel) {
      case 2: { // Convert from U32 to U16
          U16 * pDest;
          pDest = (U16 *)pBuffer;
          do {
            *pDest++ = *pBuffer++;
          } while (--NumPixels);
        }
        break;
      case 1: { // Convert from U32 to U8
          U8 * pDest;
          U8 Byte;
          pDest = (U8 *)pBuffer;
          switch (BitsPerPixel) {
          case 1:
            do {
              Byte  = ((*pBuffer++) & 0x1) << 7;
              Byte |= ((*pBuffer++) & 0x1) << 6;
              Byte |= ((*pBuffer++) & 0x1) << 5;
              Byte |= ((*pBuffer++) & 0x1) << 4;
              Byte |= ((*pBuffer++) & 0x1) << 3;
              Byte |= ((*pBuffer++) & 0x1) << 2;
              Byte |= ((*pBuffer++) & 0x1) << 1;
              Byte |= ((*pBuffer++) & 0x1);
              *pDest++ = Byte;
            } while (--NumPixels);
          case 2:
            do {
              Byte  = ((*pBuffer++) & 0x3) << 6;
              Byte |= ((*pBuffer++) & 0x3) << 4;
              Byte |= ((*pBuffer++) & 0x3) << 2;
              Byte |= ((*pBuffer++) & 0x3);
              *pDest++ = Byte;
            } while (--NumPixels);
          case 4:
            do {
              Byte  = ((*pBuffer++) & 0xF) << 4;
              Byte |= ((*pBuffer++) & 0xF);
              *pDest++ = Byte;
            } while (--NumPixels);
            break;
          default:
            do {
              *pDest++ = *pBuffer++;
            } while (--NumPixels);
            break;
          }
        }
        break;
      }
    #elif (GUI_BYTESPERPIXEL == 2)
      {
        U8 * pDest;
        pDest = (U8 *)pBuffer;
        do {
          *pDest++ = *pBuffer++;
        } while (--NumPixels);
      }
    #endif
  }
}

/*********************************************************************
*
*       GUI__CompactPixelIndicesEx
*/
void GUI__CompactPixelIndicesEx(LCD_PIXELINDEX * pBuffer, int NumPixels, int BitsPerPixel, tLCDDEV_Color2Index * pfColor2Index) {
  int BytesPerPixel;
  if (BitsPerPixel <= 8) {
    BytesPerPixel = 1;
  } else if (BitsPerPixel <= 16) {
    BytesPerPixel = 2;
  } else {
    BytesPerPixel = 4;
  }
  if (BytesPerPixel < GUI_BYTESPERPIXEL) {
    #if   (GUI_BYTESPERPIXEL == 4)
      switch (BytesPerPixel) {
      case 2: { // Convert from U32 to U16
          U16 * pDest;
          pDest = (U16 *)pBuffer;
          do {
            *pDest++ = pfColor2Index(*pBuffer++);
          } while (--NumPixels);
        }
        break;
      case 1: { // Convert from U32 to U8
          U8 * pDest;
          pDest = (U8 *)pBuffer;
          do {
            *pDest++ = pfColor2Index(*pBuffer++);
          } while (--NumPixels);
        }
        break;
      }
    #elif (GUI_BYTESPERPIXEL == 2)
      {
        U8 * pDest;
        pDest = (U8 *)pBuffer;
        do {
          *pDest++ = pfColor2Index(*pBuffer++);
        } while (--NumPixels);
      }
    #endif
  }
}

/*************************** End of file ****************************/
