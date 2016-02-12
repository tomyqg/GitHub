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
File        : GUI_SPRITE.c
Purpose     : Contains sprite support
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>
#include <string.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SPRITE_LOCK_H(h) (GUI_SPRITE_OBJ *)GUI_LOCK_H(h)

#define SET_SPRITE_LAYER()           \
  Layer    = pDevice->LayerIndex;    \
  LayerOld = GUI_Context.SelLayer;   \
  if (Layer != LayerOld) {           \
    GUI_Context.SelLayer = Layer;    \
  }

#define RESTORE_LAYER()              \
  if (Layer != LayerOld) {           \
    GUI_Context.SelLayer = LayerOld; \
  }

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
// Context used by rectangle calculating
typedef struct {
  GUI_RECT * pRectSprite;
  GUI_RECT RectDraw;
  GUI_RECT RectAll;
  int Cnt;
} CLIP_CONTEXT;

// The sprite object
typedef struct {
  GUI_DEVICE * pDevice;
  GUI_RECT Rect;
  GUI_HMEM hColors;
  GUI_HSPRITE hNext;
  GUI_HSPRITE hPrev;
  U16 Flags;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
} GUI_SPRITE_OBJ;

// Bitmap clipping context
typedef struct {
  int Diff_;
  int x0_;
  int y0_;
  int xSize_;
  int ySize_;
  const U8 GUI_UNI_PTR * pData_;
} CLIP_PARAM;

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetClipRect
*/
static int _GetClipRect(CLIP_CONTEXT * pContext) {
  GUI_RECT * pRectSprite;
  GUI_RECT * pRectDraw;
  int x0, y0, x1, y1;

  pRectSprite = pContext->pRectSprite;
  pRectDraw   = &pContext->RectDraw;
  x0 = pContext->RectAll.x0;
  y0 = pContext->RectAll.y0;
  x1 = pContext->RectAll.x1;
  y1 = pContext->RectAll.y1;
  if (pContext->Cnt == 0) {
    //
    // Check if sprite is completely outside
    //
    if (y1 < pRectSprite->y0) {
      return 0;
    }
    if (x1 < pRectSprite->x0) {
      return 0;
    }
    if (y0 > pRectSprite->y1) {
      return 0;
    }
    if (x0 > pRectSprite->x1) {
      return 0;
    }
    //
    // Part 1, above sprite
    //
    if (y0 < pRectSprite->y0) {
      pRectDraw->y0 = y0;
      pRectDraw->y1 = pRectSprite->y0 - 1;
      pRectDraw->x0 = x0;
      pRectDraw->x1 = x1;
      pContext->Cnt++;
      return 1;
    }
    pContext->Cnt++;
  }
  if (pContext->Cnt == 1) {
    //
    // Part 2, left of sprite
    //
    if (x0 < pRectSprite->x0) {
      pRectDraw->x0 = x0;
      pRectDraw->x1 = pRectSprite->x0 - 1;
      if (y0 < pRectSprite->y0) {
        pRectDraw->y0 = pRectSprite->y0;
      } else {
        pRectDraw->y0 = y0;
      }
      if (y1 > pRectSprite->y1) {
        pRectDraw->y1 = pRectSprite->y1;
      } else {
        pRectDraw->y1 = y1;
      }
      pContext->Cnt++;
      return 1;
    }
    pContext->Cnt++;
  }
  if (pContext->Cnt == 2) {
    //
    // Part 2, right of sprite
    //
    if (x1 > pRectSprite->x1) {
      pRectDraw->x1 = x1;
      pRectDraw->x0 = pRectSprite->x1 + 1;
      if (y0 < pRectSprite->y0) {
        pRectDraw->y0 = pRectSprite->y0;
      } else {
        pRectDraw->y0 = y0;
      }
      if (y1 > pRectSprite->y1) {
        pRectDraw->y1 = pRectSprite->y1;
      } else {
        pRectDraw->y1 = y1;
      }
      pContext->Cnt++;
      return 1;
    }
    pContext->Cnt++;
  }
  if (pContext->Cnt == 3) {
    //
    // Part 4, below sprite
    //
    if (y1 > pRectSprite->y1) {
      pRectDraw->y0 = pRectSprite->y1 + 1;
      pRectDraw->y1 = y1;
      pRectDraw->x0 = x0;
      pRectDraw->x1 = x1;
      pContext->Cnt++;
      return 1;
    }
    pContext->Cnt++;
  }
  return 0; // Nothing else to do...
}

/*********************************************************************
*
*       _GetClipParam
*/
static int _GetClipParam(int x0, int y0,
                         int xSize, int ySize,
                         int BitsPerPixel, 
                         int BytesPerLine,
                         const U8 GUI_UNI_PTR * pData, 
                         int Diff,
                         const GUI_RECT * pRect,
                         CLIP_PARAM * p)
{
  int x1, y1;
  p->x0_    = x0;
  p->y0_    = y0;
  p->xSize_ = xSize;
  p->ySize_ = ySize;
  p->pData_ = pData;
  y1 = y0 + ySize - 1;
  x1 = x0 + xSize - 1;
  // Clip y0 (top)
  p->Diff_ = pRect->y0 - y0;
  if (p->Diff_ > 0) {
    p->ySize_ -= p->Diff_;
    if (p->ySize_ <= 0) {
		  return 1;
    }
    p->y0_     = pRect->y0;
    #if GUI_SUPPORT_LARGE_BITMAPS // Required only for 16 bit CPUs if some bitmaps are >64kByte
      p->pData_ += (U32)     p->Diff_ * (U32)     BytesPerLine;
    #else
      p->pData_ += (unsigned)p->Diff_ * (unsigned)BytesPerLine;
    #endif
  }
  // Clip y1 (bottom)
  p->Diff_ = y1 - pRect->y1;
  if (p->Diff_ > 0) {
    p->ySize_ -= p->Diff_;
    if (p->ySize_ <= 0) {
		  return 1;
    }
  }
  // Clip right side
  p->Diff_ = x1 - pRect->x1;
  if (p->Diff_ > 0) {
    p->xSize_ -= p->Diff_;
  }
  // Clip left side
  p->Diff_ = Diff;
  if (p->x0_ < pRect->x0) {
    p->Diff_ = pRect->x0 - p->x0_;
		p->xSize_ += Diff;
		p->xSize_ -= p->Diff_;
		switch (BitsPerPixel) {
		case 1:
  		p->pData_ += (p->Diff_ >> 3); p->x0_ += (p->Diff_ >> 3) << 3; p->Diff_ &= 7;
			break;
		case 2:
	  	p->pData_ += (p->Diff_ >> 2); p->x0_ += (p->Diff_ >> 2) << 2; p->Diff_ &= 3;
			break;
		case 4:
			p->pData_ += (p->Diff_ >> 1); p->x0_ += (p->Diff_ >> 1) << 1; p->Diff_ &= 1;
			break;
		case 8:
			p->pData_ +=  p->Diff_;       p->x0_ +=  p->Diff_; p->Diff_ = 0;
			break;
		case 16:
			p->pData_ += (p->Diff_ << 1); p->x0_ +=  p->Diff_; p->Diff_ = 0;
			break;
		case 24:
    case 32:
			p->pData_ += (p->Diff_ << 2); p->x0_ +=  p->Diff_; p->Diff_ = 0;
			break;
		}
  }
  if ((p->x0_ + p->Diff_ + p->xSize_) > pRect->x1) {
    p->xSize_ = pRect->x1 - (p->x0_ + p->Diff_) + 1;
  }
  if (p->xSize_ <= 0) {
		return 1;
  }
  return 0;
}

