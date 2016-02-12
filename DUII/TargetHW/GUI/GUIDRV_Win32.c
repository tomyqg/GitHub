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
File        : GUIDRV_Win32.c
Purpose     : Driver for simulaton under Windows
---------------------------END-OF-HEADER------------------------------
*/

#if defined(WIN32)

#include <windows.h>
#include <memory.h>

#include "LCD_Private.h"              /* include LCDConf.h */
#include "LCD_SIM.h"
#include "GUI_Private.h"
#include "LCD_ConfDefaults.h"

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef struct {
  U32 VRAMAddr;
  int xSize, ySize;
  int vxSize, vySize;
  int vxSizePhys;
  int xPos, yPos;
  int Alpha;
  int IsVisible;
  unsigned NumColors;
  LCD_COLOR aColor[256];
} DRIVER_CONTEXT;

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#ifdef WIN32
  #ifndef ASSERT
    #define ASSERT(Val) \
    if (!(Val)) \
      MessageBox(NULL,"...in file "__FILE__,"Assertion failed...",MB_OK);
  #endif
#endif

#ifdef LCD_ASSERT
  #undef LCD_ASSERT
#endif
#define LCD_ASSERT(v) ASSERT(v)

/*********************************************************************
*
*       Macros for internal use
*/
#ifdef _DEBUG
static int _CheckBound(GUI_DEVICE * pDevice, unsigned int c) {
  DRIVER_CONTEXT * pContext;
  U32 NumColors;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  NumColors = pContext->NumColors ? pContext->NumColors : 0xFFFFFFFF;
  if (c > NumColors) {
    GUI_DEBUG_ERROROUT("LCDWin::SETPIXEL: parameters out of bounds");
    return 1;
  }
  return 0;
}

  #define SETPIXEL(pDevice, x, y, c) \
    if (!_CheckBound(pDevice, c)) { \
      LCDSIM_SetPixelIndex(x, y, c, pDevice->LayerIndex); \
    }
#else
  #define SETPIXEL(pDevice, x, y, c) LCDSIM_SetPixelIndex(x, y, c, pDevice->LayerIndex)
#endif
#define XORPIXEL(pDevice, x, y)    _XOR_Pixel(pDevice, x, y)

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetPixelIndex
*/
static unsigned _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y)  {
  return LCDSIM_GetPixelIndex(x,y, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _XOR_Pixel
*/
static void _XOR_Pixel(GUI_DEVICE * pDevice, int x, int y) {
  LCD_PIXELINDEX PixelIndex;
  LCD_PIXELINDEX IndexMask;
  
  PixelIndex = _GetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
  LCDSIM_SetPixelIndex(x, y, PixelIndex ^ IndexMask, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _XorPixel
*
*  Purpose:
*    Inverts 1 pixel of the display.
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  XORPIXEL(pDevice, x, y);
}

/*********************************************************************
*
*       _SetPixelIndex
*
*  Purpose:
*    Writes 1 pixel into the display.
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int ColorIndex) {
  SETPIXEL(pDevice, x, y, ColorIndex);
}

/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, int x, int y, U8 const*p, int Diff, int xSize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX Index0, Index1, IndexMask;
  int Pixel;
  const LCD_PIXELINDEX aTrans[2] = {0, 1};

  if (!pTrans) {
    pTrans = aTrans;
  }
  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  x+=Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS|LCD_DRAWMODE_XOR)) {
  case 0:    /* Write mode */
    do {
      LCDSIM_SetPixelIndex(x++,y, (*p & (0x80>>Diff)) ? Index1 : Index0, pDevice->LayerIndex);
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xSize);
    break;
  case LCD_DRAWMODE_TRANS:
    do {
  		if (*p & (0x80>>Diff))
        LCDSIM_SetPixelIndex(x,y, Index1, pDevice->LayerIndex);
      x++;
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xSize);
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
  		if (*p & (0x80>>Diff)) {
        Pixel = LCDSIM_GetPixelIndex(x,y, pDevice->LayerIndex);
        LCDSIM_SetPixelIndex(x,y, Pixel ^ IndexMask, pDevice->LayerIndex);
      }
      x++;
			if (++Diff==8) {
        Diff=0;
				p++;
			}
		} while (--xSize);
    break;
	}
}

/*********************************************************************
*
*       _DrawBitLine2BPP
*/
static void _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const * p, int Diff, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          SETPIXEL(pDevice, x++, y, PixelIndex);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          SETPIXEL(pDevice, x++, y, Index);
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            SETPIXEL(pDevice, x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (3 - CurrentPixel) << 1;
          int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
          if (Index) {
            SETPIXEL(pDevice, x, y, Index);
          }
          x++;
          if (++CurrentPixel == 4) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
  }
}

