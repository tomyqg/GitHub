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
File        : GUIARCFloat.C
Purpose     : Draw Arc routines based on floating point
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
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
*       _CalcX
*/
static void _CalcX(int*px, int y, U32 r2) {
  int x;
  U32 y2, r2y2, x2;

  x    = *px;
  y2   = (U32)y * (U32)y;
  r2y2 = r2 - y2;
  if (y2 >= r2) {
    *px = 0;
    return;
  }
  //
  // x2 = r2 - y2
  //
  do {
    x++;
    x2 = (U32)x * (U32)x;
  } while (x2 < r2y2);
  *px = x - 1;
}

/*********************************************************************
*
*       _CalcInterSectLin
*/
static float _CalcInterSectLin(float y, float y0, float y1, float x0, float x1) {
  float Slope, r;
  
  if (y1 == y0) {
    r = y0;
  } else {
    Slope = (x1 - x0) / (y1 - y0);
    r     = (y - y0) * Slope + x0;
  }
  return r;
}

/*********************************************************************
*
*       _DrawArc
*/
static void _DrawArc(int x0, int y0, int rx, int ry, int Angle0, int Angle1, int xMul, int yMul) {
  float afx[4], afy[4], ri, ro;
  I32   Sin0_HQ, Sin1_HQ, Cos0_HQ, Cos1_HQ;
  U32   ri2, ro2;
  int   y, yMax, yMin;

  ri      = rx - ((float)GUI_Context.PenSize) / 2;
  ro      = rx + ((float)GUI_Context.PenSize) / 2;
  Sin0_HQ = GUI__SinHQ(Angle0 * 1000); 
  Sin1_HQ = GUI__SinHQ(Angle1 * 1000); 
  Cos0_HQ = GUI__CosHQ(Angle0 * 1000); 
  Cos1_HQ = GUI__CosHQ(Angle1 * 1000); 
  ri2     = (U32)(ri * ri);
  ro2     = (U32)(ro * ro);
  afy[0]  = (float)((I32)(ri * Sin0_HQ) >> 16);
  afy[1]  = (float)((I32)(ro * Sin0_HQ) >> 16);
  afy[2]  = (float)((I32)(ri * Sin1_HQ) >> 16);
  afy[3]  = (float)((I32)(ro * Sin1_HQ) >> 16);
  afx[0]  = (float)((I32)(ri * Cos0_HQ) >> 16);
  afx[1]  = (float)((I32)(ro * Cos0_HQ) >> 16);
  afx[2]  = (float)((I32)(ri * Cos1_HQ) >> 16);
  afx[3]  = (float)((I32)(ro * Cos1_HQ) >> 16);
  yMin    = (int)ceil(afy[0]);
  yMax    = (int)floor(afy[3]);
  //
  // Use Clipping rect to reduce calculation (if possible)
  //
  if (GUI_Context.pClipRect_HL) {
    if (yMul == 1) {
      if (yMax > (GUI_Context.pClipRect_HL->y1 - y0)) {
        yMax = (GUI_Context.pClipRect_HL->y1 - y0);
      }
      if (yMin < (GUI_Context.pClipRect_HL->y0 - y0)) {
        yMin = (GUI_Context.pClipRect_HL->y0 - y0);
      }
    }
    if (yMul == -1) {
      if (yMin > (GUI_Context.pClipRect_HL->y1 - y0)) {
        yMin = (GUI_Context.pClipRect_HL->y1 - y0);
      }
      if (yMax < (GUI_Context.pClipRect_HL->y0 - y0)) {
        yMax = (GUI_Context.pClipRect_HL->y0 - y0);
      }
    }
  }
  //
  // Start drawing lines ...
  //
  {
  int xMinDisp, xMaxDisp, xMin = 0, xMax = 0;
    for (y = yMax; y >= yMin; y--) {
      _CalcX(&xMin, y, ri2);
      _CalcX(&xMax, y, ro2);
      if ((float)y < afy[1]) {
        xMaxDisp = (int)_CalcInterSectLin(y, afy[0], afy[1], afx[0], afx[1]);
      } else {
        xMaxDisp = xMax;      
      }
      if ((float)y > afy[2]) {
        xMinDisp = (int)_CalcInterSectLin(y, afy[2], afy[3], afx[2], afx[3]);
      } else {
        xMinDisp = xMin;      
      }
      if (xMul > 0) {
        LCD_HL_DrawHLine( xMinDisp + x0, yMul * y + y0,  xMaxDisp + x0);
      } else {
        LCD_HL_DrawHLine(-xMaxDisp + x0, yMul * y + y0, -xMinDisp + x0);
      }
    }
  }

#if 0  /* Test code */
{
  int i;
  GUI_SetColor(GUI_WHITE); 
  for (i = 0; i < 4; i++) {
    LCD_HL_DrawPixel(afx[i] + x0, afy[i] + y0);
  }
}
#endif

  GUI_USE_PARA(ry);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GL_DrawArc
*/
void GL_DrawArc(int x0, int y0, int rx, int ry, int a0, int a1) {
  int aEnd;
  a0 += 360;
  a1 += 360;
  while (a0 >= 360) {
    a0 -= 360;
    a1 -= 360;
  }
/* Do first quadrant 0-90 degree */
DoFirst:
  if (a1 <= 0) {
    return;
  }
  if (a0 < 90) {
    if (a0 < 0) {
      a0 = 0;
    }
    aEnd = (a1 < 90) ? a1 : 90;
    _DrawArc(x0, y0, rx, ry, a0, aEnd, 1, -1);
  }
  a1 -= 90;
  a0 -= 90;
/* Do second quadrant 90-180 degree */
  if (a1 <= 0) {
    return;
  }
  if (a0 < 90) {
    if (a0 < 0) {
      a0 = 0;
    }
    aEnd = (a1 < 90) ? a1 : 90;
    _DrawArc(x0, y0, rx, ry, 90 - aEnd, 90 - a0, -1, -1);
  }
  a1 -= 90;
  a0 -= 90;
/* Do third quadrant 180-270 degree */
  if (a1 <= 0) {
    return;
  }
  if (a0 < 90) {
    if (a0 < 0) {
      a0 = 0;
    }
    aEnd = (a1 < 90) ? a1 : 90;
    _DrawArc(x0, y0, rx, ry, a0, aEnd, -1, 1);
  }
  a1 -= 90;
  a0 -= 90;
/* Do last quadrant 270-360 degree */
  if (a1 <= 0) {
    return;
  }
  if (a0 < 90) {
    if (a0 < 0) {
      a0 = 0;
    }
    aEnd = (a1 < 90) ? a1 : 90;
    _DrawArc(x0, y0, rx, ry, 90 - aEnd, 90 - a0, 1, 1);
  }
  a1 -= 90;
  a0 -= 90;
  goto DoFirst;
}

/*********************************************************************
*
*       GUI_DrawArc
*/
void GUI_DrawArc(int x0, int y0, int rx, int ry, int a0, int a1) {
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ITERATE_START(NULL) {
  #endif
  GL_DrawArc( x0, y0, rx, ry, a0, a1);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
