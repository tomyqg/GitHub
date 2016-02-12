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
File        : GUI__DrawTwinArc.c
Purpose     : Implementation of GUI__DrawTwinArc
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"

#if GUI_WINSUPPORT
  #include "WM_Intern.h"
#endif

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _GetTwinArcPara
*/
static void _GetTwinArcPara(int r, int y, int * pl0, int * pl1, int * pl2) {
  U32 yy;
  int ri;

  yy = y * y;
  ri = r - 1;
  *pl0 = GUI__sqrt32((U32)r * r - yy);
  if (y < ri) {
    *pl1 = GUI__sqrt32((U32)ri * ri - yy);
    if (y < (r - 2)) {
      *pl2 = *pl1 - 1;
    } else {
      *pl2 = 0;
    }
  } else {
    *pl1 = 0;
    *pl2 = 0;
  }
}

/*********************************************************************
*
*       _GetTrippleArcPara
*/
static void _GetTrippleArcPara(int r, int y, int * pl0, int * pl1, int * pl2, int * pl3) {
  U32 yy;
  int ri, rj;

  yy = y * y;
  ri = r - 1;
  rj = r - 2;
  *pl0 = GUI__sqrt32((U32)r * r - yy);
  if (y < ri) {
    *pl1 = GUI__sqrt32((U32)ri * ri - yy);
    if (y < rj) {
      *pl2 = GUI__sqrt32((U32)rj * rj - yy);
      if (y < (r - 3)) {
        *pl3 = *pl2 - 1;
      } else {
        *pl3 = 0;
      }
    } else {
      *pl2 = 0;
      *pl3 = 0;
    }
  } else {
    *pl1 = 0;
    *pl2 = 0;
    *pl3 = 0;
  }
}

/*********************************************************************
*
*       _DrawTwinArc4
*
* Purpose:
*   Fills the edges of the given area with the desired arc.
*   It first draws the pixels of ColorR0, then
*   the pixels of ColorR1 and then it fills up the rest with ColorFill.
*   This is done to make sure that in case of using automatic alpha
*   blending no superfluous device operations are required.
*
* Parameters:
*   x0        - Leftmost pixel
*   x1        - Rightmost pixel
*   y0        - Topmost pixel
*   y1        - Bottommost pixel
*   r         - Desired radius
*   ColorR0   - Outer color of arc
*   ColorR1   - Inner color of arc
*   ColorFill - Color for filling up the remaining pixels
*/
static void _DrawTwinArc4(int x0, int y0, int x1, int y1, int r, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorFill) {
  int y, l0, l1, l2, yu, yl, xal, xel, xar, xer;

  if (r > 0) {
    //
    // Draw outer arc with ColorR0
    //
    GUI_SetColor(ColorR0);
    for (y = r - 1; y > 0; y--) {
      yu = y0 + r - y;
      yl = y1 - r + y;
      _GetTwinArcPara(r, y, &l0, &l1, &l2);
      xal = x0 + r - l0;
      xel = x0 + r - l1 - 1;
      xer = x1 - r + l0;
      xar = x1 - r + l1 + 1;
      LCD_DrawHLine(xal, yu, xel);
      LCD_DrawHLine(xar, yu, xer);
      LCD_DrawHLine(xal, yl, xel);
      LCD_DrawHLine(xar, yl, xer);
    }
    if (r > 1) {
      //
      // Draw inner arc with ColorR1
      //
      GUI_SetColor(ColorR1);
      for (y = r - 2; y > 0; y--) {
        yu = y0 + r - y;
        yl = y1 - r + y;
        _GetTwinArcPara(r, y, &l0, &l1, &l2);
        xal = x0 + r - l1;
        xel = x0 + r - l2 - 1;
        xer = x1 - r + l1;
        xar = x1 - r + l2 + 1;
        LCD_DrawHLine(xal, yu, xel);
        LCD_DrawHLine(xar, yu, xer);
        LCD_DrawHLine(xal, yl, xel);
        LCD_DrawHLine(xar, yl, xer);
      }
      if (r > 2) {
        //
        // Fill up with ColorFill
        //
        GUI_SetColor(ColorFill);
        for (y = r - 3; y; y--) {
          yu = y0 + r - y;
          yl = y1 - r + y;
          _GetTwinArcPara(r, y, &l0, &l1, &l2);
          xal = x0 + r - l2;
          xel = x0 + r - 1;
          xer = x1 - r + l2;
          xar = x1 - r + 1;
          LCD_DrawHLine(xal, yu, xel);
          LCD_DrawHLine(xar, yu, xer);
          LCD_DrawHLine(xal, yl, xel);
          LCD_DrawHLine(xar, yl, xer);
        }
      }
    }
  }
}