/*********************************************************************
*
*       _PixelExists
*/
static int _PixelExists(int x, int y, GUI_SPRITE_OBJ * pSprite) {
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  pBM = pSprite->pBM;
  if ((x >= pSprite->Rect.x0) && (y >= pSprite->Rect.y0)) {
    if (((x - pSprite->Rect.x0 + 1) <= pBM->XSize) && ((y - pSprite->Rect.y0 + 1) <= pBM->YSize)) {
      if (GUI_GetBitmapPixelIndex(pBM, x - pSprite->Rect.x0, y - pSprite->Rect.y0) != 0) {
        return 1;
      }
    }
  }
  return 0;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawBitmap
*/
static void _DrawBitmap(GUI_DEVICE * pDevice, int x0, int y0,
                       int xSize, int ySize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 * pData, int Diff,
                       const LCD_PIXELINDEX * pTrans) {
  CLIP_CONTEXT Context = {0};
  GUI_SPRITE_OBJ * pSprite;
  CLIP_PARAM Param;
  LCD_PIXELINDEX Index, IndexMask;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  LCD_PIXELINDEX * pSave;
  int x, y, xSizeSprite;
  GUI_RECT Rect;
  GUI_RECT * pRect;

  pSprite             = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite         = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pBM                 = pSprite->pBM;
  Context.pRectSprite = &pSprite->Rect;
  pDevice             = pDevice->pNext;
  Context.RectAll.x0  = x0 + Diff;
  Context.RectAll.y0  = y0;
  Context.RectAll.x1  = x0 + Diff + xSize - 1 ;
  Context.RectAll.y1  = y0 + ySize - 1;
  if (GUI_RectsIntersect(&Context.RectAll, Context.pRectSprite)) {
    //
    // Draw pixels outside of sprite
    //
    while (_GetClipRect(&Context)) {
      if (_GetClipParam(x0, y0, xSize, ySize, BitsPerPixel, BytesPerLine, pData, Diff, &Context.RectDraw, &Param)) {
        continue;
      }
      pDevice->pDeviceAPI->pfDrawBitmap(pDevice, Param.x0_, Param.y0_, Param.xSize_, Param.ySize_, BitsPerPixel, BytesPerLine, Param.pData_, Param.Diff_, pTrans);
    }
    //
    // Draw pixels inside of sprite
    //
    Rect    = *Context.pRectSprite;
    if (GUI_RectsIntersect(&Rect, &Context.RectAll)) {
      GUI__IntersectRect(&Rect, &Context.RectAll);
      pRect  = &Rect;
      if (_GetClipParam(x0, y0, xSize, ySize, BitsPerPixel, BytesPerLine, pData, Diff, pRect, &Param) == 0) {
        IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
        for (y = pRect->y0; y <= pRect->y1; y++) {
          for (x = pRect->x0; x <= pRect->x1; x++) {
            //
            // Get index value of pixel to be drawn
            //
            Index = GUI_GetBitmapPixelIndexEx(BitsPerPixel, BytesPerLine, Param.pData_, x - Param.x0_, y - Param.y0_);
            //
            // Check if sprite pixel exists
            //
            if (GUI_GetBitmapPixelIndex(pBM, x - pSprite->Rect.x0, y - pSprite->Rect.y0) != 0) {
              //
              // Sprite pixel exist on current position, so store output in sprite buffer
              //
              pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
              pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
              if (pTrans) {
                switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
                case 0:
                  *pSave = *(pTrans + Index);
                  break;
                case LCD_DRAWMODE_TRANS:
                  if (Index) {
                    *pSave = *(pTrans + Index);
                  }
                  break;
                case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
                case LCD_DRAWMODE_XOR:
                  *pSave = *(pTrans + Index) ^ IndexMask;
                  break;
                }
              } else {
                switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
                case 0:
                  *pSave = Index;
                  break;
                case LCD_DRAWMODE_TRANS:
                  if (Index) {
                    *pSave = Index;
                  }
                  break;
                case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
                case LCD_DRAWMODE_XOR:
                  *pSave = Index ^ IndexMask;
                  break;
                }
              }
            } else {
              //
              // No sprite pixel on current position, so draw it
              //
              if (pTrans) {
                switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
                case 0:
                  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, *(pTrans + Index));
                  break;
                case LCD_DRAWMODE_TRANS:
                  if (Index) {
                    pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, *(pTrans + Index));
                  }
                  break;
                case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
                case LCD_DRAWMODE_XOR:
                  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, *(pTrans + Index) ^ IndexMask);
                  break;
                }
              } else {
                switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR)) {
                case 0:
                  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, Index);
                  break;
                case LCD_DRAWMODE_TRANS:
                  if (Index) {
                    pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, Index);
                  }
                  break;
                case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
                case LCD_DRAWMODE_XOR:
                  pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, Index ^ IndexMask);
                  break;
                }
              }
            }
          }
        }
      }
    }
  } else {
    pDevice->pDeviceAPI->pfDrawBitmap(pDevice, x0, y0, xSize, ySize, BitsPerPixel, BytesPerLine, pData, Diff, pTrans);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawHLine
*/
static void _DrawHLine(GUI_DEVICE * pDevice, int x0, int y,  int x1) {
  CLIP_CONTEXT Context = {0};
  GUI_SPRITE_OBJ * pSprite;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  LCD_PIXELINDEX * pSave;
  int x, xSizeSprite;
  GUI_RECT Rect;

  pSprite             = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite         = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pBM                 = pSprite->pBM;
  Context.pRectSprite = &pSprite->Rect;
  pDevice             = pDevice->pNext;
  Context.RectAll.x0  = x0;
  Context.RectAll.y0  = y;
  Context.RectAll.x1  = x1;
  Context.RectAll.y1  = y;
  if (GUI_RectsIntersect(&Context.RectAll, Context.pRectSprite)) {
    //
    // Draw pixels outside of sprite
    //
    while (_GetClipRect(&Context)) {
      pDevice->pDeviceAPI->pfDrawHLine(pDevice, Context.RectDraw.x0, Context.RectDraw.y0, Context.RectDraw.x1);
    }
    //
    // Draw pixels inside of sprite
    //
    Rect    = *Context.pRectSprite;
    if (GUI_RectsIntersect(&Rect, &Context.RectAll)) {
      GUI__IntersectRect(&Rect, &Context.RectAll);
      for (x = Rect.x0; x <= Rect.x1; x++) {
        if (GUI_GetBitmapPixelIndex(pBM, x - pSprite->Rect.x0, y - pSprite->Rect.y0) != 0) {
          //
          // Sprite pixel exist on current position, so store output in sprite buffer
          //
          pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
          pSave += xSizeSprite * (Rect.y0 - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
          *pSave = LCD__GetColorIndex();
        } else {
          pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, LCD__GetColorIndex());
        }
      }
    }
  } else {
    pDevice->pDeviceAPI->pfDrawHLine(pDevice, x0, y,  x1);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _DrawVLine
*/
static void _DrawVLine(GUI_DEVICE * pDevice, int x, int y0,  int y1) {
  CLIP_CONTEXT Context = {0};
  GUI_SPRITE_OBJ * pSprite;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  LCD_PIXELINDEX * pSave;
  int y, xSizeSprite;
  GUI_RECT Rect;

  pSprite             = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite         = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pBM                 = pSprite->pBM;
  Context.pRectSprite = &pSprite->Rect;
  pDevice             = pDevice->pNext;
  Context.RectAll.x0  = x;
  Context.RectAll.y0  = y0;
  Context.RectAll.x1  = x;
  Context.RectAll.y1  = y1;
  if (GUI_RectsIntersect(&Context.RectAll, Context.pRectSprite)) {
    //
    // Draw pixels outside of sprite
    //
    while (_GetClipRect(&Context)) {
      pDevice->pDeviceAPI->pfDrawVLine(pDevice, Context.RectDraw.x0, Context.RectDraw.y0, Context.RectDraw.y1);
    }
    //
    // Draw pixels inside of sprite
    //
    Rect    = *Context.pRectSprite;
    if (GUI_RectsIntersect(&Rect, &Context.RectAll)) {
      GUI__IntersectRect(&Rect, &Context.RectAll);
      for (y = Rect.y0; y <= Rect.y1; y++) {
        if (GUI_GetBitmapPixelIndex(pBM, x - pSprite->Rect.x0, y - pSprite->Rect.y0) != 0) {
          //
          // Sprite pixel exist on current position, so store output in sprite buffer
          //
          pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
          pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
          *pSave = LCD__GetColorIndex();
        } else {
          pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, LCD__GetColorIndex());
        }
      }
    }
  } else {
    pDevice->pDeviceAPI->pfDrawVLine(pDevice, x, y0,  y1);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _FillRect
*/
static void _FillRect(GUI_DEVICE * pDevice, int x0, int y0, int x1, int y1) {
  CLIP_CONTEXT Context = {0};
  GUI_SPRITE_OBJ * pSprite;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  LCD_PIXELINDEX * pSave;
  int x, y, xSizeSprite;
  GUI_RECT Rect;
  GUI_RECT * pRect;

  pSprite             = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite         = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pBM                 = pSprite->pBM;
  Context.pRectSprite = &pSprite->Rect;
  pDevice             = pDevice->pNext;
  Context.RectAll.x0  = x0;
  Context.RectAll.y0  = y0;
  Context.RectAll.x1  = x1;
  Context.RectAll.y1  = y1;
  if (GUI_RectsIntersect(&Context.RectAll, Context.pRectSprite)) {
    //
    // Draw pixels outside of sprite
    //
    while (_GetClipRect(&Context)) {
      pDevice->pDeviceAPI->pfFillRect(pDevice, Context.RectDraw.x0, Context.RectDraw.y0, Context.RectDraw.x1, Context.RectDraw.y1);
    }
    //
    // Draw pixels inside of sprite
    //
    Rect    = *Context.pRectSprite;
    if (GUI_RectsIntersect(&Rect, &Context.RectAll)) {
      GUI__IntersectRect(&Rect, &Context.RectAll);
      pRect  = &Rect;
      for (y = pRect->y0; y <= pRect->y1; y++) {
        for (x = pRect->x0; x <= pRect->x1; x++) {
          if (GUI_GetBitmapPixelIndex(pBM, x - pSprite->Rect.x0, y - pSprite->Rect.y0) != 0) {
            //
            // Sprite pixel exist on current position, so store output in sprite buffer
            //
            pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
            pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
            *pSave = LCD__GetColorIndex();
          } else {
            //
            // No sprite pixel on current position, so draw it
            //
            pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, LCD__GetColorIndex());
          }
        }
      }
    }
  } else {
    pDevice->pDeviceAPI->pfFillRect(pDevice, x0, y0, x1, y1);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _GetPixelIndex
*/
static unsigned _GetPixelIndex(GUI_DEVICE * pDevice, int x, int y)  {
  GUI_SPRITE_OBJ * pSprite;
  LCD_PIXELINDEX * pSave;
  LCD_PIXELINDEX Index;
  int xSizeSprite;

  pSprite = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pDevice = pDevice->pNext;
  if (_PixelExists(x, y, pSprite)) {
    //
    // Pixel is inside of sprite, so return value from sprite buffer
    //
    pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
    pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
    Index = *pSave;
  } else {
    //
    // Pixel is outside of sprite, so rout the request to the next layer
    //
    Index = pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
  }
  GUI_UNLOCK_H(pSprite);
  return Index;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _SetPixelIndex
*/
static void _SetPixelIndex(GUI_DEVICE * pDevice, int x, int y, int ColorIndex) {
  GUI_SPRITE_OBJ * pSprite;
  LCD_PIXELINDEX * pSave;
  int xSizeSprite;

  pSprite = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pDevice = pDevice->pNext;
  if (_PixelExists(x, y, pSprite)) {
    //
    // Pixel is inside of sprite, so return value from sprite buffer
    //
    pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
    pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
    *pSave = ColorIndex;
  } else {
    //
    // Pixel is outside of sprite, so rout the request to the next layer
    //
    pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, ColorIndex);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: _XorPixel
*/
static void _XorPixel(GUI_DEVICE * pDevice, int x, int y) {
  GUI_SPRITE_OBJ * pSprite;
  LCD_PIXELINDEX * pSave;
  LCD_PIXELINDEX IndexMask;
  int xSizeSprite;

  pSprite = SPRITE_LOCK_H(pDevice->u.hContext);
  xSizeSprite = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  pDevice = pDevice->pNext;
  if (_PixelExists(x, y, pSprite)) {
    //
    // Pixel is inside of sprite, so manage sprite buffer
    //
    pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
    pSave += xSizeSprite * (y - pSprite->Rect.y0) + (x - pSprite->Rect.x0);
    IndexMask = pDevice->pColorConvAPI->pfGetIndexMask();
    *pSave ^= IndexMask;
  } else {
    //
    // Pixel is outside of sprite, so rout the request to the next layer
    //
    pDevice->pDeviceAPI->pfXorPixel(pDevice, x, y);
  }
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _GetDevData
*/
static void * _GetDevData(GUI_DEVICE * pDevice, int Index) {
  GUI_USE_PARA(pDevice);
  GUI_USE_PARA(Index);
  return NULL;
}

/*********************************************************************
*
*       GUI_DEVICE_API functions: From here there is nothing to do except routing...
*/
static void _SetOrg(GUI_DEVICE * pDevice, int x, int y) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfSetOrg(pDevice, x, y);
  }
}
static void (* _GetDevFunc(GUI_DEVICE ** ppDevice, int Index))(void) {
  GUI_USE_PARA(ppDevice);
  GUI_USE_PARA(Index);
  return NULL;
}
static I32 _GetDevProp(GUI_DEVICE * pDevice, int Index) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    return pDevice->pDeviceAPI->pfGetDevProp(pDevice, Index);
  }
  return 0;
}
static void _GetRect(GUI_DEVICE * pDevice, LCD_RECT * pRect) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfGetRect(pDevice, pRect);
  }
}

/*********************************************************************
*
*       GUI_DEVICE_API structure
*/
static const GUI_DEVICE_API _Sprite_API = {
  //
  // Data
  //
  DEVICE_CLASS_SPRITE,
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
*       _GetBkPixel
*
* Purpose:
*   Returns the background value of the given position.
*/
static int _GetBkPixel(int x, int y, GUI_DEVICE * pDevice, GUI_SPRITE_OBJ * pSprite) {
  LCD_PIXELINDEX Index;
  LCD_PIXELINDEX * pSave;
  int xPos, yPos, xSizeSprite;

  if ((x >= pSprite->Rect.x0) && (x <= pSprite->Rect.x1) && (y >= pSprite->Rect.y0) && (y <= pSprite->Rect.y1)) {
    xPos = x - pSprite->Rect.x0;
    yPos = y - pSprite->Rect.y0;
    Index = GUI_GetBitmapPixelIndex(pSprite->pBM, xPos, yPos);
  } else {
    Index = 0;
  }
  if (Index == 0) {
    pDevice = pDevice->pNext;
    if (pDevice) {
      return pDevice->pDeviceAPI->pfGetPixelIndex(pDevice, x, y);
    }
    return 0;
  } else {
    xSizeSprite = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
    pSave = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
    pSave += xSizeSprite * yPos + xPos;
    Index = *pSave;
    return Index;
  }
}

/*********************************************************************
*
*       _SetSpriteBkPixel
*
* Purpose:
*   This routine is used to draw the sprite pixels itself. It temporary
*   removes the bitmap of the given sprite and uses _SetBkPixel() for the 
*   drawing operation. This makes sure that the background will be managed 
*   in exact the same manner as in normal drawing operations.
*/
static void _SetSpriteBkPixel(int x, int y, int Index, GUI_DEVICE * pDevice) {
  pDevice = pDevice->pNext;
  if (pDevice) {
    pDevice->pDeviceAPI->pfSetPixelIndex(pDevice, x, y, Index);
  }
}

/*********************************************************************
*
*       _ClipRectEx
*/
static void _ClipRectEx(GUI_DEVICE * pDevice, GUI_RECT * pRect) {
  GUI_RECT RectDevice;
  pDevice->pDeviceAPI->pfGetRect(pDevice, &RectDevice);
  GUI__IntersectRect(pRect, &RectDevice);
}

/*********************************************************************
*
*       _Undraw
*
* Purpose:
*   This routine removes the drawing of the sprite from the screen. It
*   replaces the non transparent sprite pixels with the background stored
*   in the sprite data area. If a non transparent sprite pixel is covered
*   by a further sprite the routine makes sure that the background data
*   is passed to the right sprite.
*/
static void _Undraw(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;
  int x, y, xPos, yPos;
  int Layer, LayerOld;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  LCD_PIXELINDEX Data;
  LCD_PIXELINDEX * pData;
  GUI_DEVICE * pDevice;
  GUI_RECT ClipRect;

  pSprite = SPRITE_LOCK_H(hSprite);
  pDevice = pSprite->pDevice;
  SET_SPRITE_LAYER()
  pBM   = pSprite->pBM;
  pData = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
  ClipRect = pSprite->Rect;
  _ClipRectEx(pDevice, &ClipRect);
  for (y = ClipRect.y0; y <= ClipRect.y1; y++) {
    yPos = y - pSprite->Rect.y0;
    for (x = ClipRect.x0; x <= ClipRect.x1; x++) {
      xPos = x - pSprite->Rect.x0;
      if (GUI_GetBitmapPixelIndex(pBM, xPos, yPos) != 0) {
        Data = *(pData + yPos * pBM->XSize + xPos);
        _SetSpriteBkPixel(x, y, Data, pDevice);
      }
    }
  }
  GUI_UNLOCK_H(pSprite);
  RESTORE_LAYER()
}

/*********************************************************************
*
*       _SetBitmap
*/
static void _SetBitmap(GUI_HSPRITE hSprite, const GUI_BITMAP GUI_UNI_PTR * pBM) {
  GUI_SPRITE_OBJ * pSprite;
  int xPos, yPos, x, y, Index;
  int PixelOld, PixelNew;
  int Layer, LayerOld;
  LCD_PIXELINDEX * pData;
  LCD_PIXELINDEX * pColors;
  GUI_DEVICE * pDevice;
  GUI_RECT ClipRect;

  pSprite = SPRITE_LOCK_H(hSprite);
  pDevice = pSprite->pDevice;
  SET_SPRITE_LAYER()
  pData   = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
  pColors = (LCD_PIXELINDEX *)GUI_LOCK_H(pSprite->hColors);
  ClipRect = pSprite->Rect;
  _ClipRectEx(pDevice, &ClipRect);
  for (y = ClipRect.y0; y <= ClipRect.y1; y++) {
    yPos = y - pSprite->Rect.y0;
    for (x = ClipRect.x0; x <= ClipRect.x1; x++) {
      xPos = x - pSprite->Rect.x0;
      PixelOld = GUI_GetBitmapPixelIndex(pSprite->pBM, xPos, yPos);
      PixelNew = GUI_GetBitmapPixelIndex(pBM, xPos, yPos);
      if (PixelNew == 0) {
        if (PixelOld) {
          //
          // Restore background
          //
          Index = *(pData + yPos * pBM->XSize + xPos);
          _SetSpriteBkPixel(x, y, Index, pDevice);
        }
      } else {
        //
        // Save background
        //
        *(pData + yPos * pBM->XSize + xPos) = _GetBkPixel(x, y, pDevice, pSprite);
        //
        // Draw new pixel
        //
        Index = *(pColors + PixelNew);
        _SetSpriteBkPixel(x, y, Index, pDevice);
      }
    }
  }
  pSprite->pBM = pBM;
  GUI_UNLOCK_H(pSprite);
  GUI_UNLOCK_H(pColors);
  RESTORE_LAYER()
}

/*********************************************************************
*
*       _SetPosition
*
* Purpose:
*   Sets the new position of the given sprite and restores the background
*   on demand.
*/
static void _SetPosition(GUI_HSPRITE hSprite, int xNewPos, int yNewPos) {
  GUI_SPRITE_OBJ * pSprite;
  int x, xPos, xPosBK, xStart, xStep, xEnd, xOff, xOverlapMin, xOverlapMax;
  int y, yPos, yPosBK, yStart, yStep, yEnd, yOff, yOverlapMin, yOverlapMax;
  int xSize, ySize;
  int Layer, LayerOld;
  LCD_PIXELINDEX * pData;
  LCD_PIXELINDEX * pColors;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  GUI_DEVICE * pDevice;
  GUI_RECT ClipRect;

  pSprite = SPRITE_LOCK_H(hSprite);
  pDevice = pSprite->pDevice;
  if ((xNewPos == pSprite->Rect.x0) && (yNewPos == pSprite->Rect.y0)) {
    GUI_UNLOCK_H(pSprite);
    return;
  }
  SET_SPRITE_LAYER()
  xPos  = pSprite->Rect.x0;
  yPos  = pSprite->Rect.y0;
  xSize = pSprite->Rect.x1 - xPos + 1;
  ySize = pSprite->Rect.y1 - yPos + 1;
  if (pSprite->Flags & GUI_SPRITE_CF_SHOW) {
    pBM   = pSprite->pBM;
    pData = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
    xOff  = xNewPos - xPos;
    yOff  = yNewPos - yPos;
    if (xOff > 0) {
      xStep  = 1;
      xStart = 0;
      xEnd   = xSize;
      xOverlapMax = xEnd - 1;
      xOverlapMin = xOff;
    } else {
      xStep  = -1;
      xStart = xSize - 1;
      xEnd   = -1;
      xOverlapMin = 0;
      xOverlapMax = xStart + xOff;
    }
    if (yOff > 0) {
      yStep  = 1;
      yStart = 0;
      yEnd   = ySize;
      yOverlapMax = yEnd - 1;
      yOverlapMin = yOff;
    } else {
      yStep  = -1;
      yStart = ySize - 1;
      yEnd   = -1;
      yOverlapMin = 0;
      yOverlapMax = yStart + yOff;
    }
    pColors = (LCD_PIXELINDEX *)GUI_LOCK_H(pSprite->hColors);
    pDevice->pDeviceAPI->pfGetRect(pDevice, &ClipRect);
    for (y = yStart; y != yEnd; y += yStep) {
      char yOverlaps;
      int yNew = yPos + y + yOff;
      yOverlaps    = (y >= yOverlapMin) && (y <= yOverlapMax);
      for (x = xStart; x != xEnd; x += xStep) {
        char xyOverlaps;
        int BitmapPixel;
        int BkPixel;
        LCD_PIXELINDEX * pSave;
        int xNew;

        BkPixel = 0;
        BitmapPixel = GUI_GetBitmapPixelIndex(pBM, x, y);
        //
        // Manage pixel only if it is not transparent
        //
        if (BitmapPixel) {
          LCD_PIXELINDEX NewPixel;
          int DoSetSpriteBkPixel;
          pSave      = pData + x + y * xSize;
          xNew       = xPos + x + xOff;
          xyOverlaps = (x >= xOverlapMin) && (x <= xOverlapMax) && yOverlaps;
          //
          // Get background pixel of new position
          //
          if ((xNew >= ClipRect.x0) && (yNew >= ClipRect.y0) && (xNew <= ClipRect.x1) && (yNew <= ClipRect.y1)) {
            BkPixel = _GetBkPixel(xNew, yNew, pDevice, pSprite);
          }
          //
          // Restore old pixel if it was not transparent
          //
          DoSetSpriteBkPixel = 0;
          if (!xyOverlaps) {
            DoSetSpriteBkPixel = 1;
          } else {
            if (((x - xOff) >= 0) && ((y - yOff) >= 0)) {
              if (GUI_GetBitmapPixelIndex(pBM, x - xOff, y - yOff) == 0) {
                DoSetSpriteBkPixel = 1;
              }
            }
          }
          if (DoSetSpriteBkPixel) {
            xPosBK = x + xPos;
            yPosBK = y + yPos;
            if ((xPosBK >= ClipRect.x0) && (yPosBK >= ClipRect.y0) && (xPosBK <= ClipRect.x1) && (yPosBK <= ClipRect.y1)) {
              _SetSpriteBkPixel(xPosBK, yPosBK, *(pSave), pDevice);
            }
          }
          //
          // Draw new pixel
          //
          if ((xNew >= ClipRect.x0) && (yNew >= ClipRect.y0) && (xNew <= ClipRect.x1) && (yNew <= ClipRect.y1)) {
            NewPixel = *(pColors + BitmapPixel);
            _SetSpriteBkPixel(xNew, yNew, NewPixel, pDevice);
          }
          //
          // Save background pixel
          //
          *pSave = BkPixel;
        }
      }
    }
    GUI_UNLOCK_H(pColors);
  }
  //
  // Adjust position of sprite
  //
  pSprite->Rect.x0 = xNewPos;
  pSprite->Rect.y0 = yNewPos;
  pSprite->Rect.x1 = xNewPos + xSize - 1;
  pSprite->Rect.y1 = yNewPos + ySize - 1;
  RESTORE_LAYER()
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _SetBitmapAndPosition
*/
static void _SetBitmapAndPosition(GUI_HSPRITE hSprite, const GUI_BITMAP GUI_UNI_PTR * pBM_New, int xOff, int yOff) {
  GUI_SPRITE_OBJ * pSprite;
  int xSize, ySize;
  int x, y;
  int xStart, xEnd, yStart, yEnd;
  int xInc, yInc;
  const GUI_BITMAP GUI_UNI_PTR * pBM_Old;
  LCD_PIXELINDEX * pData;
  LCD_PIXELINDEX * pColors;
  GUI_DEVICE * pDevice;
  GUI_RECT ClipRect;
  int xPos, yPos;
  int Pixel, Overlap, Undraw;
  int PixelOld, PixelNew;
  int Index, BkIndex;
  int Layer, LayerOld;
  LCD_PIXELINDEX Data;

  pSprite = SPRITE_LOCK_H(hSprite);
  pDevice = pSprite->pDevice;
  SET_SPRITE_LAYER()
  pBM_Old   = pSprite->pBM;
  pData = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
  xSize = pSprite->Rect.x1 - pSprite->Rect.x0 + 1;
  ySize = pSprite->Rect.y1 - pSprite->Rect.y0 + 1;
  if (yOff < 0) {
    yStart = ySize - 1;
    yEnd   = -1;
    yInc   = -1;
  } else {
    yStart = 0;
    yEnd   = ySize;
    yInc   = 1;
  }
  if (xOff < 0) {
    xStart = xSize - 1;
    xEnd   = -1;
    xInc   = -1;
  } else {
    xStart = 0;
    xEnd   = xSize;
    xInc   = 1;
  }
  pColors = (LCD_PIXELINDEX *)GUI_LOCK_H(pSprite->hColors);
  pDevice->pDeviceAPI->pfGetRect(pDevice, &ClipRect);
  for (y = yStart; y != yEnd; y += yInc) {
    for (x = xStart; x != xEnd; x += xInc) {
      Undraw = 1;
      PixelOld = GUI_GetBitmapPixelIndex(pBM_Old, x, y);
      PixelNew = GUI_GetBitmapPixelIndex(pBM_New, x, y);
      xPos = x - xOff;
      yPos = y - yOff;
      Overlap = (xPos >= 0) && (yPos >= 0) && (xPos < pBM_New->XSize) && (yPos < pBM_New->YSize);
      //
      // Restore background
      //
      if (PixelOld) {
        if (Overlap) {
          Pixel = GUI_GetBitmapPixelIndex(pBM_New, xPos, yPos);
          if (Pixel) {
            Undraw = 0;
          }
        }
        if (Undraw) {
          xPos = pSprite->Rect.x0 + x;
          yPos = pSprite->Rect.y0 + y;
          if ((xPos >= ClipRect.x0) && (yPos >= ClipRect.y0) && (xPos <= ClipRect.x1) && (yPos <= ClipRect.y1)) {
            Data = *(pData + y * xSize + x);
            _SetSpriteBkPixel(pSprite->Rect.x0 + x, pSprite->Rect.y0 + y, Data, pDevice);
          }
        }
      }
      //
      // Draw new pixel and save background data
      //
      if (PixelNew) {
        xPos = pSprite->Rect.x0 + x + xOff;
        yPos = pSprite->Rect.y0 + y + yOff;
        if ((xPos >= ClipRect.x0) && (yPos >= ClipRect.y0) && (xPos <= ClipRect.x1) && (yPos <= ClipRect.y1)) {
          BkIndex = _GetBkPixel(xPos, yPos, pDevice, pSprite);
          Index = *(pColors + PixelNew);
          _SetSpriteBkPixel(xPos, yPos, Index, pDevice);
          *(pData + y * xSize + x) = BkIndex;
        }
      }
    }
  }
  GUI_UNLOCK_H(pColors);
  pSprite->pBM = pBM_New;
  pSprite->Rect.x0 += xOff;
  pSprite->Rect.x1 += xOff;
  pSprite->Rect.y0 += yOff;
  pSprite->Rect.y1 += yOff;
  RESTORE_LAYER()
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _SpriteDraw
*/
static void _SpriteDraw(GUI_HSPRITE hSprite) {
  GUI_RECT ClipRect;
  GUI_SPRITE_OBJ * pSprite;
  int xPos, yPos, x, y, Index;
  int Layer, LayerOld;
  int PixelNew;
  LCD_PIXELINDEX * pData;
  LCD_PIXELINDEX * pColors;
  const GUI_BITMAP GUI_UNI_PTR * pBM;
  GUI_DEVICE * pDevice;

  pSprite = SPRITE_LOCK_H(hSprite);
  pDevice = pSprite->pDevice;
  SET_SPRITE_LAYER()
  pBM = pSprite->pBM;
  pData = (LCD_PIXELINDEX *)((U8 *)pSprite + sizeof(GUI_SPRITE_OBJ));
  pSprite->pBM = NULL; // Temporary remove bitmap to make sure not the background of the sprite itself will be saved
  pColors = (LCD_PIXELINDEX *)GUI_LOCK_H(pSprite->hColors);
  ClipRect = pSprite->Rect;
  _ClipRectEx(pDevice, &ClipRect);
  for (y = ClipRect.y0; y <= ClipRect.y1; y++) {
    yPos = y - pSprite->Rect.y0;
    for (x = ClipRect.x0; x <= ClipRect.x1; x++) {
      xPos = x - pSprite->Rect.x0;
      PixelNew = GUI_GetBitmapPixelIndex(pBM, xPos, yPos);
      if (PixelNew) {
        //
        // Save background
        //
        *(pData + yPos * pBM->XSize + xPos) = _GetBkPixel(x, y, pDevice, pSprite);
        //
        // Draw new pixel
        //
        Index = *(pColors + PixelNew);
        _SetSpriteBkPixel(x, y, Index, pDevice);
      }
    }
  }
  GUI_UNLOCK_H(pColors);
  pSprite->pBM = pBM;
  RESTORE_LAYER()
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _Link
*
* Purpose:
*   Inserts the given sprite into the list. Sprites with non STAYONTOP attribute
*   are placed directly after the last sprite with STAYONTOP attribute. New
*   sprites with STAYONTOP attribute are placed at the top of the list.
*/
static void _Link(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;

  pSprite = SPRITE_LOCK_H(hSprite);
  GUI_DEVICE_Link(pSprite->pDevice);
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _Unlink
*/
static void _Unlink(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;

  pSprite = SPRITE_LOCK_H(hSprite);
  GUI_DEVICE_Unlink(pSprite->pDevice);
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       _DoColorConversion
*/
static int _DoColorConversion(GUI_HSPRITE hSprite, const GUI_BITMAP GUI_UNI_PTR * pBM) {
  GUI_SPRITE_OBJ * pSprite;
  LCD_PIXELINDEX * pColors;
  int i;
  int Layer;
  GUI_HMEM hColors;
  pSprite = SPRITE_LOCK_H(hSprite);
  hColors = pSprite->hColors;
  Layer = pSprite->pDevice->LayerIndex;
  GUI_UNLOCK_H(pSprite);
  if (hColors) {
    GUI_ALLOC_Free(hColors);
  }
  hColors = GUI_ALLOC_AllocZero(sizeof(LCD_PIXELINDEX) * pBM->pPal->NumEntries);
  if (hColors == 0) {
    return 1;
  }
  pColors = (LCD_PIXELINDEX *)GUI_LOCK_H(hColors);
  for (i = 0; i < pBM->pPal->NumEntries; i++) {
    LCD_COLOR Color;
    LCD_PIXELINDEX Index;
    Color = *(pBM->pPal->pPalEntries + i);
    Index = GUI_Context.apDevice[Layer]->pColorConvAPI->pfColor2Index(Color);
    *(pColors++) = Index;
  }
  GUI_UNLOCK_H(pColors);
  pSprite = SPRITE_LOCK_H(hSprite);
  pSprite->hColors = hColors;
  GUI_UNLOCK_H(pSprite);
  return 0;
}

/*********************************************************************
*
*       _Delete
*/
static void _Delete(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;
  pSprite = SPRITE_LOCK_H(hSprite);
  GUI_ALLOC_FreeFixedBlock(pSprite->pDevice);
  GUI_ALLOC_Free(pSprite->hColors); // Remove colors from memory
  GUI_ALLOC_Free(hSprite);          // Remove sprite from memory
  GUI_UNLOCK_H(pSprite);
}

/*********************************************************************
*
*       Private code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SPRITE__CreateEx
*/
GUI_HSPRITE GUI_SPRITE__CreateEx(const GUI_BITMAP GUI_UNI_PTR * pBM, int x, int y, int Layer, U16 Flags) {
  GUI_DEVICE * pDevice;        // Pointer to device object
  GUI_HSPRITE      hSpriteNew; // Handle of new sprite
  GUI_SPRITE_OBJ * pSpriteNew; // Pointer to new sprite object
  int AllocSize;
  U16 FlagsDevice;

  if ((pBM->XSize == 0) || (pBM->YSize == 0)) {
    return 0; // Error, Zerosized sprites make no sense...
  }
  if (pBM->BitsPerPixel > 8) {
    return 0; // Error, Only pallete based bitmaps are currently supported...
  }
  if (pBM->pPal->pPalEntries == NULL) {
    return 0; // Error, Only device independent bitmaps are currently supported...
  }
  //
  // Create device object
  //
  FlagsDevice = (Flags & GUI_SPRITE_CF_STAYONTOP) ? GUI_DEVICE_STAYONTOP : 0;
  pDevice = GUI_DEVICE_Create(&_Sprite_API, NULL, FlagsDevice, Layer);
  if (pDevice == NULL) {
    return 0; // Error
  }
  //
  // Create sprite object
  //
  AllocSize = sizeof(LCD_PIXELINDEX) * pBM->XSize * pBM->YSize; // Memory area for saving background data
  GUI_LOCK();
  hSpriteNew = GUI_ALLOC_AllocZero(sizeof(GUI_SPRITE_OBJ) + AllocSize);
  if (hSpriteNew == 0) {
    GUI_DEVICE_Delete(pDevice);
    GUI_UNLOCK();
    return 0; // Error, Allocation failed...
  }
  pDevice->u.hContext = hSpriteNew;
  //
  // Create and initialize object
  //
  pSpriteNew = SPRITE_LOCK_H(hSpriteNew);
  pSpriteNew->Flags   = Flags;
  pSpriteNew->pBM     = pBM;
  pSpriteNew->Rect.x0 = x;
  pSpriteNew->Rect.y0 = y;
  pSpriteNew->Rect.x1 = pSpriteNew->Rect.x0 + pBM->XSize - 1;
  pSpriteNew->Rect.y1 = pSpriteNew->Rect.y0 + pBM->YSize - 1;
  pSpriteNew->pDevice = pDevice;
  GUI_UNLOCK_H(pSpriteNew);
  //
  // Do the color conversion
  //
  if (_DoColorConversion(hSpriteNew, pBM)) { // Set bitmap data
    GUI_UNLOCK();
    return 0; // Error, Allocation failed...
  };
  //
  // Linking & Drawing
  //
  if (Flags & GUI_SPRITE_CF_SHOW) {
    _Link(hSpriteNew);        // Put it into linked list
    _SpriteDraw(hSpriteNew);  // Draw it
  }
  GUI_UNLOCK();
  return hSpriteNew;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_SPRITE_CreateEx
*/
GUI_HSPRITE GUI_SPRITE_CreateEx(const GUI_BITMAP GUI_UNI_PTR * pBM, int x, int y, int Layer) {
  return GUI_SPRITE__CreateEx(pBM, x, y, Layer, GUI_SPRITE_CF_SHOW);
}

/*********************************************************************
*
*       GUI_SPRITE_Create
*/
GUI_HSPRITE GUI_SPRITE_Create(const GUI_BITMAP GUI_UNI_PTR * pBM, int x, int y) {
  return GUI_SPRITE_CreateEx(pBM, x, y, GUI_Context.SelLayer);
}

/*********************************************************************
*
*       GUI_SPRITE_Delete
*/
void GUI_SPRITE_Delete(GUI_HSPRITE hSprite) {
  if (hSprite) {
    GUI_LOCK();
    _Undraw(hSprite);        // Remove from screen
    _Unlink(hSprite);        // Remove from linked list
    _Delete(hSprite);        // Remove from memory
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_SPRITE_SetPosition
*/
void GUI_SPRITE_SetPosition(GUI_HSPRITE hSprite, int x, int y) {
  GUI_LOCK();
  _SetPosition(hSprite, x, y);
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_SPRITE_SetBitmap
*/
int GUI_SPRITE_SetBitmap(GUI_HSPRITE hSprite, const GUI_BITMAP GUI_UNI_PTR * pBM) {
  int r = 1;
  int XSize, YSize;
  GUI_SPRITE_OBJ * pSprite;
  if (hSprite) {
    GUI_LOCK();
    pSprite = SPRITE_LOCK_H(hSprite);
    XSize = pSprite->pBM->XSize;
    YSize = pSprite->pBM->YSize;
    GUI_UNLOCK_H(pSprite);
    //
    // Only bitmaps of the same size are supported by this function
    //
    if ((XSize == pBM->XSize) && (YSize == pBM->YSize)) {
      //
      // Do the color conversion
      //
      if (_DoColorConversion(hSprite, pBM) == 0) {
        //
        // Set new image
        //
        _SetBitmap(hSprite, pBM);
        r = 0;
      }
    }
    GUI_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       GUI_SPRITE_SetBitmapAndPosition
*/
int GUI_SPRITE_SetBitmapAndPosition(GUI_HSPRITE hSprite, const GUI_BITMAP GUI_UNI_PTR * pBM, int x, int y) {
  int r = 1;
  int XSize, YSize, x0, y0;
  GUI_SPRITE_OBJ * pSprite;
  if (hSprite) {
    GUI_LOCK();
    pSprite = SPRITE_LOCK_H(hSprite);
    XSize = pSprite->pBM->XSize;
    YSize = pSprite->pBM->YSize;
    x0 = pSprite->Rect.x0;
    y0 = pSprite->Rect.y0;
    GUI_UNLOCK_H(pSprite);
    //
    // Only bitmaps of the same size are supported by this function
    //
    if ((XSize == pBM->XSize) && (YSize == pBM->YSize)) {
      //
      // Do the color conversion
      //
      if (_DoColorConversion(hSprite, pBM) == 0) {
        //
        // Set new image
        //
        _SetBitmapAndPosition(hSprite, pBM, x - x0, y - y0);
        r = 0;
      }
    }
    GUI_UNLOCK();
  }
  return r;
}

/*********************************************************************
*
*       GUI_SPRITE_Hide
*/
void GUI_SPRITE_Hide(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;
  if (hSprite) {
    GUI_LOCK();
    pSprite = SPRITE_LOCK_H(hSprite);
    if ((pSprite->Flags & GUI_SPRITE_CF_SHOW) != 0) {
      _Undraw(hSprite);          // Remove from screen
      _Unlink(hSprite);          // Remove from linked list
      pSprite->Flags &= ~GUI_SPRITE_CF_SHOW;
    }
    GUI_UNLOCK_H(pSprite);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_SPRITE_Show
*/
void GUI_SPRITE_Show(GUI_HSPRITE hSprite) {
  GUI_SPRITE_OBJ * pSprite;
  if (hSprite) {
    GUI_LOCK();
    pSprite = SPRITE_LOCK_H(hSprite);
    if ((pSprite->Flags & GUI_SPRITE_CF_SHOW) == 0) {
      _Link(hSprite);            // Put it into linked list
      _SpriteDraw(hSprite);      // Draw it
      pSprite->Flags |= GUI_SPRITE_CF_SHOW;
    }
    GUI_UNLOCK_H(pSprite);
    GUI_UNLOCK();
  }
}

/*********************************************************************
*
*       GUI_SPRITE_GetState
*/
int GUI_SPRITE_GetState(GUI_HSPRITE hSprite) {
  int r = 0;
  GUI_SPRITE_OBJ * pSprite;
  if (hSprite) {
    GUI_LOCK();
    pSprite = SPRITE_LOCK_H(hSprite);
    r = (pSprite->Flags & GUI_SPRITE_CF_SHOW) ? 1 : 0;
    GUI_UNLOCK_H(pSprite);
    GUI_UNLOCK();
  }
  return r;
}

/*************************** End of file ****************************/
