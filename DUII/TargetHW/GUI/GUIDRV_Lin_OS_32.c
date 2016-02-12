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
File        : GUIDRV_Lin_OS_32.c
Purpose     : Driver for accessing linear video memory
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>

#include "LCD_Private.h"
#include "GUI_Private.h"
#include "LCD_SIM.h"
#include "LCD_ConfDefaults.h"

#include "GUIDRV_Lin.h"

#if (!defined(WIN32) | defined(LCD_SIMCONTROLLER))

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#ifdef WIN32
  //
  // Simulation prototypes
  //
  U16  SIM_Lin_ReadMem16  (unsigned int Off);
  U32  SIM_Lin_ReadMem32  (unsigned int Off);
  U32  SIM_Lin_ReadMem32p (U32 * p);
  void SIM_Lin_WriteMem16 (unsigned int Off, U16 Data);
  void SIM_Lin_WriteMem32 (unsigned int Off, U32 Data);
  void SIM_Lin_WriteMem16p(U16 * p, U16 Data);
  void SIM_Lin_WriteMem32p(U32 * p, U32 Data);
  void SIM_Lin_memcpy     (void * pDst, const void * pSrc, int Len);
  void SIM_Lin_SetVRAMAddr(int LayerIndex, void * pVRAM);
  void SIM_Lin_SetVRAMSize(int LayerIndex, int xSize, int ySize);
  //
  // Access macro definition for internal simulation
  //
  #define LCD_READ_MEM16(VRAMAddr, Off)        SIM_Lin_ReadMem16(Off)
  #define LCD_READ_MEM32(VRAMAddr, Off)        SIM_Lin_ReadMem32(Off)
  #define LCD_READ_MEM32P(p)                   SIM_Lin_ReadMem32p(p)
  #define LCD_WRITE_MEM16(VRAMAddr, Off, Data) SIM_Lin_WriteMem16(Off, Data)
  #define LCD_WRITE_MEM32(VRAMAddr, Off, Data) SIM_Lin_WriteMem32(Off, Data)
  #define LCD_WRITE_MEM16P(p, Data)            SIM_Lin_WriteMem16p(p, Data)
  #define LCD_WRITE_MEM32P(p, Data)            SIM_Lin_WriteMem32p(p, Data)
  #undef  GUI_MEMCPY
  #define GUI_MEMCPY(pDst, pSrc, Len) SIM_Lin_memcpy(pDst, pSrc, Len)
#else
  //
  // Access macro definition for hardware
  //
  #define LCD_READ_MEM16(VRAMAddr, Off)        (*((U16 *)VRAMAddr + (U32)Off))
  #define LCD_READ_MEM32(VRAMAddr, Off)        (*((U32 *)VRAMAddr + (U32)Off))
  #define LCD_READ_MEM32P(p)                   (*((U32 *)p))
  #define LCD_WRITE_MEM16(VRAMAddr, Off, Data) *((U16 *)VRAMAddr + (U32)Off) = Data
  #define LCD_WRITE_MEM32(VRAMAddr, Off, Data) *((U32 *)VRAMAddr + (U32)Off) = Data
  #define LCD_WRITE_MEM16P(p, Data)            *((U16 *)p) = Data
  #define LCD_WRITE_MEM32P(p, Data)            *((U32 *)p) = Data
#endif

#define WRITE_MEM16(VRAMAddr, Off, Data) LCD_WRITE_MEM16(VRAMAddr, Off, Data)
#define WRITE_MEM32(VRAMAddr, Off, Data) LCD_WRITE_MEM32(VRAMAddr, Off, Data)
#define READ_MEM16(VRAMAddr, Off)        LCD_READ_MEM16(VRAMAddr, Off)
#define READ_MEM32(VRAMAddr, Off)        LCD_READ_MEM32(VRAMAddr, Off)
#define READ_MEM32P(p)                   LCD_READ_MEM32P(p)
#define WRITE_MEM16P(p, Data)            LCD_WRITE_MEM16P(p, Data)
#define WRITE_MEM32P(p, Data)            LCD_WRITE_MEM32P(p, Data)

#define OFF2PTR16(VRAMAddr, Off)     (U16 *)((U8 *)VRAMAddr + (Off << 1))
#define OFF2PTR32(VRAMAddr, Off)     (U32 *)((U8 *)VRAMAddr + (Off << 2))

