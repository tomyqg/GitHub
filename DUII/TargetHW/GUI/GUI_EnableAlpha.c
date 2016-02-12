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
File        : GUI_EnableAlpha.c
Purpose     : Enables automatic alphablending
----------------------------------------------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static const LCD_SET_COLOR_API * _pSetColorAPI;

static U32          _Color;
static U32          _BkColor;
static U8         * _pBuffer;
static GUI_DEVICE * _pDevice;
static int          _Linked;
static int          _Buffersize;
static U32          _UserAlpha;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SwapIndices
*/
static void _SwapIndices(void) {
  LCD_PIXELINDEX Temp;
  //
  // Swap GUI indices
  //
  Temp = LCD__GetColorIndex();
  LCD__SetColorIndex(LCD__GetBkColorIndex());
  LCD__SetBkColorIndex(Temp);
}

/*********************************************************************
*
*       _SwapColors
*/
static void _SwapColors(void) {
  GUI_COLOR      Temp;
  //
  // Swap internal colors
  //
  Temp     = _Color;
  _Color   = _BkColor;
  _BkColor = Temp;
}

/*********************************************************************
*
*       _ManageAlpha
*/
static void _ManageAlpha(void) {
  static U32 AlphaOld;
  U32 Alpha;
  U32 BkAlpha;

  Alpha   = _Color   >> 24;
  BkAlpha = _BkColor >> 24;
  if (Alpha || BkAlpha) {
    if (_Linked == 0) {
      GUI_DEVICE_Link(_pDevice);
      _Linked = 1;
    }
  } else {
    if (_Linked == 1) {
      GUI_DEVICE_Unlink(_pDevice);
      _Linked = 0;
    }
  }
  if (_UserAlpha) {
    Alpha += ((255 - Alpha) * _UserAlpha) / 255;
  }
  if (Alpha != AlphaOld) {
    GUI_SetAlpha(Alpha);
    AlphaOld = Alpha;
  }
}

/*********************************************************************
*
*       _DrawBitmap
*
* Purpose:
*   This is the only function which should do something other than
*   routing. It has to make sure that 1bpp bitmaps are drawn right.
*   Drawing these bitmaps is the only operation which requires the
*   background color in the display driver routines.
*   
*   The trick of drawing text with the right alpha blending for foreground
*   and background pixels is the following:
*
*   First set the mode to 'LCD_DRAWMODE_TRANS' and draw the text pixels.
*   Then swap the background colors and color indices for the driver and
*   set the alpha blending values according to the background color.
*   Then invert and draw the bitmap pixels line by line. The effect is
*   that only the background pixels will be drawn during this second step.
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans) {
  GUI_DEVICE * pDeviceNext;
  int i, MaxBytes;
  U8 * pSrc;
  U8 * pDst;

  if ((BitsPerPixel == 1) &&                        // Only 1bpp bitmaps
      (pTrans == LCD_pBkColorIndex) &&              // Only device dependent bitmaps (text)
      (GUI_Context.DrawMode == LCD_DRAWMODE_NORMAL) // Only non transparent text
     ) {
    //
    // Draw foreground pixels with the required color
    //
    GUI_Context.DrawMode = LCD_DRAWMODE_TRANS;
    pDeviceNext = pDevice->pNext;
    pDeviceNext->pDeviceAPI->pfDrawBitmap(pDeviceNext, 
                                          x0, y0, 
                                          xSize, ySize, 
                                          BitsPerPixel, BytesPerLine, pData, Diff, pTrans);
    //
    // Prepare alpha blending for background pixels
    //
    _SwapColors();
    _SwapIndices();
    _ManageAlpha();
    pDevice = pDevice->pNext;
    //
    // Draw foreground pixels with the required color line by line
    //
    if (_pBuffer) {
      if (BytesPerLine > _Buffersize) {
        MaxBytes = _Buffersize;
        xSize    = _Buffersize << 3;
      } else {
        MaxBytes = BytesPerLine;
      }
      do {
        i    = MaxBytes;
        pSrc = (U8 *)pData;
        pDst = _pBuffer;
        //
        // Invert pixel data of 1bpp bitmap
        //
        do {
          *pDst++ = (*pSrc++) ^ 0xff;
        } while (--i);
        //
        // Draw one line of bitmap
        //
        pDevice->pDeviceAPI->pfDrawBitmap(pDevice, 
                                          x0, y0++, 
                                          xSize, 1, 
                                          BitsPerPixel, BytesPerLine, _pBuffer, Diff, pTrans);
        //
        // Increment data pointer
        //
        pData += BytesPerLine;
      } while (--ySize);
    }
    //
    // Reset alpha settings
    //
    _SwapColors();
    _SwapIndices();
    _ManageAlpha();
    GUI_Context.DrawMode = LCD_DRAWMODE_NORMAL;
  } else {
    //
    // Draw bitmaps other than text by passing it to the next device
    //
    pDevice = pDevice->pNext;
    if (pDevice) {
      pDevice->pDeviceAPI->pfDrawBitmap(pDevice, 
                                        x0, y0, 
                                        xSize, ySize, 
                                        BitsPerPixel, BytesPerLine, pData, Diff, pTrans);
    }
  }
}

/*********************************************************************
*
*       _DrawHLine
*
* Purpose:
*   Routing to the next device...
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfDrawHLine(pDevice, x0, y,  x1);
  }
}

/*********************************************************************
*
*       _DrawVLine
*
* Purpose:
*   Routing to the next device...
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfDrawVLine(pDevice, x, y0,  y1);
  }
}

/*********************************************************************
*
*       _FillRect
*
* Purpose:
*   Routing to the next device...
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfFillRect(pDevice, x0, y0, x1, y1);
  }
}

/*********************************************************************
*
*       _GetPixelIndex
*
* Purpose:
*   Routing to the next device...
*/
static unsigned _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y)  {
  pDevice = pDevice->pNext;
  if (pDevice) {
    return pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
  }
  return 0;
}

