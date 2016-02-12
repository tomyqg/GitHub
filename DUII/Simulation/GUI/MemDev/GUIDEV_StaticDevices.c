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
File        : GUIDEV_StaticDevices.c
Purpose     : Manages window drawing with memory devices
---------------------------END-OF-HEADER------------------------------
*/

#include <string.h>
#include <stddef.h>

#include "GUI_Private.h"

#if (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT)

#include "WM_Intern.h"

#if (WM_SUPPORT_STATIC_MEMDEV)

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static U8 _FillByte = 0xFF;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetDistToNextAlpha
*/
static int _GetDistToNextAlpha(U32 * pSrc, int NumPixels) {
  int r = 0;

  do {
    if (((*pSrc++) >> 24) == 0xff) {
      break;
    }
    r++;
  } while (--NumPixels);
  return r;
}

/*********************************************************************
*
*       _CopyContent
*
* Purpose:
*   Copies content of source device into destination device with taking
*   account of different sizes and positions.
*/
static void _CopyContent(GUI_MEMDEV_Handle hSrc, GUI_MEMDEV_Handle hDst) {
  int x0s, y0s, x0d, y0d, x1s, y1s, x1d, y1d;
  int xRem, yRem;
  int xSizeSrc, ySizeSrc, xSizeDst, ySizeDst;
  int DistToNextAlpha, PixelPerLine;
  U32 * pSrc;
  U32 * pDst;
  U32 * pLineSrc;
  U32 * pLineDst;

  //
  // Get properties from source device
  //
  xSizeSrc = GUI_MEMDEV_GetXSize(hSrc);
  ySizeSrc = GUI_MEMDEV_GetYSize(hSrc);
  pSrc     = (U32 *)GUI_MEMDEV_GetDataPtr(hSrc);
  x0s      = GUI_MEMDEV_GetXPos(hSrc);
  y0s      = GUI_MEMDEV_GetYPos(hSrc);
  x1s      = x0s + xSizeSrc - 1;
  y1s      = y0s + ySizeSrc - 1;
  //
  // Get properties from destination device
  //
  xSizeDst = GUI_MEMDEV_GetXSize(hDst);
  ySizeDst = GUI_MEMDEV_GetYSize(hDst);
  pDst     = (U32 *)GUI_MEMDEV_GetDataPtr(hDst);
  x0d      = GUI_MEMDEV_GetXPos(hDst);
  y0d      = GUI_MEMDEV_GetYPos(hDst);
  x1d      = x0d + xSizeDst - 1;
  y1d      = y0d + ySizeDst - 1;
  //
  // Check if something is to do
  //
  if ((x0s > x1d) || (x1s < x0d) || (y0s > y1d) || (y1s < y0d)) {
    return;
  }
  //
  // Adjust source pointer
  //
  if (x0d > x0s) {
    pSrc += x0d - x0s;
  }
  if (y0d > y0s) {
    pSrc += (y0d - y0s) * xSizeSrc;
  }
  //
  // Adjust destination pointer
  //
  if (x0s > x0d) {
    pDst += x0s - x0d;
  }
  if (y0s > y0d) {
    pDst += (y0s - y0d) * xSizeDst;
  }
  //
  // Calculate remaining size
  //
  if        ((x0d >= x0s) && (x1d >= x1s)) {
    xRem = x1s - x0d + 1;
  } else if ((x0d >= x0s) && (x1d <= x1s)) {
    xRem = xSizeDst;
  } else if ((x0d <= x0s) && (x1d >= x1s)) {
    xRem = xSizeSrc;
  } else if ((x0d <= x0s) && (x1d <= x1s)) {
    xRem = x1d - x0s + 1;
  } else {
    return; // Never reach
  }
  if        ((y0d >= y0s) && (y1d >= y1s)) {
    yRem = y1s - y0d + 1;
  } else if ((y0d >= y0s) && (y1d <= y1s)) {
    yRem = ySizeDst;
  } else if ((y0d <= y0s) && (y1d >= y1s)) {
    yRem = ySizeSrc;
  } else if ((y0d <= y0s) && (y1d <= y1s)) {
    yRem = y1d - y0s + 1;
  } else {
    return; // Never reach
  }
  PixelPerLine = xRem;
  do {
    xRem     = PixelPerLine;
    pLineSrc = pSrc;
    pLineDst = pDst;
    do {
      while (((DistToNextAlpha = _GetDistToNextAlpha(pLineSrc, xRem)) == 0) && xRem) {
        pLineSrc++;
        pLineDst++;
        xRem--;
      }
      if (xRem) {
        GUI_MEMCPY(pLineDst, pLineSrc, DistToNextAlpha << 2);
        pLineSrc += DistToNextAlpha;
        pLineDst += DistToNextAlpha;
        xRem -= DistToNextAlpha;
      }
    } while (xRem);
    pDst += xSizeDst;
    pSrc += xSizeSrc;
  } while (--yRem);
}

