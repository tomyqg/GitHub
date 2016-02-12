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
File        : GUIDRV_Lin_OY_24.c
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

#define XY2OFF32(vxSizePhys, x, y)      (((U32)(y * (vxSizePhys + vxSizePhys + vxSizePhys)) + ((unsigned)(x + x + x))) >> 2)

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
  U32 Off, Data;

  //
  // Get context
  //
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  // Mirror y
  //
  y = (pContext->vySize - 1 - (y));
  //
  // Write into hardware
  //
  Off      = XY2OFF32(pContext->vxSizePhys, x, y);
  Data     = READ_MEM32(pContext->VRAMAddr, Off);
  PixelIndex &= 0xFFFFFF;
  switch (x & 3) {
  case 0:
    Data &= 0xFF000000;
    Data |= PixelIndex;
    break;
  case 1:
    Data &= 0x00FFFFFF;
    Data |= (PixelIndex << 24);
    WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    Off++;
    Data = READ_MEM32(pContext->VRAMAddr, Off);
    Data &= 0xFFFF0000;
    Data |= (PixelIndex >> 8);
    break;
  case 2:
    Data &= 0x0000FFFF;
    Data |= (PixelIndex << 16);
    WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    Off++;
    Data = READ_MEM32(pContext->VRAMAddr, Off);
    Data &= 0xFFFFFF00;
    Data |= (PixelIndex >> 16);
    break;
  case 3:
    Data &= 0x000000FF;
    Data |= (PixelIndex << 8);
    break;
  }
  WRITE_MEM32(pContext->VRAMAddr, Off, Data);
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
  U32 Off, Data, PixelIndex;

  //
  // Get context
  //
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  // Mirror y
  //
  y = (pContext->vySize - 1 - (y));
  //
  // Read from hardware
  //
  Off      = XY2OFF32(pContext->vxSizePhys, x, y);
  Data     = READ_MEM32(pContext->VRAMAddr, Off);
  switch (x & 3) {
  case 0:
    PixelIndex  = (Data & 0x00FFFFFF);
    break;
  case 1:
    PixelIndex  = (Data & 0xFF000000) >> 24;
    Off++;
    Data        = READ_MEM32(pContext->VRAMAddr, Off);
    PixelIndex |= (Data & 0x0000FFFF) << 8;
    break;
  case 2:
    PixelIndex  = (Data & 0xFFFF0000) >> 16;
    Off++;
    Data        = READ_MEM32(pContext->VRAMAddr, Off);
    PixelIndex |= (Data & 0x000000FF) << 16;
    break;
  case 3:
    PixelIndex  = (Data & 0xFFFFFF00) >> 8;
    break;
  }
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
  DRIVER_CONTEXT * pContext;
  int Off, RemPixels, Odd;
  U32 Data;
  LCD_PIXELINDEX ColorIndex;

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (; x0 <= x1; x0++) {
      _XorPixel(pDevice, x0, y);
    }
  } else {
    ColorIndex = LCD__GetColorIndex();
    //
    // Get context
    //
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    //
    // Mirror y
    //
    y = (pContext->vySize - 1 - (y));
    Off = XY2OFF32(pContext->vxSizePhys, x0, y);
    RemPixels = x1 - x0 + 1;
    //
    // First triple DWORD
    //
    Odd = x0 & 3;
    if (Odd) {
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      switch (Odd) {
      case 1:
        Data &= 0x00FFFFFF;
        Data |= ColorIndex << 24;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFF0000;
        Data |= ColorIndex >> 8;
        RemPixels--;
        if (!RemPixels) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          break;
        }
        //
        // no break at this position required...
        //
      case 2:
        Data &= 0x0000FFFF;
        Data |= ColorIndex << 16;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFFFF00;
        Data |= ColorIndex >> 16;
        RemPixels--;
        if (!RemPixels) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          break;
        }
        //
        // no break at this position required...
        //
      case 3:
        Data &= 0x000000FF;
        Data |= ColorIndex << 8;
        RemPixels--;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
      }
    }
    //
    // Complete triple DWORDS
    //
    if (RemPixels >= 4) {
      U32 Data0, Data1, Data2;
      Data0 = (ColorIndex      ) | (ColorIndex << 24);
      Data1 = (ColorIndex >>  8) | (ColorIndex << 16);
      Data2 = (ColorIndex >> 16) | (ColorIndex <<  8);
      do {
        WRITE_MEM32(pContext->VRAMAddr, Off + 0, Data0);
        WRITE_MEM32(pContext->VRAMAddr, Off + 1, Data1);
        WRITE_MEM32(pContext->VRAMAddr, Off + 2, Data2);
        Off += 3;
      } while ((RemPixels -= 4) >= 4);
    }
    //
    // Last triple DWORD
    //
    if (RemPixels) {
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFF000000;
      Data |= ColorIndex;
      RemPixels--;
      if (!RemPixels) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        return;
      }
      Data &= 0x00FFFFFF;
      Data |= ColorIndex << 24;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFF0000;
      Data |= ColorIndex >> 8;
      RemPixels--;
      if (!RemPixels) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        return;
      }
      Data &= 0x0000FFFF;
      Data |= ColorIndex << 16;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFFFF00;
      Data |= ColorIndex >> 16;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    }
  }
}

