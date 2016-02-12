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
File        : LCD.c
Purpose     : Link between GUI and LCD_L0
              Performs most of the clipping.
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/

#define RETURN_IF_Y_OUT() \
  if (y < GUI_Context.ClipRect.y0) return;             \
  if (y > GUI_Context.ClipRect.y1) return;

#define RETURN_IF_X_OUT() \
  if (x < GUI_Context.ClipRect.x0) return;             \
  if (x > GUI_Context.ClipRect.x1) return;

#define CLIP_X() \
  if (x0 < GUI_Context.ClipRect.x0) { x0 = GUI_Context.ClipRect.x0; } \
  if (x1 > GUI_Context.ClipRect.x1) { x1 = GUI_Context.ClipRect.x1; }

#define CLIP_Y() \
  if (y0 < GUI_Context.ClipRect.y0) { y0 = GUI_Context.ClipRect.y0; } \
  if (y1 > GUI_Context.ClipRect.y1) { y1 = GUI_Context.ClipRect.y1; }

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _SetBkColor
*/
static void _SetBkColor(GUI_COLOR color) {
  if (GUI_Context.BkColor != color) {
    GUI_Context.BkColor = color;
    LCD_SetBkColorIndex(LCD_Color2Index(color));
  }
}

/*********************************************************************
*
*       _SetColor
*/
static void _SetColor(GUI_COLOR color) {
  if (GUI_Context.Color != color) {
    GUI_Context.Color = color;
    LCD_SetColorIndex(LCD_Color2Index(color));
  }
}

/*********************************************************************
*
*       _SetDrawMode
*/
static LCD_DRAWMODE _SetDrawMode(LCD_DRAWMODE dm) {
  LCD_PIXELINDEX temp;
  LCD_DRAWMODE OldDM;

  OldDM = GUI_Context.DrawMode;
  if ((GUI_Context.DrawMode ^ dm) & LCD_DRAWMODE_REV) {
    temp = LCD__GetBkColorIndex();
    LCD__SetBkColorIndex(LCD__GetColorIndex());
    LCD__SetColorIndex(temp);
  }
  GUI_Context.DrawMode = dm;
  return OldDM;
}

/*********************************************************************
*
*       Static data, API table
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
*       Private data
*
**********************************************************************
*/
const LCD_SET_COLOR_API * LCD__pSetColorAPI = &_SetColorAPI;

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_SetDrawMode
*/
LCD_DRAWMODE LCD_SetDrawMode(LCD_DRAWMODE dm) {
  return LCD__pSetColorAPI->pfSetDrawMode(dm);
}

/*********************************************************************
*
*       LCD_DrawPixel
*/
void LCD_DrawPixel(int x, int y) {
  RETURN_IF_Y_OUT();
  RETURN_IF_X_OUT();
  if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
    LCDDEV_L0_XorPixel(GUI_Context.apDevice[GUI_Context.SelLayer], x, y);
  } else {
    LCDDEV_L0_SetPixelIndex(GUI_Context.apDevice[GUI_Context.SelLayer], x, y, LCD__GetColorIndex());
  }
}

/*********************************************************************
*
*       LCD_DrawHLine
*/
void LCD_DrawHLine(int x0, int y,  int x1) {
  /* Perform clipping and check if there is something to do */
  RETURN_IF_Y_OUT();
  CLIP_X();
  if (x1 < x0) {
    return;
  }
  LCDDEV_L0_DrawHLine(GUI_Context.apDevice[GUI_Context.SelLayer], x0, y, x1);
}

/*********************************************************************
*
*       LCD_DrawVLine
*/
void LCD_DrawVLine(int x, int y0,  int y1) {
  /* Perform clipping and check if there is something to do */
  RETURN_IF_X_OUT();
  CLIP_Y();
  if (y1 < y0) {
    return;
  }
  LCDDEV_L0_DrawVLine(GUI_Context.apDevice[GUI_Context.SelLayer], x, y0, y1);
}