#define XY2OFF16(vxSizePhys, x, y)    ((U32)y * (vxSizePhys << 1) + ((U32)x << 1))
#define XY2OFF32(vxSizePhys, x, y)    ((U32)y *  vxSizePhys       +       x      )

#define LOG2PHYS_X(vxSize, vySize, x, y) y
#define LOG2PHYS_Y(vxSize, vySize, x, y) x

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
  void (* pfFillRect)(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex);
} DRIVER_CONTEXT;

/*********************************************************************
*
*       Static functions
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetPixelIndex
*
* Purpose:
*   Sets the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int PixelIndex) {
  DRIVER_CONTEXT * pContext;
  U32 Off;
  int xPhys, yPhys;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  //  Convert logical into physical coordinates
  //
  xPhys = LOG2PHYS_X(pContext->vxSize, pContext->vySize, x, y);
  yPhys = LOG2PHYS_Y(pContext->vxSize, pContext->vySize, x, y);
  //
  // Write into hardware
  //
  Off      = XY2OFF32(pContext->vxSizePhys, xPhys, yPhys);
  WRITE_MEM32(pContext->VRAMAddr, Off, PixelIndex);
}

/*********************************************************************
*
*       _GetPixelIndex
*
* Purpose:
*   Returns the index of the given pixel. The upper layers
*   calling this routine make sure that the coordinates are in range, so
*   that no check on the parameters needs to be performed.
*/
static unsigned int _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y) {
  DRIVER_CONTEXT * pContext;
  U32 Off;
  LCD_PIXELINDEX PixelIndex;
  int xPhys, yPhys;

  pContext   = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  //  Convert logical into physical coordinates
  //
  xPhys = LOG2PHYS_X(pContext->vxSize, pContext->vySize, x, y);
  yPhys = LOG2PHYS_Y(pContext->vxSize, pContext->vySize, x, y);
  //
  // Read from hardware
  //
  Off        = XY2OFF32(pContext->vxSizePhys, xPhys, yPhys);
  PixelIndex = READ_MEM32(pContext->VRAMAddr, Off);
  return PixelIndex;
}

/*********************************************************************
*
*       _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  LCD_PIXELINDEX PixelIndex;
  LCD_PIXELINDEX IndexMask;

  PixelIndex = _GetPixelIndex(pDevice, x, y);
  IndexMask  = pDevice->pColorConvAPI->pfGetIndexMask();
  _SetPixelIndex(pDevice, x, y, PixelIndex ^ IndexMask);
}

/*********************************************************************
*
*       _DrawHLine
*/
static void _DrawHLine  (GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  LCD_PIXELINDEX ColorIndex;

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (; x0 <= x1; x0++) {
      _XorPixel(pDevice, x0, y);
    }
  } else {
    ColorIndex = LCD__GetColorIndex();
    for (; x0 <= x1; x0++) {
      _SetPixelIndex(pDevice, x0, y, ColorIndex);
    }
  }
}

/*********************************************************************
*
*       _DrawVLine
*/
static void _DrawVLine  (GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  DRIVER_CONTEXT * pContext;
  U32 Off, RemPixels;
  U32 * pDest;
  int x0, x1, y;
  LCD_PIXELINDEX ColorIndex;

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (; y0 <= y1; y0++) {
      _XorPixel(pDevice, x, y0);
    }
  } else {
    ColorIndex = LCD__GetColorIndex();
    pContext  = (DRIVER_CONTEXT *)pDevice->u.pContext;
    x0        = y0;
    x1        = y1;
    y         = x;
    Off       = XY2OFF32(pContext->vxSizePhys, x0, y);
    RemPixels = x1 - x0 + 1;
    pDest     = OFF2PTR32(pContext->VRAMAddr, Off);
    if (RemPixels >= 8) {
      do {
        WRITE_MEM32P(pDest,     ColorIndex);
        WRITE_MEM32P(pDest + 1, ColorIndex);
        WRITE_MEM32P(pDest + 2, ColorIndex);
        WRITE_MEM32P(pDest + 3, ColorIndex);
        WRITE_MEM32P(pDest + 4, ColorIndex);
        WRITE_MEM32P(pDest + 5, ColorIndex);
        WRITE_MEM32P(pDest + 6, ColorIndex);
        WRITE_MEM32P(pDest + 7, ColorIndex);
        pDest += 8;
      } while ((RemPixels -= 8) >= 8);
    }
    if (RemPixels) {
      do {
        WRITE_MEM32P(pDest, ColorIndex);
        pDest++;
      } while (--RemPixels);
    }
  }
}