/*********************************************************************
*
*       _DrawTwinArc2
*
* Purpose:
*   Fills the left and right side of the given area with Q1 and Q0
*   of the desired arc. It first draws the pixels of ColorR0, then
*   the pixels of ColorR1 and then it fills up the rest with ColorFill.
*   This is done to make sure that in case of using automatic alpha
*   blending no superfluous device operations are required.
*
* Parameters:
*   x0        - Leftmost pixel
*   x1        - Rightmost pixel
*   y0        - Topmost pixel
*   r         - Desired radius
*   ColorR0   - Outer color of arc
*   ColorR1   - Inner color of arc
*   ColorFill - Color for filling up the remaining pixels
*/
static void _DrawTwinArc2(int x0, int x1, int y0, int r, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorFill) {
  int y, l0, l1, l2, yu, xal, xel, xar, xer;

  if (r > 0) {
    //
    // Draw outer arc with ColorR0
    //
    GUI_SetColor(ColorR0);
    for (y = r - 1; y > 0; y--) {
      yu = y0 + r - y;
      _GetTwinArcPara(r, y, &l0, &l1, &l2);
      xal = x0 + r - l0;
      xel = x0 + r - l1 - 1;
      xer = x1 - r + l0;
      xar = x1 - r + l1 + 1;
      LCD_DrawHLine(xal, yu, xel);
      LCD_DrawHLine(xar, yu, xer);
    }
    if (r > 1) {
      //
      // Draw inner arc with ColorR1
      //
      GUI_SetColor(ColorR1);
      for (y = r - 2; y > 0; y--) {
        yu = y0 + r - y;
        _GetTwinArcPara(r, y, &l0, &l1, &l2);
        xal = x0 + r - l1;
        xel = x0 + r - l2 - 1;
        xer = x1 - r + l1;
        xar = x1 - r + l2 + 1;
        LCD_DrawHLine(xal, yu, xel);
        LCD_DrawHLine(xar, yu, xer);
      }
      if (r > 2) {
        //
        // Fill up with ColorFill
        //
        GUI_SetColor(ColorFill);
        for (y = r - 3; y; y--) {
          yu = y0 + r - y;
          _GetTwinArcPara(r, y, &l0, &l1, &l2);
          xal = x0 + r - l2;
          xel = x0 + r - 1;
          xer = x1 - r + l2;
          xar = x1 - r + 1;
          LCD_DrawHLine(xal, yu, xel);
          LCD_DrawHLine(xar, yu, xer);
        }
      }
    }
  }
}

/*********************************************************************
*
*       _FillTrippleArc
*
* Purpose:
*   Fills the left and right side of the given area with Q1 and Q0
*   of the desired arc. It first draws the pixels of ColorR0, then
*   the pixels of ColorR1 and then it fills up the rest with ColorFill.
*   This is done to make sure that in case of using automatic alpha
*   blending no superfluous device operations are required.
*
* Parameters:
*   x0, y0    - Upper left corner of area to be filled
*   Size      - Size of area to be filled
*   ColorR0   - Color of outer arc
*   ColorR1   - Color of middle arc
*   ColorR2   - Color of inner arc
*   ColorFill - Color for filling up the remaining pixels
*
* Limitations:
*   Size needs to be a multiple of 2
*/
#if 1