/*********************************************************************
*
*       _CreateStaticDevice
*/
static GUI_MEMDEV_Handle _CreateStaticDevice(WM_HWIN hWin, WM_Obj * pWin) {
  U8                * pMem;
  GUI_MEMDEV        * pDev;
  GUI_MEMDEV_Handle   hMem, hMemOld;
  int                 x0, y0, x1, y1, xSizeReq, ySizeReq, xPosMem, yPosMem, xSizeMax, ySizeMax;
  WM_HWIN             hDesktop;

  //
  // Get the required size of the memory device
  //
  x0       = pWin->Rect.x0;
  y0       = pWin->Rect.y0;
  x1       = pWin->Rect.x1;
  y1       = pWin->Rect.y1;
  xSizeReq = x1 - x0 + 1;
  ySizeReq = y1 - y0 + 1;
  //
  // Check if there is something to do
  //
  if ((xSizeReq <= 0) || (ySizeReq <= 0)) {
    return 0; // Nothing to do ...
  }
  //
  // Check if there is already a memory device of the required size and format
  //
  hMem = pWin->hMem;
  if (hMem) {
    pDev = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
    xPosMem  = pDev->x0;
    yPosMem  = pDev->y0;
    GUI_UNLOCK_H(pDev);
    if ((xPosMem != x0) || (yPosMem != y0)) {
      GUI_MEMDEV_SetOrg(hMem, x0, y0);
    }
  }
  //
  // Create memory device (if required)
  //
  if (hMem == 0) {
    //
    // !!! Check if the required size is within the legal range !!!
    //
    // Please note that the default size of the desktop windows is 4095 x 4095.
    // When using static memory devices this does not work because it waste too
    // much memory. So the size of the desktop windows will be clipped at the
    // border of the virtual display.
    //
    hDesktop = WM_GetDesktopWindow();
    if (hWin == hDesktop) {
      xSizeMax = LCD_GetVXSize();
      ySizeMax = LCD_GetVYSize();
      if ((xSizeReq > xSizeMax) || (ySizeReq > ySizeMax)) {
        if (xSizeReq > xSizeMax) {
          xSizeReq = xSizeMax;
        }
        if (ySizeReq > ySizeMax) {
          ySizeReq = ySizeMax;
        }
        WM_SetSize(hWin, xSizeReq, ySizeReq);
      }
    }
    //
    // Create device
    //
    hMem = GUI_MEMDEV__CreateFixed(x0, y0, xSizeReq, ySizeReq, GUI_MEMDEV_NOTRANS, GUI_MEMDEV_APILIST_32, GUI_COLOR_CONV_8888);
    //
    // Recalculate pointer
    //
    pWin = WM_H2P(hWin);
    //
    // Save handle of memory device
    //
    pWin->hMem = hMem;
    if (!hMem) {
      GUI_DEBUG_ERROROUT("Not enough memory to create static memory device!");
      //
      // Do the drawing operation without a memory device ...
      //
      WM__Paint1(hWin, pWin);
      //_pfPaint1Func(hWin, pWin);
      //
      // ... and return
      //
      return 0; // Not enough memory !
    } else {
      //
      // Set flag to tell WM that the device has to be deleted in case of invalidation
      //
      pWin->Status |= WM_CF_STATIC;
      //
      // Get device pointer
      //
      pDev = (GUI_MEMDEV *)GUI_LOCK_H(hMem);
      GUI_UNLOCK_H(pDev);
      //
      // Here it is important to suppress mixing up colors with the background so that the alpha values will be written
      // into the memory device. Mixing up colors will be done when drawing the device.
      //
      hMemOld = GUI_MEMDEV_Select(hMem);
      GUI__AlphaSuppressMixing(1);
      //
      // Make sure all pixels are transparent after creating the device
      //
      pMem = (U8 *)GUI_MEMDEV_GetDataPtr(hMem);
      GUI_MEMSET(pMem, _FillByte, (xSizeReq * ySizeReq) << 2);
      //
      // Draw content
      //
      WM__Paint1(hWin, pWin); // Note: After this function call, the pointer pWin can become invalid!
      //
      // Restore previous mode
      //
      GUI__AlphaSuppressMixing(0);
      GUI_MEMDEV_Select(hMemOld);
    }
  }
  return hMem;
}