/*********************************************************************
*
*       _DrawVLine, not optimized
*/
static void _DrawVLine  (GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  LCD_PIXELINDEX ColorIndex;

  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    for (; y0 <= y1; y0++) {
      _XorPixel(pDevice, x, y0);
    }
  } else {
    ColorIndex = LCD__GetColorIndex();
    for (; y0 <= y1; y0++) {
      _SetPixelIndex(pDevice, x, y0, ColorIndex);
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
    for (; y0 <= y1; y0++) {
      _DrawHLine(pDevice, x0, y0, x1);
    }
  #endif
}

/*********************************************************************
*
*       Draw Bitmap 1 BPP
*/
static void _DrawBitLine1BPP(GUI_DEVICE * pDevice, unsigned x, unsigned y, U8 const GUI_UNI_PTR * p, int Diff, int xsize, const LCD_PIXELINDEX * pTrans) {
  #define GET_PIXEL_INDEX(Index) {               \
    Index = ((Pixels & 1) ? Index1 : Index0);    \
    PixelCnt--;                                  \
    Pixels >>= 1;                                \
    if (PixelCnt == 0) {                         \
      Pixels |= LCD_aMirror[*(++p)] << PixelCnt; \
      PixelCnt += 8;                             \
    }                                            \
  }
  #define GET_PIXEL_INDEX_TRANS(Index, Bit) {    \
    Bit = Pixels & 1;                            \
    GET_PIXEL_INDEX(Index);                      \
  }
  DRIVER_CONTEXT * pContext;
  LCD_PIXELINDEX Index0, Index1;
  LCD_PIXELINDEX IndexMask;
  U32 Off, Data, Data0, Data1, Data2, Index;
  unsigned Pixels, PixelCnt, Odd, Bit;

  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  Index0 = *(pTrans + 0);
  Index1 = *(pTrans + 1);
  x += Diff;
  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    //
    // Mirror y
    //
    y = (pContext->vySize - 1 - (y));
    PixelCnt = 8 - Diff;
    Pixels   = LCD_aMirror[*p] >> Diff;
    Off      = XY2OFF32(pContext->vxSizePhys, x, y);
    //
    // First triple DWORD
    //
    Odd = x & 3;
    if (Odd) {                    
      Data    = READ_MEM32(pContext->VRAMAddr, Off);
      switch (Odd) {
      case 1:
        GET_PIXEL_INDEX(Index);
        Data &= 0x00FFFFFF;
        Data |= Index << 24;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFF0000;
        Data |= Index >> 8;
        xsize--;
        if (!xsize) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          break;
        }
      case 2:
        GET_PIXEL_INDEX(Index);
        Data &= 0x0000FFFF;
        Data |= Index << 16;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFFFF00;
        Data |= Index >> 16;
        xsize--;
        if (!xsize) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          break;
        }
      case 3:
        GET_PIXEL_INDEX(Index);
        Data &= 0x000000FF;
        Data |= Index << 8;
        xsize--;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
      }
    }
    //
    // Complete triple DWORDS
    //
    if (xsize >= 4) {
      do {
        GET_PIXEL_INDEX(Index);
        Data0  = (Index      );
        GET_PIXEL_INDEX(Index);
        Data0 |= (Index << 24);
        Data1  = (Index >>  8);
        GET_PIXEL_INDEX(Index);
        Data1 |= (Index << 16);
        Data2  = (Index >> 16);
        GET_PIXEL_INDEX(Index);
        Data2 |= (Index <<  8);
        WRITE_MEM32(pContext->VRAMAddr, Off + 0, Data0);
        WRITE_MEM32(pContext->VRAMAddr, Off + 1, Data1);
        WRITE_MEM32(pContext->VRAMAddr, Off + 2, Data2);
        Off += 3;
      } while ((xsize -= 4) >= 4);
    }
    //
    // Last triple DWORD
    //
    if (xsize) {
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFF000000;
      GET_PIXEL_INDEX(Index);
      Data |= Index;
      xsize--;
      if (!xsize) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        return;
      }
      Data &= 0x00FFFFFF;
      GET_PIXEL_INDEX(Index);
      Data |= Index << 24;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFF0000;
      Data |= Index >> 8;
      xsize--;
      if (!xsize) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        return;
      }
      Data &= 0x0000FFFF;
      GET_PIXEL_INDEX(Index);
      Data |= Index << 16;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFFFF00;
      Data |= Index >> 16;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    }
    break;
  case LCD_DRAWMODE_TRANS:
    //
    // Mirror y
    //
    y = (pContext->vySize - 1 - (y));
    PixelCnt = 8 - Diff;
    Pixels   = LCD_aMirror[*p] >> Diff;
    Off      = XY2OFF32(pContext->vxSizePhys, x, y);
    //
    // First triple DWORD
    //
    Odd = x & 3;
    if (Odd) {
      Data    = READ_MEM32(pContext->VRAMAddr, Off);
      switch (Odd) {
      case 1:
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data &= 0x00FFFFFF;
          Data |= Index << 24;
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          Off++;
          Data  = READ_MEM32(pContext->VRAMAddr, Off);
          Data &= 0xFFFF0000;
          Data |= Index >> 8;
          xsize--;
          if (!xsize) {
            WRITE_MEM32(pContext->VRAMAddr, Off, Data);
            break;
          }
        } else {
          Off++;
          xsize--;
          if (!xsize) {
            break;
          }
          Data = READ_MEM32(pContext->VRAMAddr, Off);
        }
      case 2:
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data &= 0x0000FFFF;
          Data |= Index << 16;
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          Off++;
          Data  = READ_MEM32(pContext->VRAMAddr, Off);
          Data &= 0xFFFFFF00;
          Data |= Index >> 16;
          xsize--;
          if (!xsize) {
            WRITE_MEM32(pContext->VRAMAddr, Off, Data);
            break;
          }
        } else {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          Off++;
          xsize--;
          if (!xsize) {
            break;
          }
          Data = READ_MEM32(pContext->VRAMAddr, Off);
        }
      case 3:
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data &= 0x000000FF;
          Data |= Index << 8;
        }
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        xsize--;
        Off++;
      }
    }
    //
    // Complete triple DWORDS
    //
    if (xsize >= 4) {
      do {
        Data0  = READ_MEM32(pContext->VRAMAddr, Off + 0);
        Data1  = READ_MEM32(pContext->VRAMAddr, Off + 1);
        Data2  = READ_MEM32(pContext->VRAMAddr, Off + 2);
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data0 &= 0xFF000000;
          Data0 |= (Index      );
        }
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data0 &= 0x00FFFFFF;
          Data0 |= (Index << 24);
          Data1 &= 0xFFFF0000;
          Data1 |= (Index >>  8);
        }
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data1 &= 0x0000FFFF;
          Data1 |= (Index << 16);
          Data2 &= 0xFFFFFF00;
          Data2 |= (Index >> 16);
        }
        GET_PIXEL_INDEX_TRANS(Index, Bit);
        if (Bit) {
          Data2 &= 0x000000FF;
          Data2 |= (Index <<  8);
        }
        WRITE_MEM32(pContext->VRAMAddr, Off + 0, Data0);
        WRITE_MEM32(pContext->VRAMAddr, Off + 1, Data1);
        WRITE_MEM32(pContext->VRAMAddr, Off + 2, Data2);
        Off += 3;
      } while ((xsize -= 4) >= 4);
    }
    //
    // Last triple DWORD
    //
    if (xsize) {
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      GET_PIXEL_INDEX_TRANS(Index, Bit);
      if (Bit) {
        Data &= 0xFF000000;
        Data |= Index;
        xsize--;
        if (!xsize) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          return;
        }
      } else {
        xsize--;
        if (!xsize) {
          return;
        }
      }
      GET_PIXEL_INDEX_TRANS(Index, Bit);
      if (Bit) {
        Data &= 0x00FFFFFF;
        Data |= Index << 24;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFF0000;
        Data |= Index >> 8;
        xsize--;
        if (!xsize) {
          WRITE_MEM32(pContext->VRAMAddr, Off, Data);
          return;
        }
      } else {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        xsize--;
        if (!xsize) {
          return;
        }
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
      }
      GET_PIXEL_INDEX_TRANS(Index, Bit);
      if (Bit) {
        Data &= 0x0000FFFF;
        Data |= Index << 16;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        Off++;
        Data  = READ_MEM32(pContext->VRAMAddr, Off);
        Data &= 0xFFFFFF00;
        Data |= Index >> 16;
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      } else {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      }
    }
    break;
  case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
  case LCD_DRAWMODE_XOR:
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    do {
      if (*p & (0x80 >> Diff)) {
        int Pixel = _GetPixelIndex(pDevice, x, y);
        _SetPixelIndex(pDevice, x, y, Pixel ^ IndexMask);
      }
      x++;
      if (++Diff == 8) {
        Diff = 0;
        p++;
      }
    } while (--xsize);
    break;
  }
  #undef GET_PIXEL_INDEX
  #undef GET_PIXEL_INDEX_TRANS
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
static void  _DrawBitLine8BPP(GUI_DEVICE * pDevice, int x, int y, U8 const GUI_UNI_PTR * p, int xsize, const LCD_PIXELINDEX * pTrans) {
  LCD_PIXELINDEX Pixel;

  switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
  case 0:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        _SetPixelIndex(pDevice, x, y, *p);
      }
    }
    break;
  case LCD_DRAWMODE_TRANS:
    if (pTrans) {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          _SetPixelIndex(pDevice, x, y, *(pTrans + Pixel));
        }
      }
    } else {
      for (; xsize > 0; xsize--, x++, p++) {
        Pixel = *p;
        if (Pixel) {
          _SetPixelIndex(pDevice, x, y, Pixel);
        }
      }
    }
    break;
  }
}

