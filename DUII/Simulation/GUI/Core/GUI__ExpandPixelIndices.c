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
File        : GUI__ExpandPixelIndices.c
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
*       GUI__ExpandPixelIndices
*/
void GUI__ExpandPixelIndices(void * pBuffer, int NumPixels, int BitsPerPixel) {
  int BytesPerPixel;
  U32 * pDest;

  pDest = (U32 *)pBuffer;
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
      case 2: { // Convert from U16 to U32
          U16 * pSrc;
          pSrc = (U16 *)pBuffer;
          pSrc  += NumPixels;
          pDest += NumPixels;
          do {
            *--pDest = *--pSrc;
          } while (--NumPixels);
        }
        break;
      case 1: { // Convert from U8 to U32
          U8 * pSrc;
          pSrc = (U8 *)pBuffer;
          pSrc  += NumPixels;
          pDest += NumPixels;
          do {
            *--pDest = *--pSrc;
          } while (--NumPixels);
        }
        break;
      }
    #elif (GUI_BYTESPERPIXEL == 2)
      {
        U8 * pSrc;
        pSrc = (U8 *)pBuffer;
        pSrc  += NumPixels;
        pDest += NumPixels;
        do {
          *--pDest = *--pSrc;
        } while (--NumPixels);
      }
    #endif
  }
}

/*********************************************************************
*
*       GUI__ExpandPixelIndicesEx
*/
void GUI__ExpandPixelIndicesEx(void * pBuffer, int NumPixels, int BitsPerPixel, tLCDDEV_Index2Color * pfIndex2Color) {
  int BytesPerPixel;
  U32 * pDest;

  pDest = (U32 *)pBuffer;
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
      case 2: { // Convert from U16 to U32
          U16 * pSrc;
          pSrc = (U16 *)pBuffer;
          pSrc  += NumPixels;
          pDest += NumPixels;
          do {
            *--pDest = pfIndex2Color(*--pSrc);
          } while (--NumPixels);
        }
        break;
      case 1: { // Convert from U8 to U32
          U8 * pSrc;
          pSrc = (U8 *)pBuffer;
          pSrc  += NumPixels;
          pDest += NumPixels;
          do {
            *--pDest = pfIndex2Color(*--pSrc);
          } while (--NumPixels);
        }
        break;
      }
    #elif (GUI_BYTESPERPIXEL == 2)
      {
        U8 * pSrc;
        pSrc = (U8 *)pBuffer;
        pSrc  += NumPixels;
        pDest += NumPixels;
        do {
          *--pDest = pfIndex2Color(*--pSrc);
        } while (--NumPixels);
      }
    #endif
  }
}
/*************************** End of file ****************************/