/*********************************************************************
*
*       _AddContent
*/
static void _AddContent(WM_HWIN hWin, GUI_MEMDEV_Handle * phMem) {
  int xSize, ySize;
  GUI_MEMDEV_Handle hMemDst, hMemSrc;
  GUI_RECT Rect;
  U8 * pSrc;
  U8 * pDst;

  hMemDst = *phMem;
  if (hMemDst == 0) {
    WM_GetWindowRectEx(hWin, &Rect);
    xSize = (Rect.x1 - Rect.x0) + 1;
    ySize = (Rect.y1 - Rect.y0) + 1;
    hMemDst = GUI_MEMDEV_CreateFixed(Rect.x0, Rect.y0, xSize, ySize, 0, &GUI_MEMDEV_DEVICE_32, GUICC_8888);
    if (hMemDst == 0) {
      return;
    }
    *phMem = hMemDst;
    hMemSrc = GUI_MEMDEV_GetStaticDevice(hWin);
    if (hMemSrc) {
      pSrc = (U8 *)GUI_MEMDEV_GetDataPtr(hMemSrc);
      pDst = (U8 *)GUI_MEMDEV_GetDataPtr(hMemDst);
      GUI_MEMCPY(pDst, pSrc, (xSize * ySize) << 2);
    }
  } else {
    hMemSrc = GUI_MEMDEV_GetStaticDevice(hWin);
    if (hMemSrc) {
      _CopyContent(hMemSrc, hMemDst);
    }
  }
}

/*********************************************************************
*
*       _GetWindowDevice
*/
static void _GetWindowDevice(WM_HWIN hWin, GUI_MEMDEV_Handle * phMem) {
  WM_HWIN hChild;
  WM_Obj * pWin;
  WM_Obj * pChild;
  GUI_RECT Rect;

  pWin = WM_H2P(hWin);
  if ((pWin->Status & WM_SF_ISVIS) && (pWin->cb)) {
    Rect = pWin->Rect;
    if (WM__ClipAtParentBorders(&Rect, hWin)) {
      _AddContent(hWin, phMem);
      for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
        _GetWindowDevice(hChild, phMem);
        pChild = WM_H2P(hChild);
      }
    }
  }
}

/*********************************************************************
*
*       _CreateStatic
*/
static void _CreateStatic(WM_HWIN hWin) {
  WM_HWIN hChild;
  WM_Obj * pWin;
  WM_Obj * pChild;
  GUI_RECT Rect;

  pWin = WM_H2P(hWin);
  if ((pWin->Status & WM_SF_ISVIS) && (pWin->cb)) {
    Rect = pWin->Rect;
    if (WM__ClipAtParentBorders(&Rect, hWin)) {
      WM_SetDefault();
      WM_SelectWindow(hWin);
      GUI_GotoXY(0,0);
      _CreateStaticDevice(hWin, pWin);
      _FillByte = 0xFF;
      for (hChild = pWin->hFirstChild; hChild; hChild = pChild->hNext) {
        _CreateStatic(hChild);
        pChild = WM_H2P(hChild);
      }
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_MEMDEV_Paint1Static
*/
void GUI_MEMDEV_Paint1Static(WM_HWIN hWin, WM_Obj * pWin) {
  GUI_MEMDEV_Handle hMem;

  //
  // Create static device...
  //
  hMem = _CreateStaticDevice(hWin, pWin);
  //
  // ...and write it into currently selected device
  //
  if (hMem) {
    GUI_MEMDEV_Write(hMem);
  }
}

/*********************************************************************
*
*       GUI_MEMDEV_CreateStatic
*/
void GUI_MEMDEV_CreateStatic(WM_HWIN hWin) {
  _FillByte = 0xFF;
  _CreateStatic(hWin);
}

/*********************************************************************
*
*       GUI_MEMDEV_GetWindowDevice
*/
GUI_MEMDEV_Handle GUI_MEMDEV_GetWindowDevice(WM_HWIN hWin) {
  GUI_MEMDEV_Handle hMem;
  
  hMem = 0;
  _GetWindowDevice(hWin, &hMem);
  return hMem;
}

/*********************************************************************
*
*       GUI_MEMDEV_GetStaticDevice
*/
GUI_MEMDEV_Handle GUI_MEMDEV_GetStaticDevice(WM_HWIN hWin) {
  GUI_MEMDEV_Handle hMem;
  WM_Obj * pWin;

  GUI_LOCK();
  hMem = 0;
  if (hWin) {
    pWin = WM_H2P(hWin);
    hMem = pWin->hMem;
  }
  GUI_UNLOCK();
  return hMem;
}

#endif /* WM_SUPPORT_STATIC_MEMDEV */

void GUIDEV_StaticDevices_C(void);
void GUIDEV_StaticDevices_C(void) {} /* avoid empty object files */

#else

void GUIDEV_StaticDevices_C(void);
void GUIDEV_StaticDevices_C(void) {} /* avoid empty object files */

#endif /* (GUI_SUPPORT_MEMDEV && GUI_WINSUPPORT) */

/*************************** end of file ****************************/