/*********************************************************************
*
*       Draw Bitmap 32 BPP
*/
static void _DrawBitLine32BPP(GUI_DEVICE * pDevice, int x, int y, U32 const GUI_UNI_PTR * p, int xsize) {
  #define GET_PIXEL_INDEX(Index) Index = *p++
  DRIVER_CONTEXT * pContext;
  U32 Data, Data0, Data1, Data2, Index;
  unsigned Off, Odd;

  //
  // Get context
  //
  pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
  //
  // Mirror y
  //
  y = (pContext->vySize - 1 - (y));
  //
  // First triple DWORD
  //
  Off = XY2OFF32(pContext->vxSizePhys, x, y);
  Odd = x & 3;
  if (Odd) {
    Data    = READ_MEM32(pContext->VRAMAddr, Off);
    switch (Odd) {
    case 1:
      GET_PIXEL_INDEX(Index);
      Data &= 0x00FFFFFF;
      Data |= Index << 24;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFF0000;
      Data |= Index >> 8;
      xsize--;
      if (!xsize) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        break;
      }
    case 2:
      GET_PIXEL_INDEX(Index);
      Data &= 0x0000FFFF;
      Data |= Index << 16;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
      Data  = READ_MEM32(pContext->VRAMAddr, Off);
      Data &= 0xFFFFFF00;
      Data |= Index >> 16;
      xsize--;
      if (!xsize) {
        WRITE_MEM32(pContext->VRAMAddr, Off, Data);
        break;
      }
    case 3:
      GET_PIXEL_INDEX(Index);
      Data &= 0x000000FF;
      Data |= Index << 8;
      xsize--;
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      Off++;
    }
  }
  //
  // Complete triple DWORDS
  //
  if (xsize >= 4) {
    do {
      GET_PIXEL_INDEX(Index);
      Data0  = (Index      );
      GET_PIXEL_INDEX(Index);
      Data0 |= (Index << 24);
      Data1  = (Index >>  8);
      GET_PIXEL_INDEX(Index);
      Data1 |= (Index << 16);
      Data2  = (Index >> 16);
      GET_PIXEL_INDEX(Index);
      Data2 |= (Index <<  8);
      WRITE_MEM32(pContext->VRAMAddr, Off + 0, Data0);
      WRITE_MEM32(pContext->VRAMAddr, Off + 1, Data1);
      WRITE_MEM32(pContext->VRAMAddr, Off + 2, Data2);
      Off += 3;
    } while ((xsize -= 4) >= 4);
  }
  //
  // Last triple DWORD
  //
  if (xsize) {
    Data  = READ_MEM32(pContext->VRAMAddr, Off);
    Data &= 0xFF000000;
    GET_PIXEL_INDEX(Index);
    Data |= Index;
    xsize--;
    if (!xsize) {
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      return;
    }
    Data &= 0x00FFFFFF;
    GET_PIXEL_INDEX(Index);
    Data |= Index << 24;
    WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    Off++;
    Data  = READ_MEM32(pContext->VRAMAddr, Off);
    Data &= 0xFFFF0000;
    Data |= Index >> 8;
    xsize--;
    if (!xsize) {
      WRITE_MEM32(pContext->VRAMAddr, Off, Data);
      return;
    }
    Data &= 0x0000FFFF;
    GET_PIXEL_INDEX(Index);
    Data |= Index << 16;
    WRITE_MEM32(pContext->VRAMAddr, Off, Data);
    Off++;
    Data  = READ_MEM32(pContext->VRAMAddr, Off);
    Data &= 0xFFFFFF00;
    Data |= Index >> 16;
    WRITE_MEM32(pContext->VRAMAddr, Off, Data);
  }
  #undef GET_PIXEL_INDEX
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
    for (i = 0; i < ySize; i++) {
      _DrawBitLine1BPP(pDevice, x0, i + y0, pData, Diff, xSize, pTrans);
      pData += BytesPerLine;
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
    for (i = 0; i < ySize; i++) {
      _DrawBitLine8BPP(pDevice, x0, i + y0, pData, xSize, pTrans);
      pData += BytesPerLine;
    }
    break;
  case 32:
    for (i = 0; i < ySize; i++) {
      _DrawBitLine32BPP(pDevice, x0, i + y0, (const U32 *)pData, xSize);
      pData += BytesPerLine;
    }
    break;
  }
}

/*********************************************************************
*
*       _SetOrg
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  #ifndef WIN32
    DRIVER_CONTEXT * pContext;
  #endif
  LCD_X_SETORG_INFO Data = {0};

  #ifdef WIN32
    LCDSIM_SetOrg(x, y, pDevice->LayerIndex);
  #else
    pContext = (DRIVER_CONTEXT *)pDevice->u.pContext;
    Data.xPos = x;
    Data.yPos = pContext->vySize - pContext->ySize - y;
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
    return 24;
  case LCD_DEVCAP_NUMCOLORS:
    return 0;
  case LCD_DEVCAP_XMAG:
    return 1;
  case LCD_DEVCAP_YMAG:
    return 1;
  case LCD_DEVCAP_MIRROR_X:
    return 0;
  case LCD_DEVCAP_MIRROR_Y:
    return 1;
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
    pContext->vxSizePhys = xSize;
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
      pContext->vxSizePhys = xSize;
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
const GUI_DEVICE_API GUIDRV_Lin_OY_24_API = {
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

void GUIDRV_Lin_OY_24_C(void);   // Avoid empty object files
void GUIDRV_Lin_OY_24_C(void) {}

#endif

/*************************** End of file ****************************/
