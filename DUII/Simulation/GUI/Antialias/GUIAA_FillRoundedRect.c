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
File        : GUIAA_FillRoundedRect.c
Purpose     : Fill rounded rectangle with Antialiasing
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _FillRoundedRect
*/
static void _FillRoundedRect(int x0, int y0, int x1, int y1, int r) {
  int i, x, y;
  int sqmax;
  int yMin, yMax;

  sqmax = r * r + r / 2;
  //
  // First step : find uppermost and lowermost coordinates
  //
  yMin = y0;
  yMax = y1;
  //
  // Use Clipping rect to reduce calculation (if possible)
  //
  if (GUI_Context.pClipRect_HL) {
    if (yMax > GUI_Context.pClipRect_HL->y1) {
      yMax = GUI_Context.pClipRect_HL->y1;
    }
    if (yMin < GUI_Context.pClipRect_HL->y0) {
      yMin = GUI_Context.pClipRect_HL->y0;
    }
  }
  //
  // Draw top half
  //
  for (i = 1, x = r; i < r; i++) {
    y = y0 + r - i;
    if ((y >= yMin) && (y <= yMax)) {
      //
      // Calculate proper x-value
      //
      while ((i * i + x * x) > sqmax) {
        --x;
      }
      LCD_HL_DrawHLine (x0 + r - x, y, x1 - r + x);
    }
  }
  //
  // Draw middle part
  //
  for (i = y0 + r; i < y1 - r; i++) {
    LCD_HL_DrawHLine(x0, i, x1);
  }
  //
  // Draw bottom half
  //
  for (i = 0, x = r; i < r; i++) {
    y = y1 - r + i;
    if ((y >= yMin) && (y <= yMax)) {
      //
      // Calculate proper x-value
      //
      while ((i * i + x * x) > sqmax) {
        --x;
      }
      LCD_HL_DrawHLine (x0 + r - x, y, x1 - r + x);
    }
  }
}

/*********************************************************************
*
*       _FillRoundedRectAA_HiRes
*/
static void _FillRoundedRectAA_HiRes(int x0, int y0, int x1, int y1, int r) {
  //
  // Init AA Subsystem, pass horizontal limits
  //
  GUI_AA_Init_HiRes(x0, x1);
  //
  // Do the actual drawing
  //
  _FillRoundedRect(x0, y0, x1, y1, r);
  //
  // Cleanup
  //
  GUI_AA_Exit();
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_AA_FillRoundedRect
*/
void GUI_AA_FillRoundedRect(int x0, int y0, int x1, int y1, int r) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG_AA(x0,y0);
  #endif
  if (!GUI_Context.AA_HiResEnable) {
    x0 *= GUI_Context.AA_Factor;
    y0 *= GUI_Context.AA_Factor;
    x1 *= GUI_Context.AA_Factor;
    y1 *= GUI_Context.AA_Factor;
    r  *= GUI_Context.AA_Factor;
  }
  #if (GUI_WINSUPPORT)
    Rect.x0 = GUI_AA_HiRes2Pixel(x0 - r);
    Rect.x1 = GUI_AA_HiRes2Pixel(x1 + r);
    Rect.y0 = GUI_AA_HiRes2Pixel(y0 - r);
    Rect.y1 = GUI_AA_HiRes2Pixel(y1 + r);
    WM_ITERATE_START(&Rect); {
  #endif
  _FillRoundedRectAA_HiRes(x0, y0, x1, y1, r);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
