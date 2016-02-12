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
File        : GUI_SetOrientation.c
Purpose     : Support for changing the display orientation at run time
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "GUI_SetOrientation.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _Log2Phys_xxx
*/
static void _Log2Phys_X  (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = pContext->vxSize - 1 - x;
  *py_phys = y;
}
static void _Log2Phys_Y  (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = x;
  *py_phys = pContext->vySize - 1 - y;
}
static void _Log2Phys_XY (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = pContext->vxSize - 1 - x;
  *py_phys = pContext->vySize - 1 - y;
}
static void _Log2Phys_S  (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  GUI_USE_PARA(pContext);
  *px_phys = y;
  *py_phys = x;
}
static void _Log2Phys_SX (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = pContext->vxSize - 1 - y;
  *py_phys = x;
}
static void _Log2Phys_SY (DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = y;
  *py_phys = pContext->vySize - 1 - x;
}
static void _Log2Phys_SXY(DRIVER_CONTEXT * pContext, int x, int y, int * px_phys, int * py_phys) {
  *px_phys = pContext->vxSize - 1 - y;
  *py_phys = pContext->vySize - 1 - x;
}

/*********************************************************************
*
*       Static code: API functions for drawing operations
*
* Purpose:
*   The following functions call the real drawing functions depending
*   on the cache settings.
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfDrawBitmap(pDevice, x0, y0, xSize, ySize, BitsPerPixel, BytesPerLine, pData, Diff, pTrans);
}

/*********************************************************************
*
*       _GetPixelIndex
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  return pContext->pDrawingAPI->pfGetPixelIndex(pDevice, x, y);
}

/*********************************************************************
*
*       _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int PixelIndex) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfSetPixelIndex(pDevice, x, y, PixelIndex);
}

/*********************************************************************
*
*       _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfXorPixel(pDevice, x, y);
}

/*********************************************************************
*
*       _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y, int x1) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfDrawHLine(pDevice, x0, y, x1);
}

/*********************************************************************
*
*       _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x, int y0, int y1) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfDrawVLine(pDevice, x, y0, y1);
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pContext->pDrawingAPI->pfFillRect(pDevice, x0, y0, x1, y1);
}

/*********************************************************************
*
*       Static code: Rest of API functions, only routing is required
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetOrg
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
}

/*********************************************************************
*
*       _GetDevFunc
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  switch (Index) {
  case LCD_DEVFUNC_READRECT:
    return NULL;
  }
  *ppDevice = (*ppDevice)->pNext;
  return (*ppDevice)->pDeviceAPI->pfGetDevFunc(ppDevice, Index);
}

/*********************************************************************
*
*       _GetDevProp
*/
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  switch (Index) {
  case LCD_DEVCAP_XSIZE:
    return (pContext->Orientation & GUI_SWAP_XY)  ? pContext->ySize : pContext->xSize;
  case LCD_DEVCAP_YSIZE:
    return (pContext->Orientation & GUI_SWAP_XY)  ? pContext->xSize : pContext->ySize;
  case LCD_DEVCAP_VXSIZE:
    return (pContext->Orientation & GUI_SWAP_XY)  ? pContext->vySize : pContext->vxSize;
  case LCD_DEVCAP_VYSIZE:
    return (pContext->Orientation & GUI_SWAP_XY)  ? pContext->vxSize : pContext->vySize;
  case LCD_DEVCAP_MIRROR_X:
    return (pContext->Orientation & GUI_MIRROR_X) ? 1 : 0;
  case LCD_DEVCAP_MIRROR_Y:
    return (pContext->Orientation & GUI_MIRROR_Y) ? 1 : 0;
  case LCD_DEVCAP_SWAP_XY:
    return (pContext->Orientation & GUI_SWAP_XY)  ? 1 : 0;
  }
  pDevice = pDevice->pNext;
  return pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  return pDevice->pDeviceAPI->pfGetDevData(pDevice, Index);
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  DRIVER_CONTEXT * pContext;
  GUI_RECT Rect;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pDevice = pDevice->pNext;
  pDevice->pDeviceAPI->pfGetRect(pDevice, &Rect);
  pRect->x0 = pRect->y0 = 0;
  if (pContext->Orientation & GUI_SWAP_XY) {
    pRect->x1 = Rect.y1;
    pRect->y1 = Rect.x1;
  } else {
    pRect->x1 = Rect.x1;
    pRect->y1 = Rect.y1;
  }
}