/*********************************************************************
*
*       LCD_FillRect
*/
void LCD_FillRect(int x0, int y0, int x1, int y1) {
  /* Perform clipping and check if there is something to do */
  CLIP_X();
  if (x1 < x0) {
    return;
  }
  CLIP_Y();
  if (y1 < y0) {
    return;
  }
  LCDDEV_L0_FillRect(GUI_Context.apDevice[GUI_Context.SelLayer], x0, y0, x1, y1);
}

/*********************************************************************
*
*       LCD_DrawBitmap
*/
void LCD_DrawBitmap(int x0, int y0, int xsize, int ysize, int xMul, int yMul,
                       int BitsPerPixel, int BytesPerLine,
                       const U8 GUI_UNI_PTR * pPixel, const LCD_PIXELINDEX* pTrans)
{
  U8  Data = 0;
  int x1, y1;
  /* Handle rotation if necessary */
  #if GUI_SUPPORT_ROTATION
  if (GUI_pLCD_APIList) {
    GUI_pLCD_APIList->pfDrawBitmap(x0, y0, xsize, ysize, xMul, yMul, BitsPerPixel, BytesPerLine, pPixel, pTrans);
    return;
  }
  #endif
  y1 = y0 + ysize - 1;
  x1 = x0 + xsize - 1;
  if ((xMul | yMul) == 1) {
    /*  Handle BITMAP without magnification */
    int Diff;
    /*  Clip y0 (top) */
    Diff = GUI_Context.ClipRect.y0 - y0;
    if (Diff > 0) {
      ysize -= Diff;
      if (ysize <= 0) {
		    return;
      }
      y0     = GUI_Context.ClipRect.y0;
      #if GUI_SUPPORT_LARGE_BITMAPS                       /* Required only for 16 bit CPUs if some bitmaps are >64kByte */
        pPixel += (U32)     Diff * (U32)     BytesPerLine;
      #else
        pPixel += (unsigned)Diff * (unsigned)BytesPerLine;
      #endif
    }
    /*  Clip y1 (bottom) */
    Diff = y1 - GUI_Context.ClipRect.y1;
    if (Diff > 0) {
      ysize -= Diff;
      if (ysize <= 0) {
		    return;
      }
    }
    /* Clip right side */
    Diff = x1 - GUI_Context.ClipRect.x1;
    if (Diff > 0) {
      xsize -= Diff;
    }
    /* Clip left side ... (The difficult side ...) */
    Diff = 0;
    if (x0 < GUI_Context.ClipRect.x0) {
      Diff = GUI_Context.ClipRect.x0 - x0;
			xsize -= Diff;
			switch (BitsPerPixel) {
			case 1:
  			pPixel += (Diff >> 3); x0 += (Diff >> 3) << 3; Diff &= 7;
				break;
			case 2:
	  		pPixel += (Diff >> 2); x0 += (Diff >> 2) << 2; Diff &= 3;
				break;
			case 4:
				pPixel += (Diff >> 1); x0 += (Diff >> 1) << 1; Diff &= 1;
				break;
			case 8:
				pPixel += Diff;        x0 += Diff; Diff = 0;
				break;
			case 16:
				pPixel += (Diff << 1); x0 += Diff; Diff = 0;
				break;
			case 24:
			case 32:
				pPixel += (Diff << 2); x0 += Diff; Diff = 0;
				break;
			}
    }
    if (xsize <=0) {
		  return;
    }
    /*
     * Get low level function pointer for drawing already clipped bitmaps
     * and pass the bitmap data to it.
     */
    LCDDEV_L0_DrawBitmap(GUI_Context.apDevice[GUI_Context.SelLayer], x0, y0, xsize, ysize, BitsPerPixel, BytesPerLine, pPixel, Diff, pTrans);
  } else {
    /* Handle BITMAP with magnification */
    int x, y;
    int yi;
    int Shift;
    Shift = 8 - BitsPerPixel;
    for (y = y0, yi = 0; yi < ysize; yi++, y += yMul, pPixel += BytesPerLine) {
      int yMax;
      yMax = y + yMul - 1;
      /* Draw if within clip area (Optimization ... "if" is not required !) */
      if ((yMax >= GUI_Context.ClipRect.y0) && (y <= GUI_Context.ClipRect.y1)) {
        int BitsLeft = 0;
        int xi;
        const U8 GUI_UNI_PTR * pDataLine = pPixel;
        for (x = x0, xi = 0; xi < xsize; xi++, x += xMul) {
          U8 Index;
          if (!BitsLeft) {
            Data = *pDataLine++;
            BitsLeft =8;
          }
          Index = Data >> Shift;
          Data    <<= BitsPerPixel;
          BitsLeft -= BitsPerPixel;
          if (Index || ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0)) {
            LCD_PIXELINDEX  OldColor = LCD__GetColorIndex();
            if (pTrans) {
              LCD__SetColorIndex(*(pTrans + Index));
            } else {
              LCD__SetColorIndex(Index);
            }
            LCD_FillRect(x, y, x + xMul - 1, yMax);
            LCD__SetColorIndex(OldColor);
          }
        }
      }
    }
  }
}

