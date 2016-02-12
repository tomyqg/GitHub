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
File        : GUI_Circ.c
Purpose     : Circle and ellipse drawing functions
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>       // Needed for definition of NULL

#include "GUI_Private.h"

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       Draw8Point
*/
static void Draw8Point(int x0,int y0, int xoff, int yoff) {
  LCD_HL_DrawPixel(x0 + xoff, y0 + yoff);
  LCD_HL_DrawPixel(x0 - xoff, y0 + yoff);
  LCD_HL_DrawPixel(x0 + yoff, y0 + xoff);
  LCD_HL_DrawPixel(x0 + yoff, y0 - xoff);
  if (yoff) {
    LCD_HL_DrawPixel(x0 + xoff , y0 - yoff);
    LCD_HL_DrawPixel(x0 - xoff , y0 - yoff);
    LCD_HL_DrawPixel(x0 - yoff , y0 + xoff);
    LCD_HL_DrawPixel(x0 - yoff , y0 - xoff);
  }
}

/*********************************************************************
*
*       Public code, circle
*
**********************************************************************
*/
/*********************************************************************
*
*       GL_DrawCircle
*/
void GL_DrawCircle(int x0, int y0, int r) {
  I32 i;
  int imax;
  I32 sqmax;
  I32 y;
  imax  = ((I32)    ((I32)r * 707))  / 1000 + 1;
  sqmax =  (I32)r *  (I32)r + (I32)r / 2;
  y     = r;
  Draw8Point(x0, y0, r, 0);
  for (i = 1; i <= imax; i++) {
    if ((i * i + y * y) > sqmax) {
      Draw8Point(x0, y0, i, y);
      y--;
    }
    Draw8Point(x0, y0, i, y);
  }
}

