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
File        : GUI_DrawRoundedFrame.c
Purpose     : Implementation of GUI_DrawRoundedFrame
---------------------------END-OF-HEADER------------------------------
*/

#include <math.h>

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawRoundedFrame
*/
static void _DrawRoundedFrame(int x0, int y0, int x1, int y1, int r, int w) {
#if defined(GUI_OPTIMIZE_ROUNDING)

  I32 rr0, rr1, yy;
  int y, c0, c1, d, xs, xe, MinSize;

  if (w < 1) {
    return;
  }
  MinSize = ((y1 - y0 + 1) < (x1 - x0 + 1)) ? y1 - y0 + 1 : x1 - x0 + 1;
  if (MinSize < ((r - 2) << 1)) {
    r = (MinSize >> 1) + 1;
  }
  rr0 = (I32)r * r;
  rr1 = (I32)(r - w) * (r - w);
  for (y = r - 1; y; y--) {
    yy = (I32)y * y;
    c0 = GUI__sqrt32(rr0 - yy);
    if (y < (r - w)) {
      c1 = GUI__sqrt32(rr1 - yy);
      d  = c0 - c1;
    } else {
      d  = c0;
    }
    xs = x0 + r - c0 - 1;
    xe = xs + d - 1;
    LCD_DrawHLine(xs, y0 + (r - y) - 1, xe);
    LCD_DrawHLine(xs, y1 - (r - y) + 1, xe);
    xe = x1 - r + c0 + 1;
    xs = xe - d + 1;
    LCD_DrawHLine(xs, y0 + (r - y) - 1, xe);
    LCD_DrawHLine(xs, y1 - (r - y) + 1, xe);
  }
  r -= 1;
  LCD_FillRect(x0 + r, y0, x1 - r, y0 + w - 1);
  LCD_FillRect(x0 + r, y1 - w + 1, x1 - r, y1);
  LCD_FillRect(x0, y0 + r, x0 + w - 1, y1 - r);
  LCD_FillRect(x1 - w + 1, y0 + r, x1, y1 - r);

#else

  I32 rr0, rr1;
  int i, y, ye, c0, c1, d, xs, xe;

  if (w < 1) {
    return;
  } else {
    if (((y1 - y0 + 1) >= (r << 1)) && ((x1 - x0 + 1) >= (r << 1))) {
      rr0 = (I32)r * r;
      rr1 = (I32)(r - w) * (r - w);
      y  = r;
      for (i = y0; i < (y0 + r); i++, y--) {
        c0 = (int)sqrt((float)(rr0 - (I32)y * y));
        if (y < (r - w)) {
          c1 = GUI__sqrt32(rr1 - (I32)y * y);
          d  = c0 - c1;
        } else {
          d  = c0;
        }
        xs = x0 + r - c0;
        xe = xs + d - 1;
        ye = i;
        LCD_DrawHLine(xs, ye, xe);
        ye = y1 - (i - y0);
        LCD_DrawHLine(xs, ye, xe);
        xe = x1 - r + c0;
        xs = xe - d + 1;
        ye = i;
        LCD_DrawHLine(xs, i, xe);
        ye = y1 - (i - y0);
        LCD_DrawHLine(xs, ye, xe);
      }
      LCD_FillRect(x0 + r, y0, x1 - r, y0 + w - 1);
      LCD_FillRect(x0 + r, y1 - w + 1, x1 - r, y1);
      LCD_FillRect(x0, y0 + r, x0 + w - 1, y1 - r);
      LCD_FillRect(x1 - w + 1, y0 + r, x1, y1 - r);
    }
  }

#endif
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DrawRoundedFrame
*/
void GUI_DrawRoundedFrame(int x0, int y0, int x1, int y1, int r, int w) {
  if (w == 1) {
    GUI_DrawRoundedRect(x0, y0, x1, y1, r);
  } else {
    #if (GUI_WINSUPPORT)
      GUI_RECT Rect;
    #endif
    GUI_LOCK();
    #if (GUI_WINSUPPORT)
      WM_ADDORG(x0,y0);
      WM_ADDORG(x1,y1);
      Rect.x0 = x0; Rect.x1 = x1;
      Rect.y0 = y0; Rect.y1 = y1;
      WM_ITERATE_START(&Rect); {
    #endif
    _DrawRoundedFrame(x0, y0, x1, y1, r, w);
    #if (GUI_WINSUPPORT)
      } WM_ITERATE_END();
    #endif
    GUI_UNLOCK();
  }
}

/*************************** End of file ****************************/