static void _FillTrippleArc(int x0, int y0, int Size, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorR2, GUI_COLOR ColorFill) {
  int y, l0, l1, l2, l3, yu, yl, xal, xel, xar, xer, x1, y1, r;

  //
  // Check if size is a multiple of 2
  //
  if (Size & 1) {
    return;
  }
  x1 = --x0 + Size + 1;
  y1 = --y0 + Size + 1;
  r = (Size >> 1) + 2;
  if (r > 0) {
    //
    // Draw outer arc with ColorR0
    //
    GUI_SetColor(ColorR0);
    for (y = r - 1; y > 1; y--) {
      yu = y0 + r - y;
      yl = y1 - r + y;
      _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
      xal = x0 + r - l0;
      xel = x0 + r - l1 - 1;
      xer = x1 - r + l0;
      xar = x1 - r + l1 + 1;
      if (xar < xel) {
        LCD_DrawHLine(xal, yu, xer);
        LCD_DrawHLine(xal, yl, xer);
      } else {
        LCD_DrawHLine(xal, yu, xel);
        LCD_DrawHLine(xar, yu, xer);
        LCD_DrawHLine(xal, yl, xel);
        LCD_DrawHLine(xar, yl, xer);
      }
    }
    if (r > 1) {
      //
      // Draw middle arc with ColorR1
      //
      GUI_SetColor(ColorR1);
      for (y = r - 2; y > 1; y--) {
        yu = y0 + r - y;
        yl = y1 - r + y;
        _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
        xal = x0 + r - l1;
        xel = x0 + r - l2 - 1;
        xer = x1 - r + l1;
        xar = x1 - r + l2 + 1;
        if (xar < xel) {
          LCD_DrawHLine(xal, yu, xer);
          LCD_DrawHLine(xal, yl, xer);
        } else {
          LCD_DrawHLine(xal, yu, xel);
          LCD_DrawHLine(xar, yu, xer);
          LCD_DrawHLine(xal, yl, xel);
          LCD_DrawHLine(xar, yl, xer);
        }
      }
      if (r > 2) {
        //
        // Draw inner arc with ColorR2
        //
        GUI_SetColor(ColorR2);
        for (y = r - 3; y > 1; y--) {
          yu = y0 + r - y;
          yl = y1 - r + y;
          _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
          xal = x0 + r - l2;
          xel = x0 + r - l3 - 1;
          xer = x1 - r + l2;
          xar = x1 - r + l3 + 1;
          if (xar < xel) {
            LCD_DrawHLine(xal, yu, xer);
            LCD_DrawHLine(xal, yl, xer);
          } else {
            LCD_DrawHLine(xal, yu, xel);
            LCD_DrawHLine(xar, yu, xer);
            LCD_DrawHLine(xal, yl, xel);
            LCD_DrawHLine(xar, yl, xer);
          }
        }
        if (r > 3) {
          //
          // Fill up with ColorFill
          //
          GUI_SetColor(ColorFill);
          for (y = r - 4; y; y--) {
            yu = y0 + r - y;
            yl = y1 - r + y;
            _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
            xal = x0 + r - l3;
            xer = x1 - r + l3;
            LCD_DrawHLine(xal, yu, xer);
            LCD_DrawHLine(xal, yl, xer);
          }
        }
      }
    }
  }
}

#else