/*********************************************************************
*
*       GUI_DrawCircle
*/
void GUI_DrawCircle(int x0, int y0, int r) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0, y0);
    Rect.x0 = x0 - r;
    Rect.x1 = x0 + r;
    Rect.y0 = y0 - r;
    Rect.y1 = y0 + r;
    WM_ITERATE_START(&Rect); {
  #endif
    GL_DrawCircle(x0, y0, r);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GL_FillCircle
*/
void GL_FillCircle(int x0, int y0, int r) {
  I32 i;
  int imax;
  I32 sqmax;
  I32 x;
  imax  = ((I32)    ((I32)r * 707))  / 1000 + 1;
  sqmax =  (I32)r *  (I32)r + (I32)r / 2;
  x     = r;
  LCD_HL_DrawHLine(x0 - r, y0, x0 + r);
  for (i = 1; i <= imax; i++) {
    if ((i * i + x * x) > sqmax) {
      //
      // Draw lines from outside
      //
      if (x > imax) {
        LCD_HL_DrawHLine (x0 - i + 1, y0 + x, x0 + i - 1);
        LCD_HL_DrawHLine (x0 - i + 1, y0 - x, x0 + i - 1);
      }
      x--;
    }
    //
    // Draw lines from inside (center)
    //
    LCD_HL_DrawHLine(x0 - x, y0 + i, x0 + x);
    LCD_HL_DrawHLine(x0 - x, y0 - i, x0 + x);
  }
}

/*********************************************************************
*
*       GUI_FillCircle
*/
void GUI_FillCircle(int x0, int y0, int r) {
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0, y0);
    WM_ITERATE_START(NULL); {
  #endif
  GL_FillCircle(x0, y0, r);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       Public code, ellipse
*
* To calculate the ellipse positions we use the following equation:
*
*   x^2   y^2
*   --- + --- = 1
*   a^2   b^2
*
* As a, b and y is known the equation has to be solved for x:
*
*           (       y^2 * a^2 )
*   x = sqrt( a^2 - --------- )
*           (          b^2    )
*
**********************************************************************
*/
/*********************************************************************
*
*       GL_FillEllipse
*
* Function description
*   Draws a filled ellipse by respecting pen size.
*/
void GL_FillEllipse(int xm, int ym, int rx, int ry) {
  U32 rxrx, ryry, yy; // squared variables
  int x, y, xOld, xer, xal, yu, yl;

  ryry = ry * ry;
  rxrx = rx * rx;
  xOld = rx;
  for (y = 1; y <= ry; y++) {
    yy = y * y;
    x  = GUI__sqrt32(rxrx - yy * rxrx / ryry); // See heading comment
    //
    // Avoid overlapping of old and actual line
    //
    if (x != xOld) {
      xOld--;
    }
    xal  = xm - xOld;
    xer  = xm + xOld;
    xOld = x;
    yu   = ym - y + 1;
    yl   = ym + y - 1;
    LCD_DrawHLine(xm,  yu, xer);      // 1st quadrant
    LCD_DrawHLine(xal, yu, xm - 1);   // 2nd quadrant
    //
    // Avoid double painting
    //
    if (y > 1) {
      LCD_DrawHLine(xal, yl, xm - 1); // 3rd quadrant
      LCD_DrawHLine(xm,  yl, xer);    // 4th quadrant
    }
  }
}

/*********************************************************************
*
*       GUI_FillEllipse
*/
void GUI_FillEllipse(int x0, int y0, int rx, int ry) {
  U8 PenSize;
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  PenSize = GUI_GetPenSize();
  //
  // The ellipse will be drawn half of the pen size wider.
  //
  rx += (PenSize + 1) >> 1;
  ry += (PenSize + 1) >> 1;
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    //
    // Calc rectangle in order to avoid unnecessary drawing ops.
    //
    r.x0 = x0 - rx;
    r.x1 = x0 + rx;
    r.y0 = y0 - ry;
    r.y1 = y0 + ry;
    WM_ITERATE_START(&r); {
  #endif
  GL_FillEllipse(x0, y0, rx, ry);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GL_DrawEllipse
*
* Function description
*   Draws an ellipse by respecting pen size.
*/
void GL_DrawEllipse(int xm, int ym, int rx, int ry, int w) {
  U32 rxrx, ryry, rxrxi, ryryi, yy;  // squared variables
  int x, y, xOld, xer, xal, xari, xeli, yu, yl, rxi, ryi, xi;

  rxi    = rx  - w + 1;
  ryi    = ry  - w + 1;
  ryry   = ry  * ry;
  rxrx   = rx  * rx;
  ryryi  = ryi * ryi;
  rxrxi  = rxi * rxi;
  xOld   = rx;
  for (y = 1; y <= ry; y++) {
    yy = y  * y;
    yu = ym - y + 1;
    yl = ym + y - 1;
    x  = GUI__sqrt32(rxrx - yy * rxrx / ryry); // See heading comment
    //
    // Avoid overlapping of old and actual line
    //
    if (x != xOld) {
      xOld--;
    }
    xal  = xm - xOld;
    xer  = xm + xOld;
    xOld = x;
    if (y < ryi) {
      xi   = GUI__sqrt32(rxrxi - yy * rxrxi / ryryi);
      xeli = xm - xi;
      xari = xm + xi;
    } else if (y == ryi) {
      xeli = xm - 1;
      xari = xm;
    }
    LCD_DrawHLine(xari, yu, xer);    // 1st quadrant
    LCD_DrawHLine(xal,  yu, xeli);   // 2nd quadrant
    //
    // Avoid double painting
    //
    if (y > 1) {
      LCD_DrawHLine(xal,  yl, xeli); // 3rd quadrant
      LCD_DrawHLine(xari, yl, xer);  // 4th quadrant
    }
  }
}

/*********************************************************************
*
*       GUI_DrawEllipse
*/
void GUI_DrawEllipse(int x0, int y0, int rx, int ry) {
  U8 PenSize;
  #if (GUI_WINSUPPORT)
    GUI_RECT r;
  #endif
  PenSize = GUI_GetPenSize();
  //
  // The ellipse will be drawn half of the pen size wider.
  //
  rx += (PenSize + 1) >> 1;
  ry += (PenSize + 1) >> 1;
  if (rx < PenSize || ry < PenSize) {
    return;
  }
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    //
    // Calc rectangle in order to avoid unnecessary drawing ops.
    //
    r.x0 = x0 - rx;
    r.x1 = x0 + rx;
    r.y0 = y0 - ry;
    r.y1 = y0 + ry;
    WM_ITERATE_START(&r); {
  #endif
  if (PenSize == rx || PenSize == ry) {
    GL_FillEllipse(x0, y0, rx, ry);
  } else{
    GL_DrawEllipse(x0, y0, rx, ry, PenSize);
  }
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
