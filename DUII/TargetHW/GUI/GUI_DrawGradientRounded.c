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
File        : GUI_DrawGradientRounded.c
Purpose     : Implementation of rounded gradients
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
*       _DrawGradientRounded
*/
static void _DrawGradientRounded(int x0, int y0, int x1, int y1, int rd, GUI_COLOR Color0, GUI_COLOR Color1, void (* pFunc)(int, int, int), int v) {
#if defined(GUI_OPTIMIZE_ROUNDING)

  I32 a, r, g, b, a0, r0, g0, b0, a1, r1, g1, b1, rr;
  int yy, y, d, dx, ySize, ySize1;
  GUI_COLOR Color, OldColor;

  OldColor = GUI_GetColor();
  ySize  = y1 - y0 + 1;
  ySize1 = (ySize - 1) ? ySize - 1 : 1;
  r0 = (Color0      ) & 0xff;
  g0 = (Color0 >>  8) & 0xff;
  b0 = (Color0 >> 16) & 0xff;
  a0 = (Color0 >> 24) & 0xff;
  r1 = (Color1      ) & 0xff;
  g1 = (Color1 >>  8) & 0xff;
  b1 = (Color1 >> 16) & 0xff;
  a1 = (Color1 >> 24) & 0xff;
  rr = (I32)rd * rd;
  for (y = y0; y <= y1; y++) {
    if        ((y - y0) < (rd - 1)) {
      yy = rd - (y - y0) - 1;
    } else if ((y1 - y) < (rd - 1)) {
      yy = rd - (y1 - y) - 1;
    } else {
      yy = 0;
    }
    if (yy) {
      dx = rd - GUI__sqrt32(rr - (I32)yy * yy) - 1;
    } else {
      dx = 0;
    }
    d = (y - y0);
    r = (r0 + ((r1 - r0) * d) / ySize1) & 0xff;
    g = (g0 + ((g1 - g0) * d) / ySize1) & 0xff;
    b = (b0 + ((b1 - b0) * d) / ySize1) & 0xff;
    a = (a0 + ((a1 - a0) * d) / ySize1) & 0xff;
    Color = r | (g << 8) | ((U32)b << 16) | ((U32)a << 24);
    LCD_SetColor(Color);
    if (v) {
      pFunc(x0 + dx, y, x1 - dx);
    } else {
      pFunc(y, x0 + dx, x1 - dx);
    }
  }
  LCD_SetColor(OldColor);

#else

  I32 a, r, g, b, a0, r0, g0, b0, a1, r1, g1, b1, rr;
  int yy, y, d, dx, ySize, ySize1;
  GUI_COLOR Color, OldColor;

  OldColor = GUI_GetColor();
  ySize = y1 - y0 + 1;
  ySize1 = (ySize - 1) ? ySize - 1 : 1;
  r0 = (Color0      ) & 0xff;
  g0 = (Color0 >>  8) & 0xff;
  b0 = (Color0 >> 16) & 0xff;
  a0 = (Color0 >> 24) & 0xff;
  r1 = (Color1      ) & 0xff;
  g1 = (Color1 >>  8) & 0xff;
  b1 = (Color1 >> 16) & 0xff;
  a1 = (Color1 >> 24) & 0xff;
  rr = (I32)rd * rd;
  for (y = y0; y <= y1; y++) {
    if ((y - y0) < rd) {
      yy = rd - (y - y0);
    } else if ((y1 - y) < rd) {
      yy = rd - (y1 - y);
    } else {
      yy = 0;
    }
    if (yy) {
      dx = rd - GUI__sqrt32(rr - (I32)yy * yy);
    } else {
      dx = 0;
    }
    d = (y - y0);
    r = (r0 + ((r1 - r0) * d) / ySize1) & 0xff;
    g = (g0 + ((g1 - g0) * d) / ySize1) & 0xff;
    b = (b0 + ((b1 - b0) * d) / ySize1) & 0xff;
    a = (a0 + ((a1 - a0) * d) / ySize1) & 0xff;
    Color = r | (g << 8) | ((U32)b << 16) | ((U32)a << 24);
    LCD_SetColor(Color);
    if (v) {
      pFunc(x0 + dx, y, x1 - dx);
    } else {
      pFunc(y, x0 + dx, x1 - dx);
    }
  }
  LCD_SetColor(OldColor);

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
*       GUI_DrawGradientRoundedV
*/
void GUI_DrawGradientRoundedV(int x0, int y0, int x1, int y1, int rd, GUI_COLOR Color0, GUI_COLOR Color1) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ADDORG(x1,y1);
    r.x0 = x0;
    r.x1 = x1;
    r.y0 = y0;
    r.y1 = y1;
    WM_ITERATE_START(&r); {
  #endif
  _DrawGradientRounded(x0, y0, x1, y1, rd, Color0, Color1, LCD_DrawHLine, 1);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_DrawGradientRoundedH
*/
void GUI_DrawGradientRoundedH(int x0, int y0, int x1, int y1, int rd, GUI_COLOR Color0, GUI_COLOR Color1) {
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ADDORG(x1,y1);
    r.x0 = x0;
    r.x1 = x1;
    r.y0 = y0;
    r.y1 = y1;
    WM_ITERATE_START(&r); {
  #endif
  _DrawGradientRounded(y0, x0, y1, x1, rd, Color0, Color1, LCD_DrawVLine, 0);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