/*********************************************************************
*
*       _SetPixelIndex
*
* Purpose:
*   Routing to the next device...
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int ColorIndex) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, ColorIndex);
  }
}

/*********************************************************************
*
*       _XorPixel
*
* Purpose:
*   Routing to the next device...
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfXorPixel(pDevice, x, y);
  }
}

/*********************************************************************
*
*       _GetDevData
*
* Purpose:
*   Routing to the next device...
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    return pDevice->pDeviceAPI->pfGetDevData(pDevice, Index);
  }
  return NULL;
}

/*********************************************************************
*
*       _SetOrg
*
* Purpose:
*   Routing to the next device...
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
  }
}

/*********************************************************************
*
*       _GetDevFunc
*
* Purpose:
*   Routing to the next device...
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  *ppDevice = (*ppDevice)->pNext;
  if (*ppDevice) {
    return (*ppDevice)->pDeviceAPI->pfGetDevFunc(ppDevice, Index);
  }
  return NULL;
}

/*********************************************************************
*
*       _GetDevProp
*
* Purpose:
*   Routing to the next device...
*/
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    return pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
  }
  return 0;
}

/*********************************************************************
*
*       _GetRect
*
* Purpose:
*   Routing to the next device...
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfGetRect(pDevice, pRect);
  }
}

/*********************************************************************
*
*       Static data, API table GUI_DEVICE_API _ALPHA_Device_API
*
**********************************************************************
*/
static const GUI_DEVICE_API _ALPHA_Device_API = {
  //
  // Data
  //
  DEVICE_CLASS_AUTOALPHA,
  //
  // Drawing functions
  //
  _DrawBitmap   ,
  _DrawHLine    ,
  _DrawVLine    ,
  _FillRect     ,
  _GetPixelIndex,
  _SetPixelIndex,
  _XorPixel     ,
  //
  // Set origin
  //
  _SetOrg       ,
  //
  // Request information
  //
  _GetDevFunc   ,
  _GetDevProp   ,
  _GetDevData,
  _GetRect      ,
};