/*********************************************************************
*
*       _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  #ifdef LCD_FILL_RECT
     LCD_FILL_RECT(x0, y0, x1, y1, LCD__GetColorIndex());
  #else
    for (; x0 <= x1; x0++) {
      _DrawVLine(pDevice, x0, y0, y1);
    }
  #endif
}

/*********************************************************************
*
*       Draw Bitmap 1 BPP
*/
static void _DrawBitLine1BPP_Swap(GUI_DEVICE * pDevice, unsigned x, unsigned y, U8 const GUI_UNI_PTR * pData, int ySize, const LCD_PIXELINDEX * pTrans, int BytesPerLine, U8 Pos) {
  DRIVER_CONTEXT * pContext;
  LCD_PIXELINDEX Index, Index0, Index1, IndexMask;
  U32 Off;
  U8 Mask;
  U32 * pDest;
  int x0, y0;

  pContext  = (DRIVER_CONTEXT *)pDevice->u.pContext;
  y0        = x;
  x0        = y;
  IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
  Off       = XY2OFF32(pContext->vxSizePhys, x0, y0);
  pDest     = OFF2PTR32(pContext->VRAMAddr, Off);
  Index0    = *(pTrans + 0);
  Index1    = *(pTrans + 1);
  Mask      = 0x80 >> (Pos & 0x7);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    while (ySize >= 4) {
      do {
        Index = (*pData & Mask) ? Index1 : Index0;
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = (*pData & Mask) ? Index1 : Index0;
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = (*pData & Mask) ? Index1 : Index0;
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = (*pData & Mask) ? Index1 : Index0;
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        ySize -= 4;
      } while (ySize >= 4);
    }
    if (ySize) { 
	    do {
        Index = (*pData & Mask) ? Index1 : Index0;
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
      } while (--ySize);
    }
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    do {
      if (*pData & Mask) {
        Index = READ_MEM32(pContext->VRAMAddr, Off);
        Index ^= IndexMask;
        WRITE_MEM32(pContext->VRAMAddr, Off, Index);
      }
      pData += BytesPerLine;
      Off++;
    } while (--ySize);
    break;
  case LCD_DRAWMODE_TRANS:
    while (ySize >= 4) {
      do {
        if (*pData & Mask) {
          WRITE_MEM32P(pDest, Index1);
        }
        pData += BytesPerLine;
        pDest++;
        if (*pData & Mask) {
          WRITE_MEM32P(pDest, Index1);
        }
        pData += BytesPerLine;
        pDest++;
        if (*pData & Mask) {
          WRITE_MEM32P(pDest, Index1);
        }
        pData += BytesPerLine;
        pDest++;
        if (*pData & Mask) {
          WRITE_MEM32P(pDest, Index1);
        }
        pData += BytesPerLine;
        pDest++;
        ySize -= 4;
      } while (ySize >= 4);
    }
    if (ySize) { 
	    do {
        if (*pData & Mask) {
          WRITE_MEM32P(pDest, Index1);
        }
        pData += BytesPerLine;
        pDest++;
      } while (--ySize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 2 BPP
*/
static void  _DrawBitLine2BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;

  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        _SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        _SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (3 - CurrentPixel) << 1;
        Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
        if (Index) {
          _SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 4) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 4 BPP
*/
static void  _DrawBitLine4BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixels, PixelIndex;
  int CurrentPixel, Shift, Index;

  Pixels = *p;
  CurrentPixel = Diff;
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        PixelIndex = *(pTrans + Index);
        _SetPixelIndex(pDevice, x++, y, PixelIndex);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        _SetPixelIndex(pDevice, x++, y, Index);
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          PixelIndex = *(pTrans + Index);
          _SetPixelIndex(pDevice, x, y, PixelIndex);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    } else {
      do {
        Shift = (1 - CurrentPixel) << 2;
        Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
        if (Index) {
          _SetPixelIndex(pDevice, x, y, Index);
        }
        x++;
        if (++CurrentPixel == 2) {
          CurrentPixel = 0;
          Pixels = *(++p);
        }
      } while (--xsize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 8 BPP
*/
static void _DrawBitLine8BPP_Swap(GUI_DEVICE * pDevice, unsigned x, unsigned y, U8 const GUI_UNI_PTR * pData, int ySize, const LCD_PIXELINDEX * pTrans, int BytesPerLine) {
  DRIVER_CONTEXT * pContext;
  LCD_PIXELINDEX Index;
  U32 Off;
  U8 Pixel;
  U32 * pDest;
  int x0, y0;
  
  if (pTrans == NULL) {
    return; // Because it makes no sense to use 8bpp bitmaps without palette with a 16bpp configuration
  }
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  y0        = x;
  x0        = y;
  Off      = XY2OFF32(pContext->vxSizePhys, x0, y0);
  pDest    = OFF2PTR32(pContext->VRAMAddr, Off);
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    while (ySize >= 4) {
      do {
        Index = *(pTrans + *pData);
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = *(pTrans + *pData);
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = *(pTrans + *pData);
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        Index = *(pTrans + *pData);
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
        ySize -= 4;
      } while (ySize >= 4);
    }
    if (ySize) { 
	    do {
        Index = *(pTrans + *pData);
        WRITE_MEM32P(pDest, Index);
        pData += BytesPerLine;
        pDest++;
      } while (--ySize);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    while (ySize >= 4) {
      do {
        Pixel = *pData;
        if (Pixel) {
          Index = *(pTrans + Pixel);
          WRITE_MEM32P(pDest, Index);
        }
        pData += BytesPerLine;
        pDest++;
        Pixel = *pData;
        if (Pixel) {
          Index = *(pTrans + Pixel);
          WRITE_MEM32P(pDest, Index);
        }
        pData += BytesPerLine;
        pDest++;
        Pixel = *pData;
        if (Pixel) {
          Index = *(pTrans + Pixel);
          WRITE_MEM32P(pDest, Index);
        }
        pData += BytesPerLine;
        pDest++;
        Pixel = *pData;
        if (Pixel) {
          Index = *(pTrans + Pixel);
          WRITE_MEM32P(pDest, Index);
        }
        pData += BytesPerLine;
        pDest++;
        ySize -= 4;
      } while (ySize >= 4);
    }
    if (ySize) { 
	    do {
        Pixel = *pData;
        if (Pixel) {
          Index = *(pTrans + Pixel);
          WRITE_MEM32P(pDest, Index);
        }
        pData += BytesPerLine;
        pDest++;
      } while (--ySize);
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 32 BPP
*/
static void _DrawBitLine32BPP_Swap(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * pData, int ySize, int BytesPerLine) {
  DRIVER_CONTEXT * pContext;
  U32 Off;
  U32 * pDest;
  int WordsPerLine;

  pContext     = (DRIVER_CONTEXT *)pDevice->u.pContext;
  WordsPerLine = BytesPerLine >> 2;
  Off          = XY2OFF32(pContext->vxSizePhys, y, x);
  pDest        = OFF2PTR32(pContext->VRAMAddr, Off);
  while (ySize >= 4) {
    do {
      WRITE_MEM32P(pDest, *pData);
      pDest++;
      pData += WordsPerLine;
      WRITE_MEM32P(pDest, *pData);
      pDest++;
      pData += WordsPerLine;
      WRITE_MEM32P(pDest, *pData);
      pDest++;
      pData += WordsPerLine;
      WRITE_MEM32P(pDest, *pData);
      pDest++;
      pData += WordsPerLine;
      ySize -= 4;
    } while (ySize >= 4);
  }
  if (ySize) { 
	  do {
      WRITE_MEM32P(pDest, *pData);
      pData += WordsPerLine;
      pDest++;
    } while (--ySize);
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
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans) {
  int i;

  //
  // Use _DrawBitLineXBPP and _DrawBitLineXBPP_Swap
  //
  switch (BitsPerPixel) {
  case 1:
    xSize += Diff & 7;
    for (i = Diff; i < xSize; i++) {
      _DrawBitLine1BPP_Swap(pDevice, x0 + i, y0, pData + (i >> 3), ySize, pTrans, BytesPerLine, i);
    }
    break;
  case 2:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine2BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 4:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine4BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 8:
    for (i = 0; i < xSize; i++) {
      _DrawBitLine8BPP_Swap(pDevice, x0 + i, y0, pData + i, ySize, pTrans, BytesPerLine);
    }
    break;
  case 32:
    for (i = Diff; i < xSize; i++) {
      _DrawBitLine32BPP_Swap(pDevice, x0 + i, y0, (U32 *)(pData + (i << 2)), ySize, BytesPerLine);
    }
    break;
  }
}

/*********************************************************************
*
*       _SetOrg
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  LCD_X_SETORG_INFO Data = {0};

  #ifdef WIN32
    LCDSIM_SetOrg(x, y, pDevice->LayerIndex);
  #else
    Data.xPos = y;
    Data.yPos = x;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETORG, (void *)&Data);
  #endif
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
  if (pDevice->u.pContext == NULL) {
    pDevice->u.pContext = GUI_ALLOC_GetFixedBlock(sizeof(DRIVER_CONTEXT));
    GUI__memset((U8 *)pDevice->u.pContext, 0, sizeof(DRIVER_CONTEXT));
  }
  return pDevice->u.pContext ? 0 : 1;
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
    return 32;
  case LCD_DEVCAP_NUMCOLORS:
    return 0;
  case LCD_DEVCAP_XMAG:
    return 1;
  case LCD_DEVCAP_YMAG:
    return 1;
  case LCD_DEVCAP_MIRROR_X:
    return 0;
  case LCD_DEVCAP_MIRROR_Y:
    return 0;
  case LCD_DEVCAP_SWAP_XY:
    return 1;
  }
  return -1;
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  switch (Index) {
  #if GUI_SUPPORT_MEMDEV
    case LCD_DEVDATA_MEMDEV:
      return (void *)&GUI_MEMDEV_DEVICE_32;
  #endif
  }
  return NULL;
}

/*********************************************************************
*
*       Static code: Functions available by _GetDevFunc()
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetVRAMAddr
*/
static void _SetVRAMAddr(GUI_DEVICE * pDevice, void * pVRAM) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETVRAMADDR_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->VRAMAddr = (U32)pVRAM;
    Data.pVRAM = pVRAM;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETVRAMADDR, (void *)&Data);
  }
  #ifdef WIN32
    SIM_Lin_SetVRAMAddr(pDevice->LayerIndex, pVRAM);
  #endif
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
  #ifdef WIN32
    SIM_Lin_SetVRAMSize(pDevice->LayerIndex, xSize, ySize);
  #endif
}

/*********************************************************************
*
*       _SetSize
*/
static void _SetSize(GUI_DEVICE * pDevice, int xSize, int ySize) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETSIZE_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    if (pContext->vxSizePhys == 0) {
      pContext->vxSizePhys = ySize;
    }
    pContext->xSize = xSize;
    pContext->ySize = ySize;
    Data.xSize = xSize;
    Data.ySize = ySize;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETSIZE, (void *)&Data);
  }
}

/*********************************************************************
*
*       _SetPos
*/
static void _SetPos(GUI_DEVICE * pDevice, int xPos, int yPos) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETPOS_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->xPos = xPos;
    pContext->yPos = yPos;
    Data.xPos = xPos;
    Data.yPos = yPos;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETPOS, (void *)&Data);
  }
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
*       _SetAlpha
*/
static void _SetAlpha(GUI_DEVICE * pDevice, int Alpha) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETALPHA_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->Alpha = Alpha;
    Data.Alpha = Alpha;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETALPHA, (void *)&Data);
  }
}