/*********************************************************************
*
*       LCD_SetClipRectMax
*/
void LCD_SetClipRectMax(void) {
  LCDDEV_L0_GetRect(GUI_Context.apDevice[GUI_Context.SelLayer], &GUI_Context.ClipRect);
}

/*********************************************************************
*
*       LCD_Init
*/
int LCD_Init(void) {
  GUI_DEVICE * pDevice;
  int  (* pfInit)(GUI_DEVICE * pDevice);
  void (* pfOn  )(GUI_DEVICE * pDevice);
  int r, i;

  GUI_DEBUG_LOG("\nLCD_Init...");
  LCD_SetClipRectMax();
  r = 0;
  for (i = 0; i < GUI_NUM_LAYERS; i++) {
    pDevice = GUI_DEVICE__GetpDriver(i);
    if (pDevice) {
      pfInit = (int (*)(GUI_DEVICE *))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_INIT);
      if (pfInit) {
        r |= pfInit(pDevice);
      } else {
        return 1;
      }
    }
  }
  LCD_InitLUT();
  #if (GUI_DEFAULT_BKCOLOR != GUI_INVALID_COLOR)
    for (i = GUI_NUM_LAYERS - 1; i >= 0; i--) {
      pDevice = GUI_DEVICE__GetpDriver(i);
      if (pDevice) {
        //
        // Clear video memory
        //
        GUI_SelectLayer(i);
        LCD_SetDrawMode(GUI_DRAWMODE_REV);
        LCD_FillRect(0, 0, GUI_XMAX, GUI_YMAX);
        LCD_SetDrawMode(0);
        //
        // Switch on display
        //
        pfOn = (void (*)(GUI_DEVICE *))pDevice->pDeviceAPI->pfGetDevFunc(&pDevice, LCD_DEVFUNC_ON);
        if (pfOn) {
          pfOn(pDevice);
        }
      }
    }
  #endif
  return r;
}

/*********************************************************************
*
*       LCD_Color2Index
*/
unsigned LCD_Color2Index(LCD_COLOR Color) {
  return LCDDEV_L0_Color2Index(Color);
}

/*********************************************************************
*
*       LCD_Index2Color
*/
LCD_COLOR LCD_Index2Color(int Index) {
  return LCDDEV_L0_Index2Color(Index);
}

/*********************************************************************
*
*       LCD_SetColorIndex
*/
void LCD_SetColorIndex(unsigned PixelIndex) {
  if (GUI_Context.DrawMode & LCD_DRAWMODE_REV) {
    LCD__SetBkColorIndex(PixelIndex);
  } else {
    LCD__SetColorIndex(PixelIndex);
  }
}

/*********************************************************************
*
*       LCD_SetBkColorIndex
*/
void LCD_SetBkColorIndex(unsigned PixelIndex) {
  if (GUI_Context.DrawMode & LCD_DRAWMODE_REV) {
    LCD__SetColorIndex(PixelIndex);
  } else {
    LCD__SetBkColorIndex(PixelIndex);
  }
}

/*********************************************************************
*
*       LCD_SetBkColor
*/
void LCD_SetBkColor(GUI_COLOR color) {
  LCD__pSetColorAPI->pfSetBkColor(color);
}

/*********************************************************************
*
*       LCD_SetColor
*/
void LCD_SetColor(GUI_COLOR color) {
  LCD__pSetColorAPI->pfSetColor(color);
}


/*************************** End of file ****************************/