/*********************************************************************
*
*       _DrawBitLine4BPP
*/
static void _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const * p, int Diff, int xSize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels = *p;
  int CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
    case 0:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
          SETPIXEL(pDevice, x++, y, PixelIndex);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          SETPIXEL(pDevice, x++, y, Index);
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
    case LCD_DRAWMODE_TRANS:
      if (pTrans) {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
            SETPIXEL(pDevice, x, y, PixelIndex);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      } else {
        do {
          int Shift = (1 - CurrentPixel) << 2;
          int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
          if (Index) {
            SETPIXEL(pDevice, x, y, Index);
          }
          x++;
          if (++CurrentPixel == 2) {
            CurrentPixel = 0;
            Pixels = *(++p);
          }
		    } while (--xSize);
      }
      break;
  }
}

/*********************************************************************
*
*       _DrawBitLine8BPP
*/
static void _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const*p, int xSize, const LCD_PIXELINDEX*pTrans) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS)==0) {
    if (pTrans) {
      for (;xSize > 0; xSize--,x++,p++) {
        pixel = *p;
        SETPIXEL(pDevice, x, y, *(pTrans+pixel));
      }
    } else {
      for (;xSize > 0; xSize--,x++,p++) {
        SETPIXEL(pDevice, x, y, *p);
      }
    }
  } else {   /* Handle transparent bitmap */
    if (pTrans) {
      for (; xSize > 0; xSize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(pDevice, x+0, y, *(pTrans+pixel));
        }
      }
    } else {
      for (; xSize > 0; xSize--, x++, p++) {
        pixel = *p;
        if (pixel) {
          SETPIXEL(pDevice, x+0, y, pixel);
        }
      }
    }
  }
}

/*********************************************************************
*
*       _DrawBitLine16BPP
*/
static void _DrawBitLine16BPP(GUI_DEVICE * pDevice, int x, int y, U16 const * p, int xSize) {
  LCD_PIXELINDEX pixel;
  if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0) {
    for (;xSize > 0; xSize--,x++,p++) {
      SETPIXEL(pDevice, x, y, *p);
    }
  } else {   /* Handle transparent bitmap */
    for (; xSize > 0; xSize--, x++, p++) {
      pixel = *p;
      if (pixel) {
        SETPIXEL(pDevice, x + 0, y, pixel);
      }
    }
  }
}

/*********************************************************************
*
*       _DrawBitLine32BPP
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const * p, int xSize) {
  for (;xSize > 0; xSize--, x++, p++) {
    SETPIXEL(pDevice, x, y, *p);
  }
}

/*********************************************************************
*
*       _InitOnce
*
* Purpose:
*   Allocates a fixed block for the context of the driver
*
* Return value:
*   0 on success, 1 on error
*/
static int _InitOnce(GUI_DEVICE * pDevice) {
  DRIVER_CONTEXT * pContext;
  U32 IndexMask;
  int BitsPerPixel;

  if (pDevice->u.pContext == NULL) {
    pContext = GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
    pDevice->u.pContext = pContext;
    memset(pContext, 0, sizeof(DRIVER_CONTEXT));
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    BitsPerPixel = LCD__GetBPP(IndexMask);
    if (BitsPerPixel < 24) {
      pContext->NumColors = (1 << BitsPerPixel);
    } else {
      pContext->NumColors = 0;
    }
  }
  return pDevice->u.pContext ? 0 : 1;
}

/*********************************************************************
*
*       _SetPos
*
*  Purpose:
*    Sets the layer position
*/
static void _SetPos(GUI_DEVICE * pDevice, int xPos, int yPos) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->xPos = xPos;
    pContext->yPos = yPos;
  }
  LCDSIM_SetLayerPos(xPos, yPos, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _GetPos
*/
static void _GetPos(GUI_DEVICE * pDevice, int * pxPos, int * pyPos) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    *pxPos = pContext->xPos;
    *pyPos = pContext->yPos;
  }
}

/*********************************************************************
*
*       _SetVis
*
*  Purpose:
*    Sets the layer visibility
*/
static void _SetVis(GUI_DEVICE * pDevice, int OnOff) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->IsVisible = OnOff;
  }
  LCDSIM_SetLayerVis(OnOff, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _SetAlpha
*
*  Purpose:
*    Sets the alpha blending factor
*/
static void _SetAlpha(GUI_DEVICE * pDevice, int Alpha) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->Alpha = Alpha;
  }
  LCDSIM_SetAlpha(Alpha, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _SetVSize
*/
static void _SetVSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->vxSize = xSize;
    pContext->vySize = ySize;
    pContext->vxSizePhys = ySize;
  }
}

/*********************************************************************
*
*       _SetSize
*/
static void _SetSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (pContext->vxSizePhys == 0) {
      pContext->vxSizePhys = ySize;
    }
    pContext->xSize = xSize;
    pContext->ySize = ySize;
  }
}