/*********************************************************************
*
*       _SetVis
*/
static void _SetVis(GUI_DEVICE * pDevice, int OnOff) {
  DRIVER_CONTEXT * pContext;
  LCD_X_SETVIS_INFO Data = {0};

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    pContext->IsVisible = OnOff;
    Data.OnOff = OnOff;
    LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETVIS, (void *)&Data);
  }
}

/*********************************************************************
*
*       _Init
*/
static int  _Init(GUI_DEVICE * pDevice) {
  int r;

  r = _InitOnce(pDevice);
  r |= LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_INITCONTROLLER, NULL);
  return r;
}

/*********************************************************************
*
*       _On
*/
static void _On (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_ON, NULL);
}

/*********************************************************************
*
*       _Off
*/
static void _Off (GUI_DEVICE * pDevice) {
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_OFF, NULL);
}

/*********************************************************************
*
*       _SetLUTEntry
*/
static void _SetLUTEntry(GUI_DEVICE * pDevice, U8 Pos, LCD_COLOR Color) {
  LCD_X_SETLUTENTRY_INFO Data = {0};

  Data.Pos   = Pos;
  Data.Color = Color;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETLUTENTRY, (void *)&Data);
}

/*********************************************************************
*
*       _SetAlphaMode
*/
static void _SetAlphaMode(GUI_DEVICE * pDevice, int AlphaMode) {
  LCD_X_SETALPHAMODE_INFO Data = {0};

  Data.AlphaMode = AlphaMode;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETALPHAMODE, (void *)&Data);
}

