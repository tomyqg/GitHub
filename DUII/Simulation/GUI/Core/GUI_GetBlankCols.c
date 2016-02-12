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
File        : GUI_GetBlankCols
Purpose     : Calculating of leading and trailing blank columns
              of a character
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  int xSize;
} DRIVER_CONTEXT;

/*********************************************************************
*
*       Static code, _DeviceAPI functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _XY2PTR
*/
static LCD_PIXELINDEX * _XY2PTR(int x, int y, DRIVER_CONTEXT * pContext) {
  return ((LCD_PIXELINDEX *)(pContext + 1)) + y * pContext->xSize + x;
}

/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void _DrawBitLine1BPP(const U8 GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const LCD_PIXELINDEX * pTrans, LCD_PIXELINDEX * pDest)
{
  unsigned Pixels;
  unsigned PixelCnt;
  PixelCnt = 8 - Diff;
  Pixels = LCD_aMirror[*p] >> Diff;

  do {
    //
    // Prepare loop
    //
    if (PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    //
    // Write as many pixels as we are allowed to and have loaded in this inner loop
    //
    do {
      *pDest++ = *(pTrans + (Pixels & 1));
      Pixels >>= 1;
    } while (--PixelCnt);
    //
    // Check if an other Source byte needs to be loaded
    //
    if (xsize == 0) {
      return;
    }
    PixelCnt = 8;
    Pixels = LCD_aMirror[*++p];
  } while (1);
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void _DrawBitLine2BPP(const U8 GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const LCD_PIXELINDEX * pTrans, LCD_PIXELINDEX * pDest)
{
  U8 pixels;
  U8  PixelCnt;
  PixelCnt = 4 - Diff;
  pixels = (*p) << (Diff << 1);
  do {
    if ((unsigned)PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      *pDest++ = *(pTrans + (pixels >> 6));
      pixels <<= 2;
    } while (--PixelCnt);
    if (xsize == 0) {
      break;
    }
    PixelCnt = 4;
    pixels = *(++p);
  } while (xsize);
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void _DrawBitLine4BPP(const U8 GUI_UNI_PTR * p, int Diff, unsigned int xsize,
                             const LCD_PIXELINDEX * pTrans, LCD_PIXELINDEX * pDest)
{
  U8 pixels;
  U8  PixelCnt;
  PixelCnt = 2 - Diff;
  pixels = (*p) << (Diff << 2);
  do {
    if ((unsigned)PixelCnt > xsize) {
      PixelCnt = xsize;
    }
    xsize -= PixelCnt;
    do {
      *pDest++ = *(pTrans + (pixels >> 4));
      pixels <<= 4;
    } while (--PixelCnt);
    if (xsize == 0) {
      break;
    }
    PixelCnt = 2;
    pixels = *(++p);
  } while (xsize);
}

/*********************************************************************
*
*             _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0, int xsize, int ysize,
                       int BitsPerPixel, int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans)
{
  int i;
  LCD_PIXELINDEX * pDest;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)GUI_LOCK_H(pDevice->u.hContext); {
    x0 += Diff;
    pDest = _XY2PTR(x0, y0, pContext);
    for (i = 0; i < ysize; i++) {
      switch (BitsPerPixel) {
      case 1:
        _DrawBitLine1BPP(pData, Diff, xsize, pTrans, pDest);
        break;
      case 2:
        _DrawBitLine2BPP(pData, Diff, xsize, pTrans, pDest);
        break;
      case 4:
        _DrawBitLine4BPP(pData, Diff, xsize, pTrans, pDest);
        break;
      }
      pData += BytesPerLine;
      pDest += pContext->xSize; 
    }
  } GUI_UNLOCK_H(pContext);
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  int i;
  LCD_PIXELINDEX * pDest;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)GUI_LOCK_H(pDevice->u.hContext); {
    do {
      pDest = _XY2PTR(x0, y0, pContext);
      for (i = x1 - x0 + 1; i; --i) {
        *pDest++ = LCD_COLORINDEX;
      }
    } while (++y0 <= y1);
  } GUI_UNLOCK_H(pContext);
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  GUI_USE_PARA(pDevice);
  pRect->x0 = pRect->y0 = -4095;
  pRect->x1 = pRect->y1 =  4095;
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int Index) {
  LCD_PIXELINDEX * pDest;
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)GUI_LOCK_H(pDevice->u.hContext); {
    pDest = _XY2PTR(x, y, pContext);
    *pDest = Index;
  } GUI_UNLOCK_H(pContext);
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
static const GUI_DEVICE_API _DeviceAPI = {
  //
  // Data
  //
  DEVICE_CLASS_MEMDEV,
  //
  // Drawing functions
  //
  _DrawBitmap   ,
  NULL          ,
  NULL          ,
  _FillRect     ,
  NULL          ,
  _SetPixelIndex,
  NULL          ,
  //
  // Set origin
  //
  NULL          ,
  //
  // Request information
  //
  NULL          ,
  NULL          ,
  NULL          ,
  _GetRect      ,
};

/*********************************************************************
*
*       Static code, common
*
**********************************************************************
*/
/*********************************************************************
*
*       _DeviceCreate
*
* Purpose:
*   Creates a new device and selects it for drawing operations
*/
static GUI_DEVICE * _DeviceCreate(int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;
  GUI_DEVICE * pDevice;
  GUI_DEVICE * pDeviceCurrent;
  const LCD_API_COLOR_CONV * pColorConvAPI;

  pDeviceCurrent = GUI_DEVICE__GetpDriver(GUI_Context.SelLayer);
  pColorConvAPI = pDeviceCurrent->pColorConvAPI;
  //
  // Create device object
  //
  pDevice = GUI_DEVICE_CreateAndLink(&_DeviceAPI, pColorConvAPI, 0, GUI_Context.SelLayer);
  if (pDevice) {
    //
    // Create context containing private data
    //
    pDevice->u.hContext = GUI_ALLOC_AllocZero(sizeof(DRIVER_CONTEXT) + sizeof(LCD_PIXELINDEX) * xSize * ySize);

    if (pDevice->u.hContext) {
      pContext = (DRIVER_CONTEXT *)GUI_LOCK_H(pDevice->u.hContext); {
        pContext->xSize = xSize;
      } GUI_UNLOCK_H(pContext);
    } else {
      GUI_DEVICE_Delete(pDevice); // Delete on error
    }
  }
  return pDevice;
}

/*********************************************************************
*
*       _DeviceDelete
*
* Purpose:
*   Deletes the device and restores the previous settings
*/
static void _DeviceDelete(GUI_DEVICE * pDevice) {
  //
  // Remove from device chain
  //
  GUI_DEVICE_Unlink(pDevice);
  //
  // Free payload data
  //
  GUI_ALLOC_Free(pDevice->u.hContext);
  //
  // Delete device
  //
  GUI_DEVICE_Delete(pDevice);
}

/*********************************************************************
*
*       _GetBlanks
*
* Purpose:
*   Returns the leading or trailing blank columns of the given character.
*
* Parameters:
*   c        - Character
*   Trailing - 1 for trailing columns, 1 for leading columns
*
* Return value:
*   >= 0 - Number of blank columns
*   -1   - Error
*/
static int _GetBlanks(U16 c, unsigned Trailing) {
  GUI_DEVICE * pDevice;
  int xSize, ySize, i, j, r, DispPosX, DispPosY;
  LCD_PIXELINDEX BkColorIndex;
  LCD_PIXELINDEX * pData;
  LCD_PIXELINDEX * pDataOld;
  DRIVER_CONTEXT * pContext;

  GUI_LOCK();
  r = -1;
  //
  // Calculate required dimension of device
  //
  xSize = GUI_GetCharDistX(c);
  ySize = GUI_GetFontSizeY();
  //
  // Create and select device
  //
  pDevice = _DeviceCreate(xSize, ySize);
  if (pDevice) {
    DispPosX = GUI_Context.DispPosX;
    DispPosY = GUI_Context.DispPosY;
    pContext = (DRIVER_CONTEXT *)GUI_LOCK_H(pDevice->u.hContext); {
      //
      // Get the data pointer to the device
      //
      pData = (LCD_PIXELINDEX *)(pContext + 1);
      if (pData) {
        //
        // Draw character (into device)
        //
        GUI_DispCharAt(c, 0, 0);
        BkColorIndex = GUI_GetBkColorIndex();
        if (Trailing) {
          pData += xSize - 1;
        }
        r = -1;
        //
        // Iterate over columns
        //
        for (i = 0; i < xSize; i++) {
          pDataOld = pData;
          //
          // Iterate over rows
          //
          for (j = 0; j < ySize; j++) {
            if (*pData != BkColorIndex) {
              //
              // If one pixel has been found, remember column and break
              //
              r = i;
              break;
            }
            pData += xSize;
          }
          if (r >= 0) {
            break;
          }
          pData = pDataOld;
          if (Trailing) {
            pData--;
          } else {
            pData++;
          }
        }
      }
      _DeviceDelete(pDevice);
    } GUI_UNLOCK_H(pContext);
    GUI_Context.DispPosX = DispPosX;
    GUI_Context.DispPosY = DispPosY;
  }
  GUI_UNLOCK();
  return r;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_GetLeadingBlankCols
*
* Purpose:
*   Returns the leading blank columns of the given character.
*
* Parameters:
*   c - Character
*
* Return value:
*   >= 0 - Number of blank columns
*   -1   - Error
*/
int GUI_GetLeadingBlankCols(U16 c) {
  return _GetBlanks(c, 0);
}

/*********************************************************************
*
*       GUI_GetTrailingBlankCols
*
* Purpose:
*   Returns the leading blank columns of the given character.
*
* Parameters:
*   c - Character
*
* Return value:
*   >= 0 - Number of blank columns
*   -1   - Error
*/
int GUI_GetTrailingBlankCols(U16 c) {
  return _GetBlanks(c, 1);
}

/*************************** End of file ****************************/
