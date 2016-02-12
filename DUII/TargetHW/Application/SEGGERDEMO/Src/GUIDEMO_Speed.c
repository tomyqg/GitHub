/*********************************************************************
*                SEGGER MICROCONTROLLER SYSTEME GmbH                 *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2004  SEGGER Microcontroller Systeme GmbH        *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

***** emWin - Graphical user interface for embedded applications *****
emWin is protected by international copyright laws.   Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with a license and should not be re-
distributed in any way. We appreciate your understanding and fairness.
----------------------------------------------------------------------
File        : GUIDEMO_Speed.c
Purpose     : Speed demo
----------------------------------------------------------------------
*/

#include <stdlib.h>  /* for rand */
#include "GUI.h"
#include "GUIDEMO.h"

/*********************************************************************
*
*       Static data
*
**********************************************************************
*/
static const GUI_COLOR _aColor[8] = {
  0x000000, 
  0x0000FF, 
  0x00FF00, 
  0x00FFFF, 
  0xFF0000, 
  0xFF00FF, 
  0xFFFF00, 
  0xFFFFFF
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetPixelsPerSecond
*/
static U32 _GetPixelsPerSecond(void) {
  GUI_COLOR Color, BkColor;
  U32 x0, y0, x1, y1, xSize, ySize;
  I32 t, t0;
  U32 Cnt, PixelsPerSecond, PixelCnt;

  //
  // Find an area which is not obstructed by any windows
  //
  xSize = LCD_GetXSize();
  ySize = LCD_GetYSize();
  Cnt = 0;
  x0 = 0;
  x1 = x0 + xSize - 1;
  y0 = 30;
  y1 = y0 + ySize / 2;
  Color = GUI_GetColor();
  BkColor = GUI_GetBkColor();
  GUI_SetColor(BkColor);
  //
  // Repeat fill as often as possible in 100 ms
  //
  t0 = GUI_GetTime();
  do {
    GUI_FillRect(x0, y0, x1, y1);
    Cnt++;
    t = GUI_GetTime();    
  } while ((t - (t0 + 100)) <= 0);
  //
  // Compute result
  //
  t -= t0;
  PixelCnt = (x1 - x0 + 1) * (y1 - y0 + 1) * Cnt;
  PixelsPerSecond = PixelCnt / t * 1000;   
  GUI_SetColor(Color);
  return PixelsPerSecond;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIDEMO_Speed
*/
void GUIDEMO_Speed(void) {
  int t, i;
  U32 PixelsPerSecond;
  unsigned aColorIndex[8];
  int lcd_xsize;
  int lcd_ysize;
  lcd_xsize = LCD_GetXSize();
  lcd_ysize = LCD_GetYSize();
  GUIDEMO_ShowIntro("High speed",
                    "\nmulti layer clipping"
                    "\nhighly optimized drivers"
                    );
  for (i = 0; i< 8; i++) {
    aColorIndex[i] = GUI_Color2Index(_aColor[i]);
  }  
  t = GUI_GetTime();
  for (i = 0; ((GUI_GetTime() - t) < 5000) && !GUIDEMO_CheckCancel(); i++) {
    GUI_RECT r;
    GUI_SetColorIndex(aColorIndex[i&7]);
    /* Calculate random positions */
    r.x0 = rand() % lcd_xsize - lcd_xsize / 2;
    r.y0 = rand() % lcd_ysize - lcd_ysize / 2;
    r.x1 = r.x0 + 20 + rand() % lcd_xsize;
    r.y1 = r.y0 + 20 + rand() % lcd_ysize;
    GUI_FillRect(r.x0, r.y0, r.x1, r.y1);
    /* Clip rectangle to visible area and add the number of pixels (for speed computation) */
    if (r.x1 >= lcd_xsize) {
      r.x1 = lcd_xsize - 1;
    }
    if (r.y1 >= lcd_ysize) {
      r.y1 = lcd_ysize - 1;
    }
    if (r.x0 < 0 ) {
      r.x0 = 0;
    }
    if (r.y1 < 0) {
      r.y1 = 0;
    }
    /* Allow short breaks so we do not use all available CPU time ... */
  }
  GUI_Clear();
  GUIDEMO_NotifyStartNext();
  GUI_SetFont(&GUI_FontComic24B_ASCII);
  PixelsPerSecond = _GetPixelsPerSecond();
  GUI_SetColor(GUI_WHITE);
  GUI_DispStringAt("Pixels/sec: ", 5, (lcd_ysize - GUI_GetFontSizeY()) / 2);
  GUI_DispDecMin(PixelsPerSecond);
  GUIDEMO_Wait();
}