/*********************************************************************
*
*       _SetChromaMode
*/
static void _SetChromaMode(GUI_DEVICE * pDevice, int ChromaMode) {
  LCD_X_SETCHROMAMODE_INFO Data = {0};

  Data.ChromaMode = ChromaMode;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETCHROMAMODE, (void *)&Data);
}

/*********************************************************************
*
*       _SetChroma
*/
static void _SetChroma(GUI_DEVICE * pDevice, LCD_COLOR ChromaMin, LCD_COLOR ChromaMax) {
  LCD_X_SETCHROMA_INFO Data = {0};

  Data.ChromaMin = ChromaMin;
  Data.ChromaMax = ChromaMax;
  LCD_X_DisplayDriver(pDevice->LayerIndex, LCD_X_SETCHROMA, (void *)&Data);
}

/*********************************************************************
*
*       _SetFunc
*/
static void _SetFunc(GUI_DEVICE * pDevice, int Index, void (* pFunc)(void)) {
  DRIVER_CONTEXT * pContext;

  _InitOnce(pDevice);
  if (pDevice->u.pContext) {
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    switch (Index) {
    case LCD_DEVFUNC_FILLRECT:
      pContext->pfFillRect = (void (*)(int LayerIndex, int x0, int y0, int x1, int y1, U32 PixelIndex))pFunc;
      break;
    }
  }
}