/*********************************************************************
*
*       _SetBkColor
*/
static void _SetBkColor(GUI_COLOR Color) {
  //
  // Get the right color
  //
  if (GUI_Context.DrawMode & LCD_DRAWMODE_REV) {
    _Color = Color;
  } else {
    _BkColor = Color;
  }
  //
  // Manage alpha blending in dependence of the current color(s)
  //
  _ManageAlpha();
  //
  // Call the original function
  //
  _pSetColorAPI->pfSetBkColor(Color & 0xFFFFFF);
}

/*********************************************************************
*
*       _SetColor
*/
static void _SetColor(GUI_COLOR Color) {
  //
  // Get the right color
  //
  if (GUI_Context.DrawMode & LCD_DRAWMODE_REV) {
    _BkColor = Color;
  } else {
    _Color = Color;
  }
  //
  // Manage alpha blending in dependence of the current color(s)
  //
  _ManageAlpha();
  //
  // Call the original function
  //
  _pSetColorAPI->pfSetColor(Color & 0xFFFFFF);
}

/*********************************************************************
*
*       _SetDrawMode
*/
static LCD_DRAWMODE _SetDrawMode(LCD_DRAWMODE dm) {
  if ((GUI_Context.DrawMode ^ dm) & LCD_DRAWMODE_REV) {
    _SwapColors();
  }
  //
  // Manage alpha blending in dependence of the current color(s)
  //
  _ManageAlpha();
  //
  // Call the original function
  //
  return _pSetColorAPI->pfSetDrawMode(dm);
}

/*********************************************************************
*
*       Static data, API table _SetColorAPI
*
**********************************************************************
*/
static const LCD_SET_COLOR_API _SetColorAPI = {
  _SetColor,
  _SetBkColor,
  _SetDrawMode,
};

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SetUserAlpha
*/
U32 GUI_SetUserAlpha(GUI_ALPHA_STATE * pAlphaState, U32 UserAlpha) {
  pAlphaState->UserAlpha = _UserAlpha;
  _UserAlpha = UserAlpha;
  return pAlphaState->UserAlpha;
}

/*********************************************************************
*
*       GUI_RestoreUserAlpha
*/
U32 GUI_RestoreUserAlpha(GUI_ALPHA_STATE * pAlphaState) {
  U32 OldUserAlpha;

  OldUserAlpha = _UserAlpha;
  _UserAlpha = pAlphaState->UserAlpha;
  return OldUserAlpha;
}

/*********************************************************************
*
*       GUI_EnableAlpha
*/
unsigned GUI_EnableAlpha(unsigned OnOff) {
  unsigned OldState;
  unsigned vxSizeMax;

  if (_pDevice == NULL) {
    //
    // Make sure that buffers are allocated at the beginning
    //
    GUI_SetAlpha(1);
    GUI_SetAlpha(0);
    //
    // Create device
    //
    _pDevice = GUI_DEVICE_Create(&_ALPHA_Device_API, NULL, 0, 0);
    //
    // Get buffer for at least one line of bitmap data
    //
    vxSizeMax   = LCD_GetVXSizeMax();
    _Buffersize = (vxSizeMax + 7) >> 3;
    //
    // Allocate buffer for 1bpp bitmaps
    //
    _pBuffer = (U8 *)GUI_ALLOC_GetFixedBlock(_Buffersize);
  }
  if (OnOff) {
    if (_pSetColorAPI == NULL) {
      OldState = 0;
      _pSetColorAPI     = LCD__pSetColorAPI;
      LCD__pSetColorAPI = &_SetColorAPI;
    } else {
      OldState = 1;
    }
  } else {
    if (_pSetColorAPI != NULL) {
      OldState = 1;
      LCD__pSetColorAPI = _pSetColorAPI;
      _pSetColorAPI     = NULL;
    } else {
      OldState = 0;
    }
  }
  return OldState;
}

/*************************** End of file ****************************/
