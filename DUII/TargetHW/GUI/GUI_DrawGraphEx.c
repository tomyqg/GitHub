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
File        : GUI_DrawGraphEx.c
Purpose     : Implementation of GUI_DrawGraphEx
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */

#include "GUI_Private.h"

/*********************************************************************
*
*       Defines
*
**********************************************************************
*/
#define SWAPSIGN        1
#define SWAPSIGN_DIVIDE 2
#define DIVIDE          3
#define MULTIPLY        4
#define MULTIPLY_DIVIDE 5

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawGraph
*/
static void _DrawGraph(I16 * pay, int NumPoints, int xOff, int yOff, int Numerator, int Denominator, int MirrorX) {
  int i;
  int y0, y1;
  int yClip0, yClip1;
  int Method;
  int Skip;
  int Cnt;
  int Add;
  int NumPointsVisible;
  I16 * p;
  I16 Value;
  /* Early out on wrong parameters */
  if ((Numerator == 0) || (Denominator == 0) || (NumPoints <= 0)) {
    return;
  }
  if (Numerator == -1) {
    if (Denominator == 1) {
      Method = SWAPSIGN;
    } else {
      Method = SWAPSIGN_DIVIDE;
    }
  } else if (Numerator == 1) {
    if (Denominator == 1) {
      Method = 0;
    } else {
      Method = DIVIDE;
    }
  } else {
    if (Denominator == 1) {
      Method = MULTIPLY;
    } else {
      Method = MULTIPLY_DIVIDE;
    }
  }
  /* Perform high level clipping in x */
  yClip0 = GUI_Context.pClipRect_HL->y0;
  yClip1 = GUI_Context.pClipRect_HL->y1;
  i = Cnt = 0;
  NumPointsVisible = NumPoints;
  if (GUI_Context.pClipRect_HL) {
    if (xOff < GUI_Context.pClipRect_HL->x0) {
      i = GUI_Context.pClipRect_HL->x0 - xOff;
    }
    if (xOff + NumPoints - 1 > GUI_Context.pClipRect_HL->x1) {
      NumPointsVisible = GUI_Context.pClipRect_HL->x1 - xOff + 1;
    }
  }
  if (MirrorX) {
    p   = pay + NumPoints - 1;
    Add = -1;
  } else {
    Add = 1;
    p = pay + i;
  }
  /* Start drawing if there is something left to do after x-clipping */
  Skip = 1;
  if (i < NumPointsVisible) {
    /* Iterate */
    for (; i < NumPointsVisible; i++) {
      Value = *p;
      p    += Add;
      /* Check if current data is invalid */
      if (Value == 0x7fff) {
        /* Draw point if only one value is valid */
        if ((!Cnt) && (!Skip)) {
          LCD_DrawPixel(i + xOff - 1, y0);
        }
        Skip = 1;
        Cnt = 0;
        continue;
      } else {
        if (Skip) {
          /* Do nothing but calculate new y0 value */
          Skip = 0;
          switch (Method) {
          case SWAPSIGN:
            y0 = -Value + yOff;
            break;
          case SWAPSIGN_DIVIDE:
            y0 = (U32)(-Value + yOff) / (U32)Denominator;
            break;
          case DIVIDE:
            y0 = (U32)(Value + yOff) / (U32)Denominator;
            break;
          case MULTIPLY:
            y0 = (U32)Numerator * (U32)(Value + yOff); /* Sequence swapped because of tool internal error of IAR V850 V340A */
            break;
          case MULTIPLY_DIVIDE:
            y0 = (U32)(Value + yOff) * (U32)Numerator / (U32)Denominator;
            break;
          default:
            y0 = Value + yOff;
          }
          continue;
        }
      }
      /* Perform high level clipping in y */
      switch (Method) {
      case SWAPSIGN:
        y1 = -Value + yOff;
        break;
      case SWAPSIGN_DIVIDE:
        y1 = (U32)(-Value + yOff) / (U32)Denominator;
        break;
      case DIVIDE:
        y1 = (U32)(Value + yOff) / (U32)Denominator;
        break;
      case MULTIPLY:
        y1 = (U32)(Value + yOff) * (U32)Numerator;
        break;
      case MULTIPLY_DIVIDE:
        y1 = (U32)(Value + yOff) * (U32)Numerator / (U32)Denominator;
        break;
      default:
        y1 = Value + yOff;
      }
      if ((y0 >= yClip0) || (y1 >= yClip0)) {
        if ((y0 <= yClip1) || (y1 <= yClip1)) {
          int y01;
          y01 = (y1 + y0) / 2;
          if (y0 <= y1) {
            LCD_DrawVLine(i + xOff - 1, y0, y01);
            LCD_DrawVLine(i + xOff, y01, y1);
          } else {
            LCD_DrawVLine(i + xOff - 1, y01, y0);
            LCD_DrawVLine(i + xOff, y1, y01);
          }
        }
      }
      Cnt++;
      y0 = y1;
    }
  }
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUI_DrawGraphEx
*/  
void GUI_DrawGraphEx(I16 *pay, int NumPoints, int x0, int y0, int Numerator, int Denominator, int MirrorX) {
  GUI_LOCK();
  #if (GUI_WINSUPPORT)
    WM_ADDORG(x0,y0);
    WM_ITERATE_START(NULL); {
  #endif
  _DrawGraph(pay, NumPoints, x0, y0, Numerator, Denominator, MirrorX);
  #if (GUI_WINSUPPORT)
    } WM_ITERATE_END();
  #endif
  GUI_UNLOCK();
}

/*************************** End of file ****************************/