static void _FillTrippleArc(int x0, int y0, int Size, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorR2, GUI_COLOR ColorFill) {
  int y, l0, l1, l2, l3, yu, yl, xal, xel, xar, xer, x1, y1, r;

  //
  // Check if size is a multiple of 2
  //
  if (Size & 1) {
    return;
  }
  x1 = --x0 + Size + 1;
  y1 = --y0 + Size + 1;
  r = (Size >> 1) + 1;
  if (r > 0) {
    //
    // Draw outer arc with ColorR0
    //
    GUI_SetColor(ColorR0);
    for (y = r - 1; y > 0; y--) {
      yu = y0 + r - y;
      yl = y1 - r + y;
      _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
      xal = x0 + r - l0;
      xel = x0 + r - l1 - 1;
      xer = x1 - r + l0;
      xar = x1 - r + l1 + 1;
      LCD_DrawHLine(xal, yu, xel);
      LCD_DrawHLine(xar, yu, xer);
      LCD_DrawHLine(xal, yl, xel);
      LCD_DrawHLine(xar, yl, xer);
    }
    if (r > 1) {
      //
      // Draw middle arc with ColorR1
      //
      GUI_SetColor(ColorR1);
      for (y = r - 2; y > 0; y--) {
        yu = y0 + r - y;
        yl = y1 - r + y;
        _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
        xal = x0 + r - l1;
        xel = x0 + r - l2 - 1;
        xer = x1 - r + l1;
        xar = x1 - r + l2 + 1;
        LCD_DrawHLine(xal, yu, xel);
        LCD_DrawHLine(xar, yu, xer);
        LCD_DrawHLine(xal, yl, xel);
        LCD_DrawHLine(xar, yl, xer);
      }
      if (r > 2) {
        //
        // Draw inner arc with ColorR2
        //
        GUI_SetColor(ColorR2);
        for (y = r - 3; y > 0; y--) {
          yu = y0 + r - y;
          yl = y1 - r + y;
          _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
          xal = x0 + r - l2;
          xel = x0 + r - l3 - 1;
          xer = x1 - r + l2;
          xar = x1 - r + l3 + 1;
          LCD_DrawHLine(xal, yu, xel);
          LCD_DrawHLine(xar, yu, xer);
          LCD_DrawHLine(xal, yl, xel);
          LCD_DrawHLine(xar, yl, xer);
        }
        if (r > 3) {
          //
          // Fill up with ColorFill
          //
          GUI_SetColor(ColorFill);
          for (y = r - 4; y; y--) {
            yu = y0 + r - y;
            yl = y1 - r + y;
            _GetTrippleArcPara(r, y, &l0, &l1, &l2, &l3);
            xal = x0 + r - l3;
            xer = x1 - r + l3;
            LCD_DrawHLine(xal, yu, xer);
            LCD_DrawHLine(xal, yl, xer);
          }
        }
      }
    }
  }
}

#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI__DrawTwinArc2
*/
void GUI__DrawTwinArc2(int x0, int x1, int y0, int r, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorFill) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORGX(x0);
    WM_ADDORGX(x1);
    WM_ADDORGY(y0);
    Rect.x0 = x0;
    Rect.x1 = x1;
    Rect.y0 = y0;
    Rect.y1 = Rect.y0 + r - 1;
    WM_ITERATE_START(&Rect) {
  #endif
  _DrawTwinArc2(x0, x1, y0, r, ColorR0, ColorR1, ColorFill);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI__DrawTwinArc4
*/
void GUI__DrawTwinArc4(int x0, int y0, int x1, int y1, int r, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorFill) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0, y0);
    WM_ADDORG(x1, y1);
    Rect.x0 = x0;
    Rect.y0 = y0;
    Rect.x1 = x1;
    Rect.y1 = y1;
    WM_ITERATE_START(&Rect) {
  #endif
  _DrawTwinArc4(x0, y0, x1, y1, r, ColorR0, ColorR1, ColorFill);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI__FillTrippleArc
*/
void GUI__FillTrippleArc(int x0, int y0, int Size, GUI_COLOR ColorR0, GUI_COLOR ColorR1, GUI_COLOR ColorR2, GUI_COLOR ColorFill) {
  #if (GUI_WINSUPPORT)
    GUI_RECT Rect;
  #endif
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0, y0);
    Rect.x0 = x0;
    Rect.y0 = y0;
    Rect.x1 = x0 + Size - 1;
    Rect.y1 = y0 + Size - 1;
    WM_ITERATE_START(&Rect) {
  #endif
  _FillTrippleArc(x0, y0, Size, ColorR0, ColorR1, ColorR2, ColorFill);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