/*********************************************************************
*
*       Public data
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
const GUI_DEVICE_API GUIDRV_Rotate_API = {
  //
  // Data
  //
  DEVICE_CLASS_DRIVER,
  //
  // Drawing functions
  //
  _DrawBitmap,
  _DrawHLine,
  _DrawVLine,
  _FillRect,
  _GetPixelIndex,
  _SetPixelIndex,
  _XorPixel,
  //
  // Set origin
  //
  _SetOrg,
  //
  // Request information
  //
  _GetDevFunc,
  _GetDevProp,
  _GetDevData,
  _GetRect,
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SetOrientationExCached
*/
int GUI_SetOrientationExCached(int Orientation, int LayerIndex, const GUI_ORIENTATION_API * pAPI) {
  GUI_DEVICE     * pDevice;  // The 'rotation device'
  GUI_DEVICE     * pDriver;  // The real display driver
  DRIVER_CONTEXT * pContext; // Context of 'rotation device'
  int vxSize, vySize;
  int xSize, ySize;

  //
  // Delete already existing rotation device
  //
  pDevice = GUI_DEVICE__GetpDevice(LayerIndex, DEVICE_CLASS_DRIVER);
  if (pDevice->pDeviceAPI == &GUIDRV_Rotate_API) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (pContext->pData) {
      GUI_ALLOC_FreeFixedBlock(pContext->pData);
    }
    GUI_ALLOC_FreeFixedBlock(pContext);
    GUI_DEVICE_Unlink(pDevice);
    GUI_DEVICE_Delete(pDevice);
  }
  //
  // Nothing to do if default orientation is required
  //
  if (Orientation == 0) {
    return 1;
  }
  //
  // Get size from current layer
  //
  xSize  = LCD_GetXSizeEx(LayerIndex);
  ySize  = LCD_GetYSizeEx(LayerIndex);
  vxSize = LCD_GetVXSizeEx(LayerIndex);
  vySize = LCD_GetVYSizeEx(LayerIndex);
  //
  // Don't rotate windows driver!
  //
  #ifdef WIN32
  pDevice = GUI_DEVICE__GetpDevice(LayerIndex, DEVICE_CLASS_DRIVER);
  if (pDevice->pDeviceAPI == GUIDRV_WIN32) {
    if (Orientation & GUI_SWAP_XY) {
      LCD_SetSizeEx(LayerIndex, ySize, xSize);
      LCD_SetVSizeEx(LayerIndex, vySize, vxSize);
    }
    return 0;
  }
  #endif
  //
  // Create device
  //
  pDriver = GUI_DEVICE__GetpDriver(LayerIndex);
  pDevice = GUI_DEVICE_Create(&GUIDRV_Rotate_API, pDriver->pColorConvAPI, GUI_DEVICE_STAYONTOP, LayerIndex);
  //
  // Link it into device list
  //
  if (pDevice) {
    GUI_DEVICE_Link(pDevice);
  }
  //
  // Create rotation context
  //
  pDevice->u.pContext = (DRIVER_CONTEXT *)GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
  if (pDevice->u.pContext == NULL) {
    GUI_DEBUG_ERROROUT(__FILE__": Memory allocation failed");
    return 1; // Error
  }
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  // Initialize context
  //
  pContext->xSize         = xSize;
  pContext->ySize         = ySize;
  pContext->vxSize        = vxSize;
  pContext->vySize        = vySize;
  pContext->Orientation   = Orientation;
  pContext->pDrawingAPI   = pAPI;
  //
  // Set pointer to the right LOG2PHYS-routine and pixel offset
  //
  switch (pContext->Orientation) {
  case GUI_MIRROR_X:
    pContext->pfLog2Phys  = _Log2Phys_X;
    pContext->PixelOffset = -1;
    break;
  case GUI_MIRROR_Y:
    pContext->pfLog2Phys  = _Log2Phys_Y;
    pContext->PixelOffset = 1;
    break;
  case GUI_MIRROR_Y | GUI_MIRROR_X:
    pContext->pfLog2Phys  = _Log2Phys_XY;
    pContext->PixelOffset = -1;
    break;
  case GUI_SWAP_XY:
    pContext->pfLog2Phys  = _Log2Phys_S;
    pContext->PixelOffset = pContext->vxSize;
    break;
  case GUI_SWAP_XY | GUI_MIRROR_X:
    pContext->pfLog2Phys  = _Log2Phys_SX;
    pContext->PixelOffset = pContext->vxSize;
    break;
  case GUI_SWAP_XY | GUI_MIRROR_Y:
    pContext->pfLog2Phys  = _Log2Phys_SY;
    pContext->PixelOffset = -pContext->vxSize;
    break;
  case GUI_SWAP_XY | GUI_MIRROR_Y | GUI_MIRROR_X:
    pContext->pfLog2Phys  = _Log2Phys_SXY;
    pContext->PixelOffset = -pContext->vxSize;
    break;
  }
  //
  // Allocate cache if required
  //
  if (pAPI->BytesPerPixel) {
    pContext->pData = (U8 *)GUI_ALLOC_GetFixedBlock(vxSize * vySize * pAPI->BytesPerPixel);
    if (pContext->pData == NULL) {
      GUI_DEBUG_ERROROUT(__FILE__": Memory allocation failed");
      GUI_ALLOC_FreeFixedBlock(pContext);
      return 1; // Error
    }
    pContext->BytesPerLine = pAPI->BytesPerPixel * vxSize;
  }
  return 0;
}

/*********************************************************************
*
*       GUI_SetOrientationEx
*/
int GUI_SetOrientationEx(int Orientation, int LayerIndex) {
  int BitsPerPixel;
  
  BitsPerPixel = LCD_GetBitsPerPixelEx(LayerIndex);
  if        (BitsPerPixel < 8) {
    return GUI_SetOrientationExCached(Orientation, LayerIndex, GUI_ORIENTATION_C0);
  } else if (BitsPerPixel == 8) {
    return GUI_SetOrientationExCached(Orientation, LayerIndex, GUI_ORIENTATION_C8);
  } else if (BitsPerPixel <= 16) {
    return GUI_SetOrientationExCached(Orientation, LayerIndex, GUI_ORIENTATION_C16);
  } else {
    return GUI_SetOrientationExCached(Orientation, LayerIndex, GUI_ORIENTATION_C32);
  }
}

/*********************************************************************
*
*       GUI_SetOrientation
*/
int GUI_SetOrientation(int Orientation) {
  return GUI_SetOrientationEx(Orientation, GUI_Context.SelLayer);
}

/*************************** End of file ****************************/