/*********************************************************************
*
*       _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (;x0 <= x1; x0++) {
      XORPIXEL(pDevice, x0, y);
    }
  } else {
    for (;x0 <= x1; x0++) {
      SETPIXEL(pDevice, x0, y, LCD__GetColorIndex());
    }
  }
}

/*********************************************************************
*
*       _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    while (y0 <= y1) {
      XORPIXEL(pDevice, x, y0);
      y0++;
    }
  } else {
    while (y0 <= y1) {
      SETPIXEL(pDevice, x, y0, LCD__GetColorIndex());
      y0++;
    }
  }
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  for (; y0 <= y1; y0++) {
    _DrawHLine(pDevice, x0, y0, x1);
  }
}

/*********************************************************************
*
*       _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8* pData, int Diff,
                       const LCD_PIXELINDEX* pTrans)
{
  int i;
  /*
     Use DrawBitLineXBPP
  */
  for (i = 0; i < ySize; i++) {
    switch (BitsPerPixel) {
    case 1:
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 2:
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 4:
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      break;
    case 8:
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      break;
    case 16:
      _DrawBitLine16BPP(pDevice, x0, i + y0, (const U16 *)pData, xSize);
      break;
    case 24:
    case 32:
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      break;
    }
    pData += BytesPerLine;
  }
}

/*********************************************************************
*
*       _SetOrg
*
*  Purpose:
*    Sets the original position of the virtual display.
*    Has no function at this point with the PC-driver.
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  LCDSIM_SetOrg(x, y, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _On
*       _Off
*
*  (Not supported in Simulation)
*/
static void _Off          (GUI_DEVICE * pDevice) {}
static void _On           (GUI_DEVICE * pDevice) {}

/*********************************************************************
*
*       _SetLUTEntry
*/
static void _SetLUTEntry(GUI_DEVICE * pDevice, U8 Pos, LCD_COLOR Color) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (pContext->NumColors <= Pos) {
      pContext->NumColors = Pos + 1;
    }
    if (Pos < GUI_COUNTOF(pContext->aColor)) {
      pContext->aColor[Pos] = Color;
    }
  }
  LCDSIM_SetLUTEntry(Pos, Color, pDevice->LayerIndex);
}

/*********************************************************************
*
*       _Init
*/
static int  _Init(GUI_DEVICE * pDevice) {
  int r;

  r = _InitOnce(pDevice);
	return r;
}

/*********************************************************************
*
*       _GetRect
*/
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  pRect->x0 = 0;
  pRect->y0 = 0;
  pRect->x1 = pContext->vxSize - 1;
  pRect->y1 = pContext->vySize - 1;
}

/*********************************************************************
*
*       _GetDevFunc
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case LCD_DEVFUNC_SETPOS:
    return (void (*)(void))_SetPos;
  case LCD_DEVFUNC_GETPOS:
    return (void (*)(void))_GetPos;
  case LCD_DEVFUNC_SETVIS:
    return (void (*)(void))_SetVis;
  case LCD_DEVFUNC_SETALPHA:
    return (void (*)(void))_SetAlpha;
  case LCD_DEVFUNC_SET_VSIZE:
    return (void (*)(void))_SetVSize;
  case LCD_DEVFUNC_SET_SIZE:
    return (void (*)(void))_SetSize;
  case LCD_DEVFUNC_SETLUTENTRY:
    return (void (*)(void))_SetLUTEntry;
  }
  return NULL;
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
    return pContext->xSize;
  case LCD_DEVCAP_YSIZE:
    return pContext->ySize;
  case LCD_DEVCAP_VXSIZE:
    return pContext->vxSize;
  case LCD_DEVCAP_VYSIZE:
    return pContext->vySize;
  case LCD_DEVCAP_BITSPERPIXEL:
    return LCD__GetBPP(pDevice->pColorConvAPI->pfGetIndexMask());
  case LCD_DEVCAP_NUMCOLORS:
    return pContext->NumColors;
  case LCD_DEVCAP_XMAG:
    return 1;
  case LCD_DEVCAP_YMAG:
    return 1;
  case LCD_DEVCAP_MIRROR_X:
    return 0;
  case LCD_DEVCAP_MIRROR_Y:
    return 0;
  case LCD_DEVCAP_SWAP_XY:
    return 0;
  }
  return -1;
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  DRIVER_CONTEXT * pContext;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  switch (Index) {
  #if GUI_SUPPORT_MEMDEV
    case LCD_DEVDATA_MEMDEV:
      pDevice = GUI_DEVICE__GetpDriver(GUI_Context.SelLayer);
      switch (LCD__GetBPPDevice(pDevice->pColorConvAPI->pfGetIndexMask())) {
      case 1:
        return (void *)&GUI_MEMDEV_DEVICE_1;
      case 8:
        return (void *)&GUI_MEMDEV_DEVICE_8;
      case 16:
        return (void *)&GUI_MEMDEV_DEVICE_16;
      case 32:
        return (void *)&GUI_MEMDEV_DEVICE_32;
      }
      return (void *)NULL;
  #endif
  case LCD_DEVDATA_PHYSPAL:
    return  (void *)pContext->aColor;
  }
  return NULL;
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
const GUI_DEVICE_API GUIDRV_Win_API = {
  //
  // Data
  //
  DEVICE_CLASS_DRIVER,
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
  // Initialization
  //
  //
  // Request information
  //
  _GetDevFunc   ,
  _GetDevProp   ,
  _GetDevData,
  _GetRect      ,
};

#else

void LCDWin_c(void);
void LCDWin_c(void) { } /* avoid empty object files */

#endif /* defined(WIN32) && defined(LCD_USE_WINSIM) */

/*************************** End of file ****************************/