/*********************************************************************
*
*       _GetDevFunc
*/
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  switch (Index) {
  case LCD_DEVFUNC_READRECT:
    return (void (*)(void))NULL;
  case LCD_DEVFUNC_SET_VRAM_ADDR:
    return (void (*)(void))_SetVRAMAddr;
  case LCD_DEVFUNC_SET_VSIZE:
    return (void (*)(void))_SetVSize;
  case LCD_DEVFUNC_SET_SIZE:
    return (void (*)(void))_SetSize;
  case LCD_DEVFUNC_SETPOS:
    return (void (*)(void))_SetPos;
  case LCD_DEVFUNC_GETPOS:
    return (void (*)(void))_GetPos;
  case LCD_DEVFUNC_SETALPHA:
    return (void (*)(void))_SetAlpha;
  case LCD_DEVFUNC_SETVIS:
    return (void (*)(void))_SetVis;
  case LCD_DEVFUNC_INIT:
    return (void (*)(void))_Init;
  case LCD_DEVFUNC_ON:
    return (void (*)(void))_On;
  case LCD_DEVFUNC_OFF:
    return (void (*)(void))_Off;
  case LCD_DEVFUNC_SETLUTENTRY:
    return (void (*)(void))_SetLUTEntry;

  case LCD_DEVFUNC_ALPHAMODE:
    return (void (*)(void))_SetAlphaMode;
  case LCD_DEVFUNC_CHROMAMODE:
    return (void (*)(void))_SetChromaMode;
  case LCD_DEVFUNC_CHROMA:
    return (void (*)(void))_SetChroma;
  
  case LCD_DEVFUNC_SETFUNC:
    return (void (*)(void))_SetFunc;
  }
  return NULL;
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
const GUI_DEVICE_API GUIDRV_Lin_OS_32_API = {
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

#else

void GUIDRV_Lin_OS_32_C(void);   // Avoid empty object files
void GUIDRV_Lin_OS_32_C(void) {}

#endif

/*************************** End of file ****************************/
